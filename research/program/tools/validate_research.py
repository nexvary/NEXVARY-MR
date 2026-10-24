#!/usr/bin/env python3
"""Offline validator for the NEXVARY research registry; standard library only."""
from pathlib import Path
import json
import sys
from urllib.parse import urlparse

ROOT = Path(__file__).resolve().parents[1]
STATUSES = {'verified', 'documented', 'hypothesis', 'rejected'}


def validate(root=ROOT):
    errors = []
    def load(relative):
        try:
            return json.loads((root / relative).read_text(encoding='utf-8'))
        except (OSError, ValueError) as exc:
            errors.append(f'{relative}: {exc}')
            return {}

    sources = load('research/sources/registry.json').get('sources', [])
    ids = set()
    for source in sources:
        sid = source.get('id')
        if not sid or sid in ids:
            errors.append(f'duplicate/missing source id: {sid}')
        ids.add(sid)
        url = urlparse(source.get('url', ''))
        if url.scheme != 'https' or not url.netloc:
            errors.append(f'invalid source URL: {sid}')
        if not source.get('publisher') or not source.get('title'):
            errors.append(f'missing source attribution: {sid}')
    entries = load('research/knowledge/initial-research.json').get('entries', [])
    seen = set()
    for entry in entries:
        eid = entry.get('id')
        if not eid or eid in seen:
            errors.append(f'duplicate/missing knowledge id: {eid}')
        seen.add(eid)
        if entry.get('status') not in STATUSES:
            errors.append(f'invalid status: {eid}')
        if not entry.get('source_ids') or any(s not in ids for s in entry['source_ids']):
            errors.append(f'unknown/missing source reference: {eid}')
        if not entry.get('limitations') or not entry.get('claim'):
            errors.append(f'missing claim/limitations: {eid}')
        if entry.get('status') == 'verified':
            errors.append(f'{eid}: verified claims require an attached reviewed experiment; use documented until linked')
    matrix = load('research/compatibility/matrix.json')
    required = matrix.get('required_fields', [])
    for row in matrix.get('entries', []):
        missing = [key for key in required if not row.get(key)]
        if missing:
            errors.append(f'compatibility entry missing: {missing}')
    return errors


if __name__ == '__main__':
    failures = validate()
    if failures:
        print('\n'.join(failures), file=sys.stderr)
        sys.exit(1)
    print('NEXVARY research registry: validation passed')
