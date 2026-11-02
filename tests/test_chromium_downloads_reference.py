"""SQL runtime oracle; independent of all MR parsers and table traversal."""
import base64
from contextlib import closing
import hashlib
import json
from pathlib import Path
import sqlite3
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class ChromiumDownloadsReferenceTests(unittest.TestCase):
    def test_runtime_sql_records_chains_and_unchanged_image(self):
        f = json.loads((ROOT / 'samples/chromium-downloads/reference.json').read_text(encoding='utf-8'))
        image = base64.b64decode(f['snapshotBase64'], validate=True)
        self.assertEqual(hashlib.sha256(image).hexdigest(), f['snapshotSha256'])
        self.assertNotEqual(image, base64.b64decode(f['mainBase64'], validate=True))
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / 'image.sqlite'
            path.write_bytes(image)
            with closing(sqlite3.connect(path.as_uri() + '?immutable=1', uri=True)) as db:
                self.assertEqual(db.execute('PRAGMA quick_check').fetchone(), ('ok',))
                names = [r[1] for r in db.execute('PRAGMA table_info(downloads)')]
                expected = [{('declaredHashHex' if k == 'hash' else k):
                             v.hex() if isinstance(v, bytes) else str(v) if isinstance(v, int) else v
                             for k, v in zip(names, row)}
                            for row in db.execute('SELECT * FROM downloads ORDER BY id')]
                self.assertEqual(expected, f['expected'])
                chains = [[str(v) for v in row] for row in db.execute(
                    'SELECT rowid,id,chain_index,url FROM downloads_url_chains ORDER BY id,chain_index')]
                self.assertEqual(chains, f['expectedChains'])
                self.assertEqual(expected[0]['target_path'], 'C:\\synthetic\\محدّث-من-WAL.bin')
                self.assertEqual(expected[1]['id'], '9007199254740993')
                self.assertEqual(expected[0]['received_bytes'], '9007199254740993')
                self.assertEqual(expected[0]['total_bytes'], '-1')
            self.assertEqual(path.read_bytes(), image)


if __name__ == '__main__':
    unittest.main()
