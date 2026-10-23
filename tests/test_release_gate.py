import copy
import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('release_gate', Path(__file__).parents[1] / 'scripts/release_gate.py')
gate = importlib.util.module_from_spec(spec)
spec.loader.exec_module(gate)

class ReleaseGateTests(unittest.TestCase):
    sha = 'a' * 40

    def evidence(self):
        return {'schemaVersion': 1, 'commit': self.sha, 'buildProfile': 'production-read-only',
                'unresolvedCriticalRisks': [], 'gates': {
                    name: {'passed': True, 'commit': self.sha, 'reference': 'synthetic-unit-fixture',
                           'reviewer': 'synthetic reviewer'} for name in gate.REQUIRED}}

    def test_complete_fixture_only(self):
        self.assertEqual(gate.blockers(self.evidence(), self.sha), [])

    def test_missing_gate_cannot_be_substituted_by_ci(self):
        for name in gate.REQUIRED:
            with self.subTest(name=name):
                data = self.evidence()
                del data['gates'][name]
                self.assertTrue(gate.blockers(data, self.sha))

    def test_wrong_commit_profile_and_unresolved_risk(self):
        for field, value in [('commit', 'b' * 40), ('buildProfile', 'laboratory-experimental'),
                             ('unresolvedCriticalRisks', ['unreviewed'])]:
            data = self.evidence()
            data[field] = value
            self.assertTrue(gate.blockers(data, self.sha))

    def test_string_booleans_missing_reference_and_stale_evidence(self):
        for field, value in [('passed', 'true'), ('passed', 1), ('commit', 'b' * 40),
                             ('reference', ''), ('reviewer', None)]:
            data = copy.deepcopy(self.evidence())
            data['gates']['android_physical_read_only'][field] = value
            self.assertTrue(gate.blockers(data, self.sha))

    def test_malformed_evidence(self):
        self.assertTrue(gate.blockers({}, self.sha))
        self.assertTrue(gate.blockers({'gates': []}, self.sha))
        self.assertTrue(gate.blockers(self.evidence(), 'short'))
