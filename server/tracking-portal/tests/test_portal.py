import copy
import hashlib
import io
import json
import sqlite3
import sys
import tempfile
import unittest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from portal_app import Portal, utc
from portal_admin import backup, restore
from render import render, valid_domain


class PortalTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.now = 1800000000
        self.key = 'a' * 64
        self.token = 'b' * 64
        self.config = {'database': str(self.root / 'portal.sqlite3'), 'domain': 'tracking.example.test',
                       'sourceId': 'c' * 32, 'apiKeySha256': hashlib.sha256(self.key.encode()).hexdigest()}
        self.app = Portal(self.config, clock=lambda: self.now)
        self.record = {'id': 1, 'tokenHash': hashlib.sha256(self.token.encode()).hexdigest(),
                       'expiresAtUtc': utc(self.now + 86400), 'updatedAtUtc': utc(self.now),
                       'brand': 'SYNTHETIC', 'model': 'Training-01', 'status': 'Repairing',
                       'message': '<script>synthetic</script>', 'history': [{'status': 'Received', 'atUtc': utc(self.now - 60)}]}

    def tearDown(self):
        self.temp.cleanup()

    def call(self, path, method='GET', value=None, key=None, **extra):
        body = json.dumps(value).encode() if value is not None else b''
        env = {'PATH_INFO': path, 'REQUEST_METHOD': method, 'HTTP_HOST': self.config['domain'],
               'wsgi.url_scheme': 'https', 'REMOTE_ADDR': '127.0.0.1', 'wsgi.input': io.BytesIO(body),
               'CONTENT_LENGTH': str(len(body)), 'CONTENT_TYPE': 'application/json'}
        if key is not None:
            env['HTTP_AUTHORIZATION'] = 'Bearer ' + key
        env.update(extra)
        captured = {}
        def start(status, headers):
            captured.update(status=int(status[:3]), headers=dict(headers))
        data = b''.join(self.app(env, start))
        return captured['status'], data, captured['headers']

    def snapshot(self, records=None, revision=0):
        return {'schemaVersion': 1, 'sourceId': self.config['sourceId'], 'expectedRevision': revision,
                'records': [self.record] if records is None else records}

    def test_status_auth_privacy_revision_rotation(self):
        self.assertEqual(self.call('/api/v1/state')[0], 401)
        self.assertEqual(self.call('/api/v1/snapshot', 'POST', self.snapshot(), key='wrong')[0], 401)
        self.assertEqual(self.call('/api/v1/snapshot', 'POST', self.snapshot(), key=self.key)[0], 200)
        code, body, headers = self.call('/t/' + self.token)
        self.assertEqual(code, 200)
        self.assertIn(b'&lt;script&gt;', body)
        self.assertNotIn(b'<script>', body)
        self.assertEqual(headers['Cache-Control'], 'no-store')
        self.assertIn('frame-ancestors', headers['Content-Security-Policy'])
        self.assertNotIn(self.token.encode(), Path(self.config['database']).read_bytes())
        self.assertEqual(self.call('/api/v1/snapshot', 'POST', self.snapshot(), key=self.key)[0], 409)
        self.assertEqual(self.call('/api/v1/snapshot', 'POST', self.snapshot([], 1), key=self.key)[0], 200)
        self.assertEqual(self.call('/t/' + self.token)[0], 404)

    def test_rejects_private_extra_fields_and_transaction_is_atomic(self):
        self.assertEqual(self.call('/api/v1/snapshot', 'POST', self.snapshot(), key=self.key)[0], 200)
        bad = copy.deepcopy(self.record);bad['customerPhone'] = 'MUST-NOT-UPLOAD'
        self.assertEqual(self.call('/api/v1/snapshot', 'POST', self.snapshot([bad], 1), key=self.key)[0], 400)
        self.assertEqual(self.call('/t/' + self.token)[0], 200)
        self.assertEqual(json.loads(self.call('/api/v1/state', key=self.key)[1])['revision'], 1)
        self.assertEqual(self.call('/api/v1/snapshot', 'POST', self.snapshot([self.record, self.record], 1), key=self.key)[0], 400)

    def test_expiry_lease_host_scheme_limits_and_source(self):
        self.assertEqual(self.call('/api/v1/snapshot', 'POST', self.snapshot(), key=self.key)[0], 200)
        self.now += 6 * 3600
        self.assertEqual(self.call('/t/' + self.token)[0], 404)
        self.assertEqual(self.call('/t/' + self.token, HTTP_HOST='evil.example')[0], 400)
        self.assertEqual(self.call('/t/' + self.token, **{'wsgi.url_scheme': 'http'})[0], 400)
        self.assertEqual(self.call('/api/v1/snapshot', 'POST', key=self.key, CONTENT_LENGTH=str(5000000))[0], 413)
        wrong = self.snapshot([], 1);wrong['sourceId'] = 'd' * 32
        self.assertEqual(self.call('/api/v1/snapshot', 'POST', wrong, key=self.key)[0], 409)
        expired = copy.deepcopy(self.record);expired['expiresAtUtc'] = utc(self.now - 1)
        self.assertEqual(self.call('/api/v1/snapshot', 'POST', self.snapshot([expired], 1), key=self.key)[0], 400)

    def test_rate_limit_and_malformed_json(self):
        for _ in range(120):
            self.assertEqual(self.call('/t/' + self.token)[0], 404)
        self.assertEqual(self.call('/t/' + self.token)[0], 429)
        bad = b'{"schemaVersion":1,"schemaVersion":1}'
        self.assertEqual(self.call('/api/v1/snapshot', 'POST', key=self.key, CONTENT_LENGTH=str(len(bad)), **{'wsgi.input': io.BytesIO(bad)})[0], 400)

    def test_backup_restore_revokes_and_recovers_missing_database(self):
        self.assertEqual(self.call('/api/v1/snapshot', 'POST', self.snapshot(), key=self.key)[0], 200)
        saved = backup(self.config, self.root / 'backups')
        with self.assertRaises(ValueError):
            restore(self.config, saved, False)
        Path(self.config['database']).unlink()
        restore(self.config, saved, True)
        self.assertEqual(self.call('/t/' + self.token)[0], 404)
        self.assertEqual(json.loads(self.call('/api/v1/state', key=self.key)[1])['revision'], 2)

    def test_render_no_secrets_in_units_and_upgrade_retains_key(self):
        folder = self.root / 'rendered';render('tracking.example.com', 'admin@example.com', folder)
        creds = json.loads((folder / 'client-credentials.json').read_text())
        for f in folder.iterdir():
            if f.name != 'client-credentials.json':
                self.assertNotIn(creds['apiKey'], f.read_text())
        config = json.loads((folder / 'settings.json').read_text())
        self.assertEqual(config['apiKeySha256'], hashlib.sha256(creds['apiKey'].encode()).hexdigest())
        upgraded = self.root / 'upgrade';render('tracking.example.com', 'admin@example.com', upgraded, folder / 'settings.json')
        self.assertEqual(json.loads((upgraded / 'settings.json').read_text()), config)
        self.assertFalse((upgraded / 'client-credentials.json').exists())
        for invalid in ['x;touch /tmp/pwn', '*.example.com', '127.0.0.1', 'a..com', '-bad.example']:
            self.assertFalse(valid_domain(invalid))


if __name__ == '__main__':
    unittest.main()
