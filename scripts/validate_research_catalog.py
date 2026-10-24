#!/usr/bin/env python3
"""Offline data gate; catalog assertions cannot grant a native capability."""
import datetime
import json
import sys
from pathlib import Path
from urllib.parse import urlsplit

ROOT = Path(__file__).resolve().parents[1]
STATUSES = {'Vendor Claimed', 'Publicly Documented', 'Research Hypothesis', 'Historical', 'Unsupported'}


def validate(root=ROOT):
    errors = []
    path = Path(root) / 'research/program/catalog.json'
    try:
        if path.stat().st_size > 1024 * 1024:
            return ['catalog exceeds 1 MiB']
        data = json.loads(path.read_text(encoding='utf-8'))
    except (ValueError, OSError) as exc:
        return [str(exc)]
    if not isinstance(data, dict) or data.get('schemaVersion') != 1:
        return ['invalid catalog schema']
    ids = set()
    sources = data.get('sources', [])
    if not isinstance(sources, list):
        return ['invalid sources']
    for source in sources:
        if not isinstance(source, dict):
            errors.append('source must be an object')
            continue
        sid = source.get('id')
        if not isinstance(sid, str) or not sid or sid in ids:
            errors.append('missing/duplicate source identifier')
        else:
            ids.add(sid)
        try:
            url = urlsplit(source.get('url', ''))
            if url.scheme != 'https' or not url.hostname or url.username or url.password:
                errors.append('invalid source URL')
            datetime.date.fromisoformat(source.get('reviewedOn', ''))
        except (ValueError, TypeError):
            errors.append('invalid source URL/review date')
        if not all(isinstance(source.get(k), str) and source[k].strip() for k in ('publisher', 'rights', 'review', 'publishedOn')):
            errors.append('missing source attribution/rights/review')
    seen = set()
    for category in ('atlas', 'tools', 'experiments'):
        rows = data.get(category)
        if not isinstance(rows, list) or not 1 <= len(rows) <= 500:
            errors.append(f'invalid category: {category}')
            continue
        for row in rows:
            if not isinstance(row, dict):
                errors.append('record must be an object')
                continue
            rid = row.get('id')
            if not isinstance(rid, str) or not rid or rid in seen:
                errors.append('missing/duplicate research identifier')
            else:
                seen.add(rid)
            if row.get('status') not in STATUSES:
                errors.append('catalog cannot assert Lab Verified or unknown evidence class')
            refs = row.get('sourceIds')
            if not isinstance(refs, list) or not refs or any(not isinstance(ref, str) or ref not in ids for ref in refs):
                errors.append('unknown/missing research provenance')
            required = ['title', 'limitations']
            if category == 'tools':
                required += ['company', 'advertised', 'independentEvidence', 'architecture', 'interfaces', 'processing', 'reporting', 'strengths', 'originalDevelopment', 'requiredExperiment']
            if category == 'experiments':
                required += ['question', 'hypothesis', 'deviceModel', 'osFirmware', 'environment', 'equipment', 'authorizedScope', 'methodology', 'expected', 'actualResults', 'reproducibility', 'researcher', 'reviewStatus']
            if not all(isinstance(row.get(k), str) and row[k].strip() for k in required):
                errors.append(f'incomplete {category} record: {rid}')
    if data.get('compatibility') != []:
        errors.append('seed catalog must not invent physical compatibility results')
    return errors


if __name__ == '__main__':
    failures = validate()
    print('\n'.join(failures) if failures else 'Research catalog: offline validation passed')
    sys.exit(bool(failures))
