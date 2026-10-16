import importlib.util
import sys
import tempfile
import unittest
from pathlib import Path

spec = importlib.util.spec_from_file_location("signature_scan", Path(__file__).with_name("signature_scan.py"))
scanner = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = scanner
spec.loader.exec_module(scanner)


class SignatureScanTests(unittest.TestCase):
    def test_signatures_and_offsets(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "evidence.img"
            path.write_bytes(b"x" * 32 + scanner.SIGNATURES["png"] + b"z" * 8 +
                             scanner.SIGNATURES["pdf"] + b"end")
            result = scanner.scan_image(path, chunk_size=16)
            self.assertEqual([(item.format, item.offset) for item in result],
                             [("png", 32), ("pdf", 48)])

    def test_boundary_crossing(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "evidence.img"
            path.write_bytes(b"x" * 15 + scanner.SIGNATURES["sqlite"] + b"rest")
            result = scanner.scan_image(path, chunk_size=16)
            self.assertEqual([(item.format, item.offset) for item in result],
                             [("sqlite", 15)])

    def test_reject_oversized_input(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "evidence.img"
            path.write_bytes(b"0" * 100)
            with self.assertRaises(ValueError):
                scanner.scan_image(path, max_bytes=10)

    def test_reject_excessive_matches(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "evidence.img"
            path.write_bytes(scanner.SIGNATURES["pdf"] * 5)
            with self.assertRaises(ValueError):
                scanner.scan_image(path, max_matches=2)

    def test_jpeg_exif_marker(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "exif.img"
            path.write_bytes(b"padding" + bytes.fromhex("ffd8ffe1") + b"Exif")
            matches = scanner.scan_image(path)
            self.assertEqual([(m.format, m.offset) for m in matches], [("jpeg", 7)])

    def test_empty_image(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "evidence.img"
            path.write_bytes(b"")
            self.assertEqual(scanner.scan_image(path), [])


if __name__ == "__main__":
    unittest.main()
