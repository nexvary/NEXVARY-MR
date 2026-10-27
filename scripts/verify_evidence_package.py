#!/usr/bin/env python3
"""Offline, standard-library verifier for a NEXVARY working-copy package.

Supply the manifest digest retained through a separate trusted channel. A matching
hash proves byte integrity relative to that digest, not provenance or legal scope.
This script never opens evidence with a parser or executes it.
"""
import argparse
import hashlib
import hmac
import json
import os
from pathlib import Path
import re
import stat
import sys
import uuid

LIMIT = 16 * 1024 * 1024


def regular_bytes(path, limit):
    if path.is_symlink():
        raise ValueError("Symbolic links are not accepted")
    flags = os.O_RDONLY | getattr(os, "O_BINARY", 0) | getattr(os, "O_NOFOLLOW", 0) | getattr(os, "O_NONBLOCK", 0)
    fd = os.open(path, flags)
    with os.fdopen(fd, "rb") as stream:
        info = os.fstat(stream.fileno())
        if not stat.S_ISREG(info.st_mode) or not 0 < info.st_size <= limit:
            raise ValueError("Expected a nonempty bounded regular file")
        data = stream.read(limit + 1)
        if len(data) != info.st_size or len(data) > limit:
            raise ValueError("File changed size or exceeds limit")
        return data


def unique_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError("Duplicate JSON field")
        result[key] = value
    return result


def verify_package(directory, expected_manifest_sha256):
    if not re.fullmatch(r"[0-9a-fA-F]{64}", expected_manifest_sha256):
        raise ValueError("A separately retained 64-character manifest SHA-256 is required")
    root = Path(directory)
    if root.is_symlink() or not root.is_dir():
        raise ValueError("Expected a package directory, not a symbolic link")
    if {p.name for p in root.iterdir()} != {"manifest.json", "evidence.bin"}:
        raise ValueError("Unexpected or missing package files")
    data = regular_bytes(root / "manifest.json", 64 * 1024)
    if not hmac.compare_digest(hashlib.sha256(data).hexdigest(), expected_manifest_sha256.lower()):
        raise ValueError("Manifest digest does not match the separately retained value")
    def invalid_constant(_):
        raise ValueError("Non-finite JSON number")
    manifest = json.loads(data.decode("utf-8"), object_pairs_hook=unique_object, parse_constant=invalid_constant)
    if not isinstance(manifest, dict) or manifest.get("schema") != "nexvary-evidence-package/v1":
        raise ValueError("Unsupported manifest schema")
    for key in ("caseId", "packageId", "evidenceId", "deviceId"):
        value = manifest.get(key)
        if not isinstance(value, str) or str(uuid.UUID(value)) != value:
            raise ValueError("Invalid package/case/evidence/device UUID")
    revision = manifest.get("caseRevision")
    if type(revision) is not int or not 1 <= revision <= 10000 or type(manifest.get("synthetic")) is not bool:
        raise ValueError("Invalid source revision or data profile")
    if not isinstance(manifest.get("auditHead"), str) or not re.fullmatch(r"[0-9a-f]{64}", manifest["auditHead"]):
        raise ValueError("Invalid audit anchor digest")
    for key in ("actor", "purpose", "exportedAtUtc", "sourceName", "trustBoundary"):
        if not isinstance(manifest.get(key), str) or not manifest[key] or len(manifest[key].encode("utf-8")) > 8192:
            raise ValueError("Invalid manifest description")
    files = manifest.get("files")
    if not isinstance(files, list) or len(files) != 1 or not isinstance(files[0], dict):
        raise ValueError("Expected one exported working copy")
    item = files[0]
    if set(item) != {"path", "size", "sha256"} or item["path"] != "evidence.bin":
        raise ValueError("Invalid working-copy path or fields")
    if type(item["size"]) is not int or not 0 < item["size"] <= LIMIT:
        raise ValueError("Invalid evidence size")
    if not isinstance(item["sha256"], str) or not re.fullmatch(r"[0-9a-f]{64}", item["sha256"]):
        raise ValueError("Invalid evidence digest")
    evidence = regular_bytes(root / "evidence.bin", LIMIT)
    if len(evidence) != item["size"] or not hmac.compare_digest(hashlib.sha256(evidence).hexdigest(), item["sha256"]):
        raise ValueError("Working-copy SHA-256/size mismatch")
    return {"integrityVerified": True, "caseId": manifest["caseId"], "evidenceId": manifest["evidenceId"],
            "sourceRevision": revision, "synthetic": manifest["synthetic"], "externalAuthenticityVerified": False}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory")
    parser.add_argument("--manifest-sha256", required=True)
    args = parser.parse_args()
    try:
        result = verify_package(args.directory, args.manifest_sha256)
    except (ValueError, OSError, UnicodeError, RecursionError) as error:
        print("Verification failed: " + str(error), file=sys.stderr)
        return 1
    print(json.dumps(result, sort_keys=True))
    return 0


if __name__ == "__main__":
    sys.exit(main())
