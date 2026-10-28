"""Independent SQLite runtime/stdlib fixture checks, without invoking MR code."""
import base64
import hashlib
import json
from pathlib import Path
import sqlite3
import tempfile
import unittest

class SqliteFamilyOracleTests(unittest.TestCase):
    def test_source_digests_and_runtime_page_image(self):
        r=json.loads((Path(__file__).resolve().parents[1]/'samples/sqlite-family/reference.json').read_text())
        for name in ['main','wal','shm','snapshot']:
            self.assertEqual(hashlib.sha256(base64.b64decode(r[name+'Base64'])).hexdigest(),r[name+'Sha256'])
        with tempfile.TemporaryDirectory() as temp:
            path=Path(temp)/'page-image.sqlite';path.write_bytes(base64.b64decode(r['snapshotBase64']));before=path.read_bytes()
            with sqlite3.connect(path.as_uri()+'?mode=ro&immutable=1',uri=True) as db:
                self.assertEqual([list(row) for row in db.execute('SELECT id,url,title,last_visit_time FROM urls ORDER BY id')],r['expectedUrls'])
                self.assertEqual([list(row) for row in db.execute('SELECT * FROM visits ORDER BY id')],r['expectedVisits'])
                self.assertEqual(db.execute('PRAGMA quick_check').fetchone()[0],'ok')
            self.assertEqual(path.read_bytes(),before)
