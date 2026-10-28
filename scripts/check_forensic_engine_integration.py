"""Validate the forensic-engine migration gate manifest without external dependencies."""
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "research" / "forensic-engine-integration.json"
EXPECTED = {
    "nexvary/NEXVARY-Digital-Forensics-Workbench",
    "nexvary/NEXVARY-Browser-Forensics-Lab",
    "nexvary/NEXVARY-DC",
}

def require(condition, message):
    if not condition:
        raise ValueError(message)


def validate(payload):
    require(isinstance(payload, dict) and payload.get("schema_version") == 1, "unknown manifest version")
    sources = payload.get("sources")
    require(isinstance(sources, list) and len(sources) == 3, "exactly three approved sources required")
    require(all(isinstance(s, dict) for s in sources), "invalid source record")
    require({s.get("repository") for s in sources} == EXPECTED, "unexpected source repository")
    adapters = {
        "nexvary/NEXVARY-Digital-Forensics-Workbench": "workbench-export-manifest",
        "nexvary/NEXVARY-Browser-Forensics-Lab": "chromium-bookmarks",
        "nexvary/NEXVARY-DC": "dc-image-partitions",
    }
    for source in sources:
        require(source.get("permission") == "read-only", "destructive integration forbidden")
        require(type(source.get("enabled")) is bool, "enabled must be boolean")
        require(type(source.get("license_verified")) is bool, "license flag must be boolean")
        if source["enabled"]:
            require(source["license_verified"], "cannot enable source before selected-module permission audit")
            require(isinstance(source.get("source_sha"), str) and re.fullmatch(r"[0-9a-f]{40}", source["source_sha"]) is not None, "pin exact upstream SHA")
            require(source.get("permission_basis") == "LicenseRef-NEXVARY-Owner-Authorized-Integration", "missing scoped owner permission")
            require(source.get("permission_record") == "docs/FORENSIC_SOURCE_PERMISSIONS.md", "missing auditable permission record")
            require(source.get("adapter") == adapters[source["repository"]], "unapproved adapter or write command")
            require(source.get("adapter_version") == "nexvary-mr-forensic/1", "unknown adapter version")
    stages = payload.get("stages")
    require(isinstance(stages, list) and all(isinstance(s, dict) for s in stages), "invalid stages")
    require([s.get("id") for s in stages] == list(range(1, 10)), "nine ordered stages required")
    require(all(s.get("state") in {"pending", "in-progress", "verified", "blocked"} for s in stages), "invalid stage state")

if __name__ == "__main__":
    validate(json.loads(MANIFEST.read_text(encoding="utf-8")))
    print("forensic engine manifest gate: OK (does not certify any adapter)")
