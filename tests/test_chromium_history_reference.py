"""Independent runtime checks for the committed offline History fixture."""
import base64
from contextlib import closing
from datetime import datetime, timedelta, timezone
import hashlib
import json
from pathlib import Path
import sqlite3
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class ChromiumHistoryReferenceTests(unittest.TestCase):
    def test_independent_sql_join_and_exact_times(self):
        fixture = json.loads((ROOT / 'samples/chromium-history/reference.json').read_text())
        image = base64.b64decode(fixture['snapshotBase64'], validate=True)
        self.assertEqual(hashlib.sha256(image).hexdigest(), fixture['snapshotSha256'])
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / 'image.sqlite'
            path.write_bytes(image)
            with closing(sqlite3.connect(path.as_uri() + '?immutable=1', uri=True)) as db:
                self.assertEqual(db.execute('PRAGMA quick_check').fetchone(), ('ok',))
                rows = [list(row) for row in db.execute('SELECT v.id,u.id,u.url,u.title,v.visit_time,u.visit_count,u.typed_count,u.last_visit_time,v.transition,v.from_visit FROM visits v JOIN urls u ON u.id=v.url ORDER BY v.id')]
                self.assertEqual(rows, fixture['expected'])
                self.assertEqual(rows[0][3], 'عنوان معدّل من WAL')
                timestamp = datetime(1601, 1, 1, tzinfo=timezone.utc) + timedelta(microseconds=rows[0][4])
                self.assertEqual(timestamp.isoformat(timespec='microseconds'), '2023-11-14T22:13:20.123456+00:00')
            self.assertEqual(path.read_bytes(), image)

    def test_fixture_declares_wal_distinct_from_main(self):
        script = ROOT / 'samples/chromium-history/generate_reference.py'
        fixture = json.loads(script.with_name('reference.json').read_text())
        # Runtime version changes need review, not an unqualified byte equality claim.
        self.assertTrue(fixture['sqliteVersion'])
        self.assertEqual(len(fixture['expected']), 2)
        self.assertNotEqual(base64.b64decode(fixture['mainBase64']), base64.b64decode(fixture['snapshotBase64']))


if __name__ == '__main__':
    unittest.main()
