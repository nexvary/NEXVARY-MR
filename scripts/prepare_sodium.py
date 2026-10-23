#!/usr/bin/env python3
"""Prepare a pinned, upstream-minisign-reviewed crypto dependency (build only).

The SHA256 pins were established after verifying each official archive and its
trusted comment with minisign and the key published by libsodium. CI checks the
reviewed archive hash before extraction or executing configure. No implicit
dependency downloads occur in CMake or in the application.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import platform
import signal
import subprocess
import tarfile
import tempfile
import time
import urllib.request
import zipfile

VERSION = "1.0.22"
ARCHIVES = {
    "linux": ("libsodium-1.0.22.tar.gz", "adbdd8f16149e81ac6078a03aca6fc03b592b89ef7b5ed83841c086191be3349"),
    "windows": ("libsodium-1.0.22-msvc.zip", "3e03a726fac4bc09cb61d8f29d658ef7a5eca0811de59082130414f7ca2e4279"),
}
BASE_URL = "https://download.libsodium.org/libsodium/releases/"
MAX_ARCHIVE = 32 * 1024 * 1024


def digest(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def checked_archive(path, expected):
    if not path.is_file() or path.stat().st_size > MAX_ARCHIVE or digest(path) != expected:
        raise ValueError("libsodium archive SHA256 mismatch or size limit")


def relative_name(name):
    path = PurePosixPath(name)
    if not name or "\\" in name or path.is_absolute() or ".." in path.parts or ":" in name:
        raise ValueError("unsafe dependency archive path")
    return path


def fetch_archive(cache, filename, expected):
    cache.mkdir(parents=True, exist_ok=True)
    path = cache / filename
    if path.exists():
        checked_archive(path, expected)
        return path
    partial = path.with_suffix(path.suffix + ".partial")
    for attempt in range(3):
        try:
            with urllib.request.urlopen(BASE_URL + filename, timeout=30) as response, partial.open("wb") as output:
                count = 0
                while chunk := response.read(1024 * 1024):
                    count += len(chunk)
                    if count > MAX_ARCHIVE:
                        raise ValueError("dependency download size limit")
                    output.write(chunk)
            checked_archive(partial, expected)
            partial.replace(path)
            return path
        except (OSError, TimeoutError):
            partial.unlink(missing_ok=True)
            if attempt == 2:
                raise
            time.sleep(attempt + 1)


def extract_windows(archive, destination):
    selected = {}
    with zipfile.ZipFile(archive) as bundle:
        for entry in bundle.infolist():
            path = relative_name(entry.filename)
            if entry.is_dir():
                continue
            if entry.filename.startswith("libsodium/include/"):
                output = PurePosixPath(*path.parts[1:])
            elif entry.filename in {"libsodium/x64/Release/v143/dynamic/libsodium.dll",
                                    "libsodium/x64/Release/v143/dynamic/libsodium.lib"}:
                output = PurePosixPath("bin" if path.suffix == ".dll" else "lib") / path.name
            else:
                continue
            if output in selected or entry.file_size > 8 * 1024 * 1024 or entry.external_attr >> 16 & 0o170000 == 0o120000:
                raise ValueError("invalid dependency archive member")
            selected[output] = entry
        if sum(entry.file_size for entry in selected.values()) > 16 * 1024 * 1024:
            raise ValueError("dependency extraction size limit")
        for name in ("include/sodium.h", "lib/libsodium.lib", "bin/libsodium.dll"):
            if PurePosixPath(name) not in selected:
                raise ValueError("release x64 dependency member missing")
        for relative, entry in selected.items():
            output = destination.joinpath(*relative.parts)
            output.parent.mkdir(parents=True, exist_ok=True)
            output.write_bytes(bundle.read(entry))


def build_source(archive, destination):
    with tempfile.TemporaryDirectory(prefix="nexvary-sodium-", dir=destination.parent) as scratch:
        scratch = Path(scratch)
        with tarfile.open(archive) as bundle:
            members = bundle.getmembers()
            if len(members) > 6000 or sum(entry.size for entry in members) > 64 * 1024 * 1024:
                raise ValueError("source archive limits")
            for entry in members:
                relative_name(entry.name)
                if not entry.isdir() and not entry.isfile():
                    raise ValueError("nonregular source archive member")
            bundle.extractall(scratch, filter="data")
        sources = [path for path in scratch.iterdir() if path.is_dir()]
        if len(sources) != 1:
            raise ValueError("unexpected source archive root")
        source = sources[0]
        (destination / "LICENSE").write_bytes((source / "LICENSE").read_bytes())
        environment = dict(os.environ, CFLAGS="-O2", CPPFLAGS="", LDFLAGS="")
        commands = [[str(source / "configure"), "--prefix=" + str(destination), "--disable-shared", "--enable-static", "--with-pic"],
                    ["make", "-j2"], ["make", "check", "-j2"], ["make", "install"]]
        with (destination / "build.log").open("w") as log:
            for command in commands:
                child = subprocess.Popen(command, cwd=source, env=environment, stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
                try:
                    result = child.wait(timeout=240)
                except subprocess.TimeoutExpired:
                    os.killpg(child.pid, signal.SIGKILL)
                    child.wait()
                    raise RuntimeError("dependency build/check deadline exceeded") from None
                if result != 0:
                    raise RuntimeError("dependency build/check failed; see build.log")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--destination", required=True, type=Path)
    parser.add_argument("--cache", type=Path)
    args = parser.parse_args()
    target = "windows" if os.name == "nt" else "linux"
    if target == "windows" and platform.machine().lower() not in {"amd64", "x86_64"}:
        parser.error("Only x64 Windows dependency builds are approved")
    destination = args.destination.resolve()
    if destination.exists() and any(destination.iterdir()):
        parser.error("Destination must be new or empty; existing files are not replaced")
    destination.mkdir(parents=True, exist_ok=True)
    filename, expected = ARCHIVES[target]
    archive = fetch_archive(args.cache or destination.parent / "sodium-downloads", filename, expected)
    if target == "windows":
        extract_windows(archive, destination)
    else:
        build_source(archive, destination)
    files = {str(path.relative_to(destination)).replace("\\", "/"): digest(path)
             for folder in ("include", "lib", "bin") if (destination / folder).exists()
             for path in (destination / folder).rglob("*") if path.is_file()}
    receipt = {"schemaVersion": 1, "version": VERSION, "source": BASE_URL + filename,
               "archiveSha256": expected, "platform": target, "files": files,
               "upstreamSignatureReviewed": True, "ciIntegrityCheck": "pinned-sha256",
               "upstreamSelfTestsRun": target == "linux"}
    (destination / "build-receipt.json").write_text(json.dumps(receipt, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"prepared": str(destination), "version": VERSION, "archiveSha256": expected}))


if __name__ == "__main__":
    main()
