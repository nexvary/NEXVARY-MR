"""Verify an evaluation artifact before extracting into a new private temp folder.

Downloads only the supplied artifact URL. Does not execute or install anything.
The provider digest must come from separately inspected GitHub metadata.
"""
import argparse
import hashlib
import io
import json
from pathlib import Path, PurePosixPath
import re
import shutil
import stat
import tempfile
import urllib.request
import zipfile


def members(archive):
    result = {}
    total = 0
    for item in archive.infolist():
        name = item.filename.replace("\\", "/")
        path = PurePosixPath(name)
        if (len(result) >= 5000 or path.is_absolute() or ":" in name
                or any(p in (".", "..") or p.rstrip(" .") != p for p in name.rstrip("/").split("/"))
                or name.casefold() in {n.casefold() for n in result}
                or stat.S_ISLNK(item.external_attr >> 16)
                or item.file_size > 256 * 1024**2):
            raise ValueError("Unsafe or oversized archive member")
        total += item.file_size
        if total > 2 * 1024**3:
            raise ValueError("Expanded archive exceeds limit")
        result[name] = item
    return result


def prepare(url, digest, source):
    if not re.fullmatch(r"[a-f0-9]{64}", digest) or not re.fullmatch(r"[a-f0-9]{40}", source):
        raise ValueError("Expected pinned source and digest")
    root = Path(tempfile.mkdtemp(prefix="NEXVARY-MR-Evaluation-"))
    downloaded = root / "provider-artifact.zip"
    observed = hashlib.sha256()
    size = 0
    with urllib.request.urlopen(url, timeout=60) as response, downloaded.open("xb") as output:
        while chunk := response.read(1024**2):
            size += len(chunk)
            if size > 512 * 1024**2:
                raise ValueError("Provider archive exceeds limit")
            output.write(chunk)
            observed.update(chunk)
    if observed.hexdigest() != digest:
        raise ValueError("Provider ZIP digest mismatch")
    with zipfile.ZipFile(downloaded) as outer:
        outer_members = members(outer)
        nested = [n for n in outer_members if n.endswith("-portable-unsigned.zip")]
        if len(nested) != 1:
            raise ValueError("Expected one portable bundle")
        data = outer.read(outer_members[nested[0]])
    with zipfile.ZipFile(io.BytesIO(data)) as archive:
        entries = {n: e for n, e in members(archive).items() if not e.is_dir()}
        prefix = "NEXVARY-MR/"
        if any(not n.startswith(prefix) for n in entries):
            raise ValueError("Unexpected bundle root")
        profile = json.loads(archive.read(entries[prefix + "BUILD_PROFILE.json"]).decode("utf-8-sig"))
        if profile["commit"] != source or profile["buildProfile"] != "production-read-only" or profile["signed"] is not False:
            raise ValueError("Unexpected source/profile/signing metadata")
        manifest = archive.read(entries[prefix + "SHA256SUMS.txt"]).decode("utf-8-sig")
        declared = {}
        for line in manifest.splitlines():
            match = re.fullmatch(r"([a-f0-9]{64})  (.+)", line)
            if not match:
                raise ValueError("Malformed checksum manifest")
            name = prefix + match[2].replace("\\", "/")
            if name not in entries or name in declared:
                raise ValueError("Missing or duplicate checksum member")
            declared[name] = match[1]
        if set(declared) != set(entries) - {prefix + "SHA256SUMS.txt"}:
            raise ValueError("Incomplete checksum manifest")
        for name, expected in declared.items():
            with archive.open(entries[name]) as stream:
                if hashlib.file_digest(stream, "sha256").hexdigest() != expected:
                    raise ValueError("Inner file digest mismatch")
        # No executable is launched. Existing directories/files are never reused.
        destination = root / "portable"
        destination.mkdir()
        for name, entry in entries.items():
            target = destination.joinpath(*PurePosixPath(name).parts)
            target.parent.mkdir(parents=True, exist_ok=True)
            with archive.open(entry) as stream, target.open("xb") as output:
                shutil.copyfileobj(stream, output, 1024**2)
        return {"source": source, "providerSha256": digest,
                "bundleManifestFilesVerified": len(declared),
                "executableSha256": declared[prefix + "NEXVARY-MR.exe"],
                "importWorkerSha256": declared[prefix + "mr_evidence_import_worker.exe"],
                "testRoot": str(root), "bundleDirectory": str(destination / "NEXVARY-MR"),
                "executed": False, "signed": False}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--url", required=True)
    parser.add_argument("--provider-sha256", required=True)
    parser.add_argument("--source", required=True)
    arguments = parser.parse_args()
    print(json.dumps(prepare(arguments.url, arguments.provider_sha256, arguments.source), indent=2))
