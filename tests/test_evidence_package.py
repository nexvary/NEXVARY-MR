import hashlib
import importlib.util
import json
import os
from pathlib import Path
import tempfile
import unittest
import uuid

spec = importlib.util.spec_from_file_location("verify_evidence_package", Path(__file__).resolve().parents[1] / "scripts/verify_evidence_package.py")
verifier = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verifier)


class EvidencePackageTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.data = b"Independent synthetic byte fixture\x00\xff"
        (self.root / "evidence.bin").write_bytes(self.data)
        self.manifest = {"schema": "nexvary-evidence-package/v1", "packageId": str(uuid.uuid4()), "caseId": str(uuid.uuid4()),
                         "evidenceId": str(uuid.uuid4()), "deviceId": str(uuid.uuid4()), "caseRevision": 3,
                         "auditHead": "a" * 64, "synthetic": True, "actor": "Synthetic examiner", "purpose": "Independent fixture",
                         "exportedAtUtc": "2026-10-25T00:00:00Z", "sourceName": "fixture.bin", "trustBoundary": "Byte integrity only",
                         "files": [{"path": "evidence.bin", "size": len(self.data), "sha256": hashlib.sha256(self.data).hexdigest()}]}

    def save(self):
        data = json.dumps(self.manifest, ensure_ascii=False).encode()
        (self.root / "manifest.json").write_bytes(data)
        return hashlib.sha256(data).hexdigest()

    def test_valid_independent_fixture(self):
        result = verifier.verify_package(self.root, self.save())
        self.assertTrue(result["integrityVerified"])
        self.assertFalse(result["externalAuthenticityVerified"])

    def test_replaced_manifest_requires_separate_digest(self):
        expected = self.save()
        self.manifest["purpose"] = "Changed"
        self.save()
        with self.assertRaises(ValueError):
            verifier.verify_package(self.root, expected)

    def test_same_size_evidence_tampering(self):
        expected = self.save()
        (self.root / "evidence.bin").write_bytes(b"X" * len(self.data))
        with self.assertRaises(ValueError):
            verifier.verify_package(self.root, expected)

    def test_path_and_schema_attacks(self):
        for value in ("../evidence.bin", "/tmp/evidence.bin", "C:\\data", "file:///evidence.bin"):
            self.manifest["files"][0]["path"] = value
            with self.assertRaises(ValueError):
                verifier.verify_package(self.root, self.save())
        self.manifest["files"][0]["path"] = "evidence.bin"
        self.manifest["files"][0]["size"] = True
        with self.assertRaises(ValueError):
            verifier.verify_package(self.root, self.save())

    def test_missing_added_symlink_and_large_files(self):
        expected = self.save()
        (self.root / "unexpected").write_bytes(b"X")
        with self.assertRaises(ValueError):
            verifier.verify_package(self.root, expected)
        (self.root / "unexpected").unlink()
        (self.root / "evidence.bin").unlink()
        with self.assertRaises(ValueError):
            verifier.verify_package(self.root, expected)
        if os.name != "nt":
            (self.root / "evidence.bin").symlink_to(self.root / "manifest.json")
            with self.assertRaises(ValueError):
                verifier.verify_package(self.root, expected)
            (self.root / "evidence.bin").unlink()
        with (self.root / "evidence.bin").open("wb") as stream:
            stream.truncate(verifier.LIMIT + 1)
        with self.assertRaises(ValueError):
            verifier.verify_package(self.root, expected)

    def test_duplicate_fields_and_invalid_json(self):
        for data in (b'{"schema":1,"schema":2}', b'{"bad":NaN}', b'[]', b'\xff'):
            (self.root / "manifest.json").write_bytes(data)
            with self.assertRaises((ValueError, UnicodeError)):
                verifier.verify_package(self.root, hashlib.sha256(data).hexdigest())
