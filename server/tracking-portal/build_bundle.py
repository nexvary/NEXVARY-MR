"""Produce a single reviewable self-extracting installer; no embedded credentials."""
import argparse
import base64
import hashlib
import io
import tarfile
from pathlib import Path

FILES = ['install.sh', 'render.py', 'portal_app.py', 'portal_admin.py', 'gunicorn.conf.py', 'requirements.lock', 'README_AR.md']


def build(destination):
    root = Path(__file__).resolve().parent
    archive = io.BytesIO()
    with tarfile.open(fileobj=archive, mode='w:gz') as out:
        for name in FILES:
            content = (root / name).read_bytes()
            info = tarfile.TarInfo(name)
            info.size = len(content)
            info.mode = 0o644
            info.mtime = 0
            out.addfile(info, io.BytesIO(content))
    payload = archive.getvalue()
    sha = hashlib.sha256(payload).hexdigest()
    bootstrap = '''#!/usr/bin/env bash
# NEXVARY MR customer-tracking server installer. Read this script before running.
# Embedded payload SHA256: DIGEST
set -Eeuo pipefail
umask 077
bundle_tmp=$(mktemp -d)
trap 'rm -rf -- "$bundle_tmp"' EXIT
script_path=${BASH_SOURCE[0]}
awk '/^__NEXVARY_PAYLOAD_BELOW__$/ {found=1;next} found {print}' "$script_path" | base64 --decode > "$bundle_tmp/payload.tar.gz"
printf '%s  %s\\n' 'DIGEST' "$bundle_tmp/payload.tar.gz" | sha256sum --check --status
tar -xzf "$bundle_tmp/payload.tar.gz" -C "$bundle_tmp"
bash "$bundle_tmp/install.sh" "$@"
exit $?
__NEXVARY_PAYLOAD_BELOW__
'''.replace('DIGEST', sha)
    encoded = base64.encodebytes(payload).decode('ascii')
    path = Path(destination)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(bootstrap + encoded)
    path.chmod(0o755)
    print(path)
    print('Installer SHA256:', hashlib.sha256(path.read_bytes()).hexdigest())


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('output')
    build(parser.parse_args().output)
