import hashlib
import importlib.util
import tempfile
import unittest
from pathlib import Path

source = Path(__file__).with_name("evidence_manifest.py")
spec = importlib.util.spec_from_file_location("evidence_manifest", source)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class EvidenceManifestTests(unittest.TestCase):
    def test_known_hash(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "sample.bin"
            path.write_bytes(b"abc")
            self.assertEqual(module.sha256_file(path), hashlib.sha256(b"abc").hexdigest())

    def test_compare(self):
        expected = {"a": "123", "b": "456", "c": "789"}
        found = {"a": "123", "b": "000"}
        self.assertEqual(module.compare_manifest(expected, found), {
            "a": "verified", "b": "hash-mismatch", "c": "not-recovered"
        })

    def test_output_safety(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            with self.assertRaises(ValueError):
                module.write_manifest(root, root / "manifest.json")


if __name__ == "__main__":
    unittest.main()
