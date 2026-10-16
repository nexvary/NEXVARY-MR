"""Stable JSON-lines protocol for the optional recovery assessment process.

Input is a single JSON object. Output is a single JSON object. No device I/O.
"""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

from recovery_assessment import Source, assess


def evaluate(request: dict) -> dict:
    if not isinstance(request, dict):
        raise ValueError("Request must be a JSON object")
    allowed = {"source", "factory_reset", "authorized", "readable"}
    if set(request) - allowed:
        raise ValueError("Unknown request fields")
    if not isinstance(request.get("factory_reset"), bool):
        raise ValueError("factory_reset must be boolean")
    if not isinstance(request.get("authorized"), bool):
        raise ValueError("authorized must be boolean")
    readable = request.get("readable")
    if readable is not None and not isinstance(readable, bool):
        raise ValueError("readable must be boolean or null")
    source = Source(request["source"])
    result = assess(source, factory_reset=request["factory_reset"],
                    authorized=request["authorized"], readable=readable)
    return {"schema": 1, "verdict": result.verdict.value,
            "next_action": result.next_action, "limitations": list(result.limitations)}


def main() -> int:
    parser = argparse.ArgumentParser(description="Offline, read-only recovery assessment")
    parser.add_argument("--input", type=Path, help="Read JSON from file rather than stdin")
    args = parser.parse_args()
    try:
        if args.input:
            if args.input.stat().st_size > 16384:
                raise ValueError("Input too large")
            raw = args.input.read_text(encoding="utf-8")
        else:
            raw = sys.stdin.read(16385)
            if len(raw) > 16384:
                raise ValueError("Input too large")
        output = evaluate(json.loads(raw))
        print(json.dumps(output, ensure_ascii=False))
        return 0
    except (ValueError, KeyError, TypeError, OSError, json.JSONDecodeError) as exc:
        print(json.dumps({"schema": 1, "error": str(exc)}), file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
