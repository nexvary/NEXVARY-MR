"""Safe, model-aware OEM firmware preflight; never flashes devices."""
from dataclasses import dataclass
from pathlib import Path
import hashlib
import re

MODEL_PATTERN = re.compile(r"^[A-Za-z0-9_.-]{2,64}$")
HEX_SHA256 = re.compile(r"^[0-9a-fA-F]{64}$")
SUPPORTED_SUFFIXES = {".zip", ".img", ".bin", ".tar", ".md5"}

@dataclass(frozen=True)
class FirmwareManifest:
    brand: str
    model: str
    region: str
    expected_sha256: str
    verified_oem_source: bool

def verify_firmware(path, manifest, connected_brand, connected_model, consent=False):
    path = Path(path)
    if not isinstance(manifest, FirmwareManifest):
        raise TypeError("FirmwareManifest required")
    if not consent:
        raise PermissionError("Explicit owner consent and backup check required")
    if not manifest.verified_oem_source:
        raise PermissionError("OEM provenance has not been verified")
    if not MODEL_PATTERN.fullmatch(manifest.model):
        raise ValueError("Invalid model identifier")
    if manifest.brand.casefold() != connected_brand.casefold() or manifest.model.casefold() != connected_model.casefold():
        raise ValueError("Connected device does not match firmware manifest")
    if not HEX_SHA256.fullmatch(manifest.expected_sha256):
        raise ValueError("Invalid expected SHA-256")
    if not path.is_file() or path.suffix.lower() not in SUPPORTED_SUFFIXES:
        raise ValueError("Firmware file or extension not supported")
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    if digest.hexdigest().lower() != manifest.expected_sha256.lower():
        raise ValueError("Firmware checksum mismatch")
    return {"brand": manifest.brand, "model": manifest.model, "region": manifest.region,
            "sha256": digest.hexdigest(), "verified_hash": True,
            "signature_verified": False, "ready_to_flash": False}
