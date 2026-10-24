"""Read-only, opt-in import of sanitized USB Studio diagnostic exports.

No device I/O, process execution, network access, or filesystem writes.
This is a deliberately narrow interoperability boundary, not a SIM driver.
"""
from __future__ import annotations

import json
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

MAX_BYTES = 1024 * 1024
MAX_FINDINGS = 128
ALLOWED_STATUS = frozenset({"ok", "warning", "error", "unknown", "unsupported"})
ALLOWED_KEYS = frozenset({"schema_version", "tool_version", "collection_time_utc",
                           "device_scope", "evidence_status", "limitations", "findings"})
SENSITIVE_KEYS = frozenset({"imsi", "iccid", "ki", "pin", "puk", "apdu", "raw", "secret",
                            "token", "password", "location", "cell_id", "imei"})


class UsbStudioImportError(ValueError):
    """Invalid, unsupported or unsafe diagnostic report."""


def _text(value: Any, limit: int = 512) -> str:
    if not isinstance(value, str) or not value or len(value) > limit:
        raise UsbStudioImportError("invalid bounded text field")
    return value


def parse_report(payload: bytes, *, max_age_seconds: int = 86400,
                 now: datetime | None = None) -> dict[str, Any]:
    """Validate an explicitly exported, normalized v1 report.

    No automatic translation of USB Studio's native JSON format is claimed.
    A future exporter must explicitly produce this documented schema.
    """
    if len(payload) > MAX_BYTES:
        raise UsbStudioImportError("report exceeds size limit")
    try:
        data = json.loads(payload.decode("utf-8"))
    except (UnicodeError, ValueError) as exc:
        raise UsbStudioImportError("invalid UTF-8 JSON") from exc
    if not isinstance(data, dict) or set(data) != ALLOWED_KEYS:
        raise UsbStudioImportError("unexpected report fields")
    if data["schema_version"] != "1":
        raise UsbStudioImportError("unsupported schema version")
    tool = _text(data["tool_version"], 80)
    scope = _text(data["device_scope"], 128)
    status = _text(data["evidence_status"], 32)
    if status not in ALLOWED_STATUS:
        raise UsbStudioImportError("invalid evidence status")
    timestamp = _text(data["collection_time_utc"], 40)
    try:
        collected = datetime.fromisoformat(timestamp.replace("Z", "+00:00"))
    except ValueError as exc:
        raise UsbStudioImportError("invalid timestamp") from exc
    if collected.tzinfo is None or collected.utcoffset().total_seconds() != 0:
        raise UsbStudioImportError("UTC timestamp required")
    current = now or datetime.now(timezone.utc)
    if current.tzinfo is None:
        raise UsbStudioImportError("timezone-aware now required")
    age = (current - collected).total_seconds()
    if age < -300 or age > max_age_seconds:
        raise UsbStudioImportError("stale or future report")
    limitations = data["limitations"]
    findings = data["findings"]
    if not isinstance(limitations, list) or len(limitations) > 32:
        raise UsbStudioImportError("invalid limitations")
    if not isinstance(findings, list) or len(findings) > MAX_FINDINGS:
        raise UsbStudioImportError("invalid findings")
    clean_limits = [_text(item) for item in limitations]
    clean_findings = []
    for item in findings:
        if not isinstance(item, dict) or set(item) != {"code", "status", "summary"}:
            raise UsbStudioImportError("unexpected finding fields")
        code = _text(item["code"], 80)
        finding_status = _text(item["status"], 32)
        summary = _text(item["summary"])
        if finding_status not in ALLOWED_STATUS:
            raise UsbStudioImportError("invalid finding status")
        clean_findings.append({"code": code, "status": finding_status, "summary": summary})
    # Refuse sensitive field names even when nested inside apparently benign text.
    for field in [scope, *clean_limits, *(f["summary"] for f in clean_findings)]:
        if any(token in field.lower() for token in SENSITIVE_KEYS):
            raise UsbStudioImportError("possible sensitive identifier in export")
    return {"schema_version": "1", "tool_version": tool, "collection_time_utc": timestamp,
            "device_scope": scope, "evidence_status": status,
            "limitations": clean_limits, "findings": clean_findings}


def load_report(path: str | Path) -> dict[str, Any]:
    """Load a user-selected file only; no directory discovery or subprocess."""
    file = Path(path)
    with file.open("rb") as stream:
        payload = stream.read(MAX_BYTES + 1)
    return parse_report(payload)
