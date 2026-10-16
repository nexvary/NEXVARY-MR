"""Read-only signature discovery for authorized, already-acquired evidence images.

This scanner reports candidate offsets only; it does not claim recovered files.
No direct block device access or modifications are performed.
"""
from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import stat

SIGNATURES: dict[str, bytes] = {
    "jpeg": bytes.fromhex("ffd8ff"),
    "png": bytes.fromhex("89504e470d0a1a0a"),
    "pdf": b"%PDF-",
    "zip": bytes.fromhex("504b0304"),
    "sqlite": b"SQLite format 3" + bytes([0]),
}
MAX_MATCHES = 10000
MAX_SCAN_BYTES = 4 * 1024 * 1024 * 1024


@dataclass(frozen=True)
class Candidate:
    format: str
    offset: int


def scan_image(path: Path, *, chunk_size: int = 1024 * 1024,
               max_bytes: int = MAX_SCAN_BYTES,
               max_matches: int = MAX_MATCHES) -> list[Candidate]:
    """Scan a regular evidence file with overlap for boundary-crossing signatures."""
    if chunk_size < max(map(len, SIGNATURES.values())) or chunk_size > 16 * 1024 * 1024:
        raise ValueError("Invalid chunk size")
    if not 0 < max_bytes <= MAX_SCAN_BYTES or not 0 < max_matches <= MAX_MATCHES:
        raise ValueError("Invalid scan limits")
    path = Path(path)
    if path.is_symlink() or not path.is_file():
        raise ValueError("Input must be a regular file, not a symlink or device")
    if path.stat().st_size > max_bytes:
        raise ValueError("Image exceeds configured scan limit")
    max_signature = max(map(len, SIGNATURES.values()))
    matches: list[Candidate] = []
    previous = b""
    processed = 0
    with path.open("rb") as stream:
        if not stat.S_ISREG(os.fstat(stream.fileno()).st_mode):
            raise ValueError("Not a regular file")
        while True:
            chunk = stream.read(chunk_size)
            if not chunk:
                break
            data = previous + chunk
            start_offset = processed - len(previous)
            for kind, signature in SIGNATURES.items():
                position = data.find(signature)
                while position >= 0:
                    offset = start_offset + position
                    if offset + len(signature) > processed:
                        matches.append(Candidate(kind, offset))
                        if len(matches) > max_matches:
                            raise ValueError("Too many candidates")
                    position = data.find(signature, position + 1)
            processed += len(chunk)
            previous = data[-(max_signature - 1):]
    return sorted(matches, key=lambda candidate: (candidate.offset, candidate.format))
