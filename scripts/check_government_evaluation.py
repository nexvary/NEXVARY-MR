#!/usr/bin/env python3
"""Independent, synthetic known-answer check of the existing import worker.

Does not exercise case authorization, UI, storage, timeline, PDF or acquisition.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--worker', type=Path, required=True)
    reference_root = Path(__file__).resolve().parents[1] / 'samples/government-evaluation'
    if not reference_root.is_dir():
        reference_root = Path(__file__).resolve().parents[1] / 'evaluation-samples'
    parser.add_argument('--samples', type=Path, default=reference_root)
    args = parser.parse_args()
    truth = json.loads((args.samples / 'ground-truth.json').read_text(encoding='utf-8'))
    source = (args.samples / 'evidence.json').read_bytes()
    digest = hashlib.sha256(source).hexdigest()
    if digest != truth['sourceSha256'] or len(source) != truth['sourceSize']:
        raise ValueError('Reference source identity mismatch')
    def invoke(name, sha):
        return subprocess.run([str(args.worker.resolve()), '--input', str((args.samples / name).resolve()), '--sha256', sha, '--adapter', 'json'], capture_output=True, timeout=10, check=False)
    run = invoke('evidence.json', digest)
    if run.returncode or len(run.stdout) > 60 * 1024:
        raise ValueError('Reference worker failed or output limit exceeded')
    report = json.loads(run.stdout)
    for key, expected in [('schema', 'nexvary-authorized-import/v1'), ('status', 'imported'), ('adapter', 'json'), ('sourceSha256', digest), ('sourceSize', len(source))]:
        if report.get(key) != expected:
            raise ValueError('Reference report field mismatch: ' + key)
    observed = [{key: row.get(key) for key in ('reference', 'kind', 'value')} for row in report['findings']]
    if observed != truth['expectedFindings'] or any(row.get('valueTruncated') is not False for row in report['findings']):
        raise ValueError('Finding loss, addition, reordering or value mismatch')
    again = invoke('evidence.json', digest)
    if again.returncode or again.stdout != run.stdout:
        raise ValueError('Non-reproducible parser report')
    wrong_hash = invoke('evidence.json', '0' * 64)
    duplicate = (args.samples / 'duplicate-key.json').read_bytes()
    if hashlib.sha256(duplicate).hexdigest() != truth['negativeSha256']:
        raise ValueError('Negative source identity mismatch')
    rejected = invoke('duplicate-key.json', truth['negativeSha256'])
    if wrong_hash.returncode == 0 or rejected.returncode == 0:
        raise ValueError('Invalid input accepted')
    print(json.dumps({'schema': 'nexvary-government-reference-check/v1', 'sourceSha256': digest, 'reportSha256': hashlib.sha256(run.stdout).hexdigest(), 'findings': len(observed), 'missing': 0, 'unexpected': 0, 'deterministicRepeat': True, 'wrongHashRefused': True, 'duplicateKeyRefused': True, 'scope': 'Direct synthetic import-worker check only; not full product or competitor acceptance.'}, sort_keys=True))


if __name__ == '__main__':
    main()
