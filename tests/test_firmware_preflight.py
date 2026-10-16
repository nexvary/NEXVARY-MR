import hashlib
import tempfile
import unittest
from pathlib import Path
from nexvary_mr.firmware_preflight import FirmwareManifest, verify_firmware

class FirmwarePreflightTests(unittest.TestCase):
    def test_matching_checksum_and_model_still_not_flash_ready(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp) / "official.zip"
            p.write_bytes(b"sample firmware test bytes")
            m = FirmwareManifest("Samsung", "SM-A525F", "EG", hashlib.sha256(p.read_bytes()).hexdigest(), True)
            result = verify_firmware(p, m, "Samsung", "SM-A525F", True)
            self.assertTrue(result["verified_hash"])
            self.assertFalse(result["ready_to_flash"])
            self.assertFalse(result["signature_verified"])

    def test_rejects_wrong_model(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp) / "firmware.zip"
            p.write_bytes(b"x")
            m = FirmwareManifest("Samsung", "SM-A525F", "EG", hashlib.sha256(b"x").hexdigest(), True)
            with self.assertRaises(ValueError):
                verify_firmware(p, m, "Samsung", "SM-A526B", True)

    def test_rejects_unverified_source(self):
        m = FirmwareManifest("Samsung", "SM-A525F", "EG", "0"*64, False)
        with self.assertRaises(PermissionError):
            verify_firmware("missing.zip", m, "Samsung", "SM-A525F", True)

    def test_requires_consent(self):
        m = FirmwareManifest("Samsung", "SM-A525F", "EG", "0"*64, True)
        with self.assertRaises(PermissionError):
            verify_firmware("missing.zip", m, "Samsung", "SM-A525F")
