import importlib.util
from pathlib import Path
import subprocess
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('public_history', Path(__file__).parents[1] / 'scripts/audit_public_history.py')
guard = importlib.util.module_from_spec(spec)
spec.loader.exec_module(guard)


class PublicHistoryTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.git('init', '-q')
        self.git('config', 'user.email', 'test@example.invalid')
        self.git('config', 'user.name', 'Synthetic test')

    def git(self, *args):
        return subprocess.check_output(['git', '-C', str(self.root), *args], stderr=subprocess.STDOUT)

    def commit(self, path, data):
        p = self.root / path; p.parent.mkdir(parents=True, exist_ok=True); p.write_bytes(data)
        self.git('add', '--', path); self.git('commit', '-qm', 'Synthetic fixture')

    def test_clean_code_and_template(self):
        self.commit('.env.example', b'API_TOKEN=replace-me\n')
        self.commit('source.py', b'print("Hello")\n')
        result = guard.audit(self.root)
        self.assertEqual(result['findings'], [])
        self.assertEqual(result['reachableCommits'], 2)

    def test_deleted_database_remains_a_finding(self):
        self.commit('workshop.db', b'SQLite format 3\0synthetic')
        self.git('rm', 'workshop.db'); self.git('commit', '-qm', 'Remove fixture')
        findings = guard.audit(self.root)['findings']
        self.assertEqual({f['rule'] for f in findings}, {'operational-data-or-keystore-path', 'sqlite-database-content'})
        self.assertTrue(all('synthetic' not in str(f) for f in findings))

    def test_other_branch_and_renamed_evidence(self):
        self.commit('readme.txt', b'safe')
        initial = self.git('rev-parse', 'HEAD').strip().decode()
        self.git('checkout', '-qb', 'other')
        self.commit('looks-like-image.bin', bytes.fromhex('d4c3b2a1') + b'SYNTHETIC')
        self.git('checkout', '--detach', initial)
        self.assertIn('packet-capture-content', {f['rule'] for f in guard.audit(self.root)['findings']})

    def test_private_material_and_large_unreviewed_blob(self):
        self.commit('opaque.txt', b'-----BEGIN ' + b'PRIVATE KEY-----\nSYNTHETIC ONLY')
        self.commit('large.bin', b'A' * 100)
        old = guard.LIMIT
        try:
            guard.LIMIT = 99
            rules = {f['rule'] for f in guard.audit(self.root)['findings']}
        finally:
            guard.LIMIT = old
        self.assertIn('private-key-content', rules)
        self.assertIn('oversize-blob-requires-separate-review', rules)

    def test_reused_blob_is_checked_under_every_filename(self):
        self.commit('safe.txt', b'Identical synthetic bytes')
        self.commit('backup.db', b'Identical synthetic bytes')
        result = guard.audit(self.root)
        self.assertTrue(any(f.get('path') == 'backup.db' for f in result['findings']))

    def test_backups_and_lfs_require_review(self):
        self.assertEqual(guard.content_rule(b'ANDROID BACKUP\n1\n'), 'android-backup-content')
        self.assertEqual(guard.content_rule(b'version https://git-lfs.github.com/spec/v1\noid sha256:example'), 'lfs-content-requires-separate-review')
        self.assertEqual(guard.path_rule('.env.production'), 'environment-data-path')
        self.assertEqual(guard.path_rule('keys/signing.pfx'), 'operational-data-or-keystore-path')
