import hashlib
import importlib.util
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
spec = importlib.util.spec_from_file_location("signature_report", Path(__file__).with_name("signature_report.py"))
reporter = importlib.util.module_from_spec(spec)
spec.loader.exec_module(reporter)


class SignatureReportTests(unittest.TestCase):
    def test_report_hash_and_candidate(self):
        with tempfile.TemporaryDirectory() as directory:
            image = Path(directory) / "sample.img"
            data = b"header" + b"%PDF-" + b"content"
            image.write_bytes(data)
            result = reporter.report(image)
            self.assertEqual(result["sha256"], hashlib.sha256(data).hexdigest())
            self.assertEqual(result["candidate_count"], 1)
            self.assertEqual(result["candidates"], [{"format": "pdf", "offset": 6}])
            self.assertEqual(image.read_bytes(), data)


if __name__ == "__main__":
    unittest.main()
