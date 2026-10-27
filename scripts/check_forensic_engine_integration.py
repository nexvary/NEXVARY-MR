"""Validate the forensic-engine migration gate manifest without external dependencies."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "research" / "forensic-engine-integration.json"
EXPECTED = {
    "nexvary/NEXVARY-Digital-Forensics-Workbench",
    "nexvary/NEXVARY-Browser-Forensics-Lab",
    "nexvary/NEXVARY-DC",
}

def validate(payload):
    assert payload.get("schema_version") == 1, "unknown manifest version"
    sources = payload.get("sources")
    assert isinstance(sources, list) and len(sources) == 3, "exactly three approved sources required"
    assert {s["repository"] for s in sources} == EXPECTED, "unexpected source repository"
    for source in sources:
        assert source["permission"] == "read-only", "destructive integration forbidden"
        assert isinstance(source["enabled"], bool), "enabled must be boolean"
        assert isinstance(source["license_verified"], bool), "license flag must be boolean"
        if source["enabled"]:
            assert source["license_verified"], "cannot enable source before license audit"
            sha = source["source_sha"]
            assert isinstance(sha, str) and len(sha) == 40 and all(c in "0123456789abcdef" for c in sha), "pin exact upstream SHA"
    stages = payload.get("stages")
    assert isinstance(stages, list) and [s["id"] for s in stages] == list(range(1, 10)), "nine ordered stages required"
    assert all(s["state"] in {"pending", "in-progress", "verified", "blocked"} for s in stages), "invalid stage state"

if __name__ == "__main__":
    validate(json.loads(MANIFEST.read_text(encoding="utf-8")))
    print("forensic engine manifest gate: OK (does not certify any adapter)")
