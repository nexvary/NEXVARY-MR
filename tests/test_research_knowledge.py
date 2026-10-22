"""Traceability integrity; a source URL or passing fixture never approves a model."""
import copy
import json
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]


def validate(document):
    if document.get("schemaVersion") != 1:
        return False
    groups = ("sources", "technologies", "chipsets", "devices", "firmware", "capabilities", "testEvidence")
    maps = {}
    try:
        for group in groups:
            rows = document[group]
            maps[group] = {row["id"]: row for row in rows}
            if len(maps[group]) != len(rows) or any(not isinstance(key, str) or not key for key in maps[group]):
                return False
        for technology in document["technologies"]:
            if not technology["sourceIds"] or any(key not in maps["sources"] for key in technology["sourceIds"]):
                return False
        for capability in document["capabilities"]:
            for field, group in (("technologyIds", "technologies"), ("chipsetIds", "chipsets"), ("deviceIds", "devices"), ("firmwareIds", "firmware"), ("testEvidenceIds", "testEvidence")):
                if any(key not in maps[group] for key in capability[field]):
                    return False
            if not capability["technologyIds"] or not capability["testEvidenceIds"]:
                return False
            if capability["proofLevel"] not in ("synthetic-only", "device-approved"):
                return False
            if capability["proofLevel"] == "device-approved":
                if not capability["deviceIds"] or not capability["firmwareIds"]:
                    return False
                # Approval requires exact device/firmware coverage, not a global pass.
                for device in capability["deviceIds"]:
                    for firmware in capability["firmwareIds"]:
                        matching = [maps["testEvidence"][key] for key in capability["testEvidenceIds"]]
                        if not any(e.get("kind") == "physical-android" and e.get("passed") is True
                                   and device in e.get("deviceIds", []) and firmware in e.get("firmwareIds", [])
                                   and isinstance(e.get("commit"), str) and len(e["commit"]) == 40
                                   and all(c in "0123456789abcdef" for c in e["commit"])
                                   and e.get("reviewer") and e.get("artifact") for e in matching):
                            return False
        return True
    except (KeyError, TypeError, AttributeError):
        return False


class ResearchKnowledgeTests(unittest.TestCase):
    def setUp(self):
        self.document = json.loads((ROOT / "research/advanced-mobile/knowledge.json").read_text())

    def test_repository_paths_and_references(self):
        self.assertTrue(validate(self.document))
        for row in self.document["testEvidence"]:
            self.assertTrue((ROOT / row["path"]).is_file())
        for row in self.document["capabilities"]:
            for path in row["implementationPaths"]:
                self.assertTrue((ROOT / path).is_file())

    def test_dangling_and_duplicate_records_rejected(self):
        invalid = copy.deepcopy(self.document)
        invalid["capabilities"][0]["testEvidenceIds"].append("missing")
        self.assertFalse(validate(invalid))
        invalid = copy.deepcopy(self.document)
        invalid["sources"].append(invalid["sources"][0])
        self.assertFalse(validate(invalid))

    def test_fixture_cannot_be_promoted_to_physical_support(self):
        self.document["capabilities"][0]["proofLevel"] = "device-approved"
        self.assertFalse(validate(self.document))
        self.document["devices"] = [{"id": "fixture-device"}]
        self.document["firmware"] = [{"id": "fixture-build"}]
        self.document["capabilities"][0].update(deviceIds=["fixture-device"], firmwareIds=["fixture-build"])
        self.assertFalse(validate(self.document))

    def test_physical_record_requires_exact_scope_and_strict_pass(self):
        capability = self.document["capabilities"][0]
        capability.update(proofLevel="device-approved", deviceIds=["fixture-device"], firmwareIds=["fixture-build"])
        self.document["devices"] = [{"id": "fixture-device"}]
        self.document["firmware"] = [{"id": "fixture-build"}]
        evidence = self.document["testEvidence"][0]
        evidence.update(kind="physical-android", passed=True, deviceIds=["fixture-device"], firmwareIds=["other-build"], commit="a"*40, reviewer="fixture", artifact="fixture-reference")
        self.assertFalse(validate(self.document))
        evidence["firmwareIds"] = ["fixture-build"]
        self.assertTrue(validate(self.document))
        evidence["passed"] = "true"
        self.assertFalse(validate(self.document))

    def test_avb_implementation_declares_only_synthetic_scope(self):
        capability = self.document["capabilities"][0]
        self.assertEqual(capability["id"], "firmware.avb.metadata.v1")
        self.assertEqual(capability["proofLevel"], "synthetic-only")
        self.assertIs(capability["mutatesDevice"], False)
        self.assertEqual(capability["deviceIds"], [])
        self.assertEqual(capability["firmwareIds"], [])


if __name__ == "__main__":
    unittest.main()
