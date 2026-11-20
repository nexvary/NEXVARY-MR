"""Synthetic, non-destructive recovery evidence manifest utility.

Only hashes files already accessible to the operator. Never accesses devices,
private app storage or encrypted partitions.
"""
from __future__ import annotations

import hashlib
import json
from pathlib import Path


def sha256_file(path: Path, chunk_size: int = 1024 * 1024) -> str:
    if chunk_size <= 0:
        raise ValueError("chunk_size must be positive")
    digest = hashlib.sha256()
    with Path(path).open("rb") as source:
        for chunk in iter(lambda: source.read(chunk_size), b""):
            digest.update(chunk)
    return digest.hexdigest()


def compare_manifest(original: dict[str, str], recovered: dict[str, str]) -> dict[str, str]:
    """Classify names as verified, hash-mismatch, or not-recovered.

    The manifests must be derived from independently acquired datasets;
    a matching filename alone is never proof of recovery.
    """
    result: dict[str, str] = {}
    for name, expected in original.items():
        observed = recovered.get(name)
        result[name] = ("not-recovered" if observed is None else
                        "verified" if observed == expected else "hash-mismatch")
    return result


def build_manifest(directory: Path) -> dict[str, str]:
    root = Path(directory).resolve(strict=True)
    if not root.is_dir():
        raise NotADirectoryError(root)
    files = sorted(p for p in root.rglob("*") if p.is_file() and not p.is_symlink())
    return {p.relative_to(root).as_posix(): sha256_file(p) for p in files}


def write_manifest(directory: Path, output: Path) -> None:
    root = Path(directory).resolve(strict=True)
    destination = Path(output).resolve()
    if destination == root or root in destination.parents:
        raise ValueError("manifest output must be outside evidence directory")
    data = build_manifest(root)
    destination.write_text(json.dumps(data, indent=2, sort_keys=True) + "\n", encoding="utf-8")
