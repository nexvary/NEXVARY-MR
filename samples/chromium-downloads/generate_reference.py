"""Independent SQLite runtime oracle. Generates synthetic, offline WAL bytes only."""
import base64
from contextlib import closing
import hashlib
import json
from pathlib import Path
import sqlite3
import tempfile

DOWNLOAD_SCHEMA = '''CREATE TABLE downloads (
 id INTEGER PRIMARY KEY, guid VARCHAR NOT NULL,
 current_path LONGVARCHAR NOT NULL, target_path LONGVARCHAR NOT NULL,
 start_time INTEGER NOT NULL, received_bytes INTEGER NOT NULL,
 total_bytes INTEGER NOT NULL, state INTEGER NOT NULL,
 danger_type INTEGER NOT NULL, interrupt_reason INTEGER NOT NULL,
 hash BLOB NOT NULL, end_time INTEGER NOT NULL, opened INTEGER NOT NULL,
 last_access_time INTEGER NOT NULL, transient INTEGER NOT NULL,
 referrer VARCHAR NOT NULL, site_url VARCHAR NOT NULL,
 embedder_download_data VARCHAR NOT NULL, tab_url VARCHAR NOT NULL,
 tab_referrer_url VARCHAR NOT NULL, http_method VARCHAR NOT NULL,
 by_ext_id VARCHAR NOT NULL, by_ext_name VARCHAR NOT NULL,
 by_web_app_id VARCHAR NOT NULL, etag VARCHAR NOT NULL,
 last_modified VARCHAR NOT NULL, mime_type VARCHAR(255) NOT NULL,
 original_mime_type VARCHAR(255) NOT NULL)'''
CHAIN_SCHEMA = '''CREATE TABLE downloads_url_chains (
 id INTEGER NOT NULL, chain_index INTEGER NOT NULL, url LONGVARCHAR NOT NULL,
 PRIMARY KEY (id, chain_index) )'''


def main():
    with tempfile.TemporaryDirectory() as temp:
        path = Path(temp) / 'History'
        with closing(sqlite3.connect(path)) as db:
            db.execute('PRAGMA page_size=1024')
            db.execute('PRAGMA journal_mode=WAL')
            db.execute('PRAGMA wal_autocheckpoint=0')
            db.execute(DOWNLOAD_SCHEMA)
            db.execute(CHAIN_SCHEMA)
            db.commit()
            db.execute('PRAGMA wal_checkpoint(TRUNCATE)')
            names = [row[1] for row in db.execute('PRAGMA table_info(downloads)')]
            integer_fields = {'start_time', 'received_bytes', 'total_bytes', 'state',
                              'danger_type', 'interrupt_reason', 'end_time', 'opened',
                              'last_access_time', 'transient'}
            for ident in (42, 9007199254740993):
                row = {key: 0 if key in integer_fields else '' for key in names}
                row.update(id=ident, guid='synthetic-' + str(ident),
                           current_path=r'C:\synthetic\unfinished.bin',
                           target_path='C:\\synthetic\\ملف.bin',
                           start_time=13344473600123456, end_time=0,
                           received_bytes=9007199254740993, total_bytes=-1,
                           state=4, hash=bytes.fromhex('00ff'), mime_type='application/octet-stream',
                           tab_url='https://context.invalid/tab')
                db.execute('INSERT INTO downloads VALUES(' + ','.join('?' for _ in names) + ')',
                           [row[key] for key in names])
            # Physical rowid order deliberately disagrees with redirect-index order.
            db.execute('INSERT INTO downloads_url_chains VALUES(42,1,?)', ('https://final.invalid/file',))
            db.execute('INSERT INTO downloads_url_chains VALUES(42,0,?)', ('https://initial.invalid/file',))
            db.commit()
            db.execute('UPDATE downloads SET target_path=? WHERE id=42', ('C:\\synthetic\\محدّث-من-WAL.bin',))
            db.commit()
            image = db.serialize()
            expected = []
            for row in db.execute('SELECT * FROM downloads ORDER BY id'):
                item = dict(zip(names, row))
                expected.append({('declaredHashHex' if k == 'hash' else k):
                                 v.hex() if isinstance(v, bytes) else str(v) if isinstance(v, int) else v
                                 for k, v in item.items()})
            encode = lambda value: base64.b64encode(value).decode('ascii')
            out = {'sqliteVersion': sqlite3.sqlite_version,
                   'mainBase64': encode(path.read_bytes()),
                   'walBase64': encode(Path(str(path) + '-wal').read_bytes()),
                   'shmBase64': encode(Path(str(path) + '-shm').read_bytes()),
                   'snapshotBase64': encode(image),
                   'snapshotSha256': hashlib.sha256(image).hexdigest(),
                   'expected': expected,
                   'expectedChains': [[str(v) for v in row] for row in db.execute(
                       'SELECT rowid,id,chain_index,url FROM downloads_url_chains ORDER BY id,chain_index')]}
    Path(__file__).with_name('reference.json').write_text(
        json.dumps(out, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print(out['snapshotSha256'])


if __name__ == '__main__':
    main()
