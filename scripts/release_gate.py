#!/usr/bin/env python3
"""Fail-closed RC evidence gate. Evidence is reviewed, never inferred from CI alone."""
import argparse
import json
import re
from pathlib import Path

REQUIRED = (
    'exact_head_ci', 'production_policy', 'android_physical_read_only',
    'windows_10_acceptance', 'windows_11_acceptance', 'installer_portable_uninstall',
    'signing', 'sbom_review', 'upgrade_rollback', 'privacy_review', 'reviewer_signoff',
)

def blockers(document, expected_sha):
    result = []
    if not re.fullmatch(r'[a-f0-9]{40}', expected_sha):
        return ['Expected commit must be a full lowercase SHA-1']
    if document.get('schemaVersion') != 1 or document.get('commit') != expected_sha:
        result.append('Evidence schema/commit does not match the requested candidate')
    if document.get('buildProfile') != 'production-read-only':
        result.append('Only the production-read-only profile can be a production RC')
    evidence = document.get('gates', {})
    if not isinstance(evidence, dict):
        evidence = {}
    for name in REQUIRED:
        row = evidence.get(name, {})
        if (not isinstance(row, dict) or row.get('passed') is not True
                or row.get('commit') != expected_sha
                or not isinstance(row.get('reference'), str) or not row['reference'].strip()
                or not isinstance(row.get('reviewer'), str) or not row['reviewer'].strip()):
            result.append(f'{name}: missing accepted evidence for this commit')
    risks = document.get('unresolvedCriticalRisks')
    if not isinstance(risks, list) or risks:
        result.append('Critical-risk review is missing or still has unresolved items')
    return result

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('evidence', type=Path)
    parser.add_argument('--commit', required=True)
    args = parser.parse_args()
    try:
        document = json.loads(args.evidence.read_text(encoding='utf-8'))
        if not isinstance(document, dict):
            raise ValueError('Evidence must be an object')
        pending = blockers(document, args.commit)
    except (OSError, ValueError) as exc:
        pending = [f'Cannot read evidence: {exc}']
    print(json.dumps({'candidateAllowed': not pending, 'blockers': pending}, indent=2))
    return 1 if pending else 0

if __name__ == '__main__':
    raise SystemExit(main())
