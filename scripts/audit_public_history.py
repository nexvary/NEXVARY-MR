#!/usr/bin/env python3
"""Inspect every reachable Git blob for operational data unsuitable for publication.

Complements, rather than replaces, Gitleaks. Reports paths/rules/blob IDs only,
never matched bytes. No network access, checkout, extraction or history rewrite.
"""
from __future__ import annotations
import argparse
import json
from pathlib import PurePosixPath
import re
import subprocess

LIMIT = 32 * 1024 * 1024
DATA_SUFFIXES = {'.db', '.sqlite', '.sqlite3', '.pcap', '.pcapng', '.p12', '.pfx', '.jks', '.keystore'}
PRIVATE_KEY = re.compile(rb'-----BEGIN (?:RSA |EC |DSA |OPENSSH |ENCRYPTED )?PRIVATE KEY-----')


def git(root, *args):
    return subprocess.check_output(['git', '-C', str(root), *args])


def path_rule(path):
    name = PurePosixPath(path).name.lower()
    if PurePosixPath(name).suffix in DATA_SUFFIXES:
        return 'operational-data-or-keystore-path'
    if name in {'id_rsa', 'id_ed25519', 'credentials', 'credentials.json', 'secrets.json', '.env'}:
        return 'credential-path'
    if name.startswith('.env.') and not name.endswith(('.example', '.sample', '.template')):
        return 'environment-data-path'
    return None


def content_rule(data):
    if data.startswith(b'SQLite format 3\0'):
        return 'sqlite-database-content'
    if data.startswith(b'ANDROID BACKUP\n'):
        return 'android-backup-content'
    if data[:4] in {bytes.fromhex(x) for x in ['d4c3b2a1', 'a1b2c3d4', '4d3cb2a1', 'a1b23c4d', '0a0d0d0a']}:
        return 'packet-capture-content'
    if PRIVATE_KEY.search(data):
        return 'private-key-content'
    if data.startswith(b'version https://git-lfs.github.com/spec/v1\n'):
        return 'lfs-content-requires-separate-review'
    return None


def audit(root):
    refs = git(root, 'for-each-ref', '--format=%(refname)').decode().splitlines()
    objects = git(root, 'rev-list', '--objects', '--all').splitlines()
    mapping = {line.split(b' ', 1)[0]: set() for line in objects}
    findings = []
    trees = set(git(root, 'log', '--all', '--format=%T').splitlines())
    # Enumerate each tree, not only rev-list's first name for a reused blob.
    # -z preserves non-ASCII and otherwise quoted filenames without ambiguity.
    for tree in trees:
        for entry in git(root, 'ls-tree', '-rz', '--full-tree', tree.decode()).split(b'\0'):
            if not entry:
                continue
            metadata, raw_path = entry.split(b'\t', 1)
            _, kind, oid = metadata.split()
            path = raw_path.decode('utf-8', 'backslashreplace')
            if kind == b'commit':
                findings.append({'rule': 'external-submodule-requires-review', 'path': path, 'blob': oid.decode()})
            elif kind == b'blob':
                mapping.setdefault(oid, set()).add(path)
    proc = subprocess.Popen(['git', '-C', str(root), 'cat-file', '--batch'], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    blobs = 0
    try:
        for oid, paths in mapping.items():
            proc.stdin.write(oid + b'\n'); proc.stdin.flush()
            header = proc.stdout.readline().split()
            if len(header) != 3 or header[0] != oid:
                raise RuntimeError('Unexpected Git object response')
            kind, size = header[1], int(header[2])
            remaining = size; chunks = []
            while remaining:
                block = proc.stdout.read(min(1024 * 1024, remaining))
                if not block:
                    raise RuntimeError('Truncated Git object')
                if size <= LIMIT:
                    chunks.append(block)
                remaining -= len(block)
            if proc.stdout.read(1) != b'\n':
                raise RuntimeError('Unexpected Git object framing')
            if kind != b'blob':
                continue
            blobs += 1
            rules = []
            for path in sorted(paths):
                rule = path_rule(path)
                if rule:
                    findings.append({'rule': rule, 'path': path, 'blob': oid.decode()})
            if size > LIMIT:
                rules.append('oversize-blob-requires-separate-review')
            else:
                rule = content_rule(b''.join(chunks))
                if rule:
                    rules.append(rule)
            for rule in rules:
                findings.append({'rule': rule, 'paths': sorted(paths), 'blob': oid.decode()})
    finally:
        proc.stdin.close()
        proc.stdout.close()
        proc.wait()
    if proc.returncode:
        raise RuntimeError('Git object reader failed')
    return {'schemaVersion': 1, 'head': git(root, 'rev-parse', 'HEAD').decode().strip(),
            'scope': 'All locally fetched refs, reachable Git blobs; no unreachable/remote-deleted objects or external systems.',
            'refs': refs, 'reachableCommits': int(git(root, 'rev-list', '--all', '--count')),
            'objects': len(mapping), 'blobs': blobs, 'findings': findings,
            'notice': 'Heuristic operational-data guard plus independent Gitleaks required. Zero findings does not certify absence of personal data or unknown secrets.'}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', default='.')
    parser.add_argument('--report')
    args = parser.parse_args()
    result = audit(args.repo)
    text = json.dumps(result, ensure_ascii=True, indent=2) + '\n'
    if args.report:
        from pathlib import Path
        Path(args.report).write_text(text, encoding='utf-8')
    print(f"Public-history guard: {result['reachableCommits']} reachable commits; {result['blobs']} blobs; {len(result['findings'])} findings")
    return int(bool(result['findings']))


if __name__ == '__main__':
    raise SystemExit(main())
