#!/usr/bin/env python3
"""Build-time only: prepare the unchanged official pinned Windows scrcpy runtime."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import shutil
import stat
import urllib.request
import zipfile

VERSION = '5.0.1'
SHA256 = 'b12a2c4ee8be317422451fc7dcf8ee20a71b5ea7ef9ad73ddd825a25316227a5'
URL = 'https://github.com/Genymobile/scrcpy/releases/download/v5.0.1/scrcpy-win64-v5.0.1.zip'
MAX_ARCHIVE = 16 * 1024 * 1024
MAX_EXPANDED = 64 * 1024 * 1024
ROOT = 'scrcpy-win64-v5.0.1'
REQUIRED = {'scrcpy.exe', 'adb.exe', 'scrcpy-server', 'AdbWinApi.dll', 'AdbWinUsbApi.dll', 'SDL3.dll', 'LICENSE.txt'}


def digest(path):
    with Path(path).open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def checked_members(bundle):
    members = bundle.infolist()
    if len(members) > 40 or sum(m.file_size for m in members) > MAX_EXPANDED:
        raise ValueError('Screen-tool archive exceeds bounds')
    names = set()
    files = []
    for member in members:
        path = PurePosixPath(member.filename)
        mode = member.external_attr >> 16
        if ('\\' in member.filename or ':' in member.filename or path.is_absolute()
                or '..' in path.parts or any(ord(c) < 32 for c in member.filename)
                or not path.parts or path.parts[0] != ROOT
                or (mode and stat.S_IFMT(mode) not in (0, stat.S_IFREG, stat.S_IFDIR))):
            raise ValueError('Unsafe screen-tool archive entry')
        if member.is_dir():
            if path.parts != (ROOT,):
                raise ValueError('Unexpected nested directory')
            continue
        if len(path.parts) != 2 or path.name.lower() in names or member.flag_bits & 1:
            raise ValueError('Duplicate, encrypted or nested screen-tool entry')
        names.add(path.name.lower())
        files.append((member, path.name))
    if not REQUIRED <= {name for _, name in files}:
        raise ValueError('Incomplete official screen-tool runtime')
    return files


def prepare(archive, destination):
    archive, destination = Path(archive), Path(destination)
    if (not archive.is_file() or archive.is_symlink() or archive.stat().st_size > MAX_ARCHIVE
            or digest(archive) != SHA256):
        raise ValueError('Screen-tool archive SHA-256 mismatch or input limit')
    if destination.exists() or destination.is_symlink():
        raise ValueError('Screen-tool destination must be new')
    with zipfile.ZipFile(archive) as bundle:
        files = checked_members(bundle)
        destination.mkdir(parents=True)
        try:
            for member, name in files:
                with bundle.open(member) as source, (destination / name).open('xb') as target:
                    shutil.copyfileobj(source, target, length=65536)
            receipt = {'schemaVersion': 1, 'version': VERSION, 'source': URL,
                       'archiveSha256': SHA256, 'runtimeConnections': 'none during preparation',
                       'provenance': 'Pinned bytes from official release, not Authenticode verification',
                       'files': {name: digest(destination / name) for _, name in files}}
            (destination / 'SOURCE_PROVENANCE.json').write_text(json.dumps(receipt, indent=2) + '\n', encoding='utf-8')
        except BaseException:
            shutil.rmtree(destination)
            raise
    return receipt


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--destination', type=Path, required=True)
    parser.add_argument('--archive', type=Path)
    parser.add_argument('--cache', type=Path)
    args = parser.parse_args()
    archive = args.archive
    if archive is None:
        cache = args.cache or args.destination.parent / 'screen-tool-downloads'
        cache.mkdir(parents=True, exist_ok=True)
        archive = cache / 'scrcpy-win64-v5.0.1.zip'
        if not archive.exists():
            partial = archive.with_suffix('.partial')
            try:
                with urllib.request.urlopen(URL, timeout=30) as source, partial.open('xb') as target:
                    total = 0
                    while block := source.read(65536):
                        total += len(block)
                        if total > MAX_ARCHIVE:
                            raise ValueError('Screen-tool download exceeds limit')
                        target.write(block)
                if digest(partial) != SHA256:
                    raise ValueError('Screen-tool download SHA-256 mismatch')
                partial.rename(archive)
            finally:
                partial.unlink(missing_ok=True)
    receipt = prepare(archive, args.destination)
    print(json.dumps({'prepared': str(args.destination), 'version': receipt['version'], 'archiveSha256': SHA256}))


if __name__ == '__main__':
    main()
