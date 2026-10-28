"""Independent SQLite oracle; writes only generated temporary fixtures."""
import base64
import hashlib
import json
import sqlite3
import tempfile
from pathlib import Path


def main():
    with tempfile.TemporaryDirectory() as temp:
        path = Path(temp) / 'History'
        db = sqlite3.connect(path)
        db.execute('PRAGMA page_size=1024')
        db.execute('PRAGMA journal_mode=WAL')
        db.execute('PRAGMA wal_autocheckpoint=0')
        # Deliberately reordered columns: parser must interpret the declaration.
        db.execute('CREATE TABLE urls(title LONGVARCHAR, id INTEGER PRIMARY KEY, url LONGVARCHAR NOT NULL, visit_count INTEGER DEFAULT 0 NOT NULL, typed_count INTEGER DEFAULT 0 NOT NULL, last_visit_time INTEGER NOT NULL, hidden INTEGER DEFAULT 0 NOT NULL)')
        db.execute('CREATE TABLE visits(transition INTEGER DEFAULT 0 NOT NULL, visit_time INTEGER NOT NULL, id INTEGER PRIMARY KEY, url INTEGER NOT NULL, from_visit INTEGER, visit_duration INTEGER DEFAULT 0 NOT NULL)')
        db.execute('PRAGMA wal_checkpoint(TRUNCATE)')
        db.execute('INSERT INTO urls VALUES(?,?,?,?,?,?,?)', ('عنوان عربي', 7, 'https://example.invalid/a?x=1', 2, 1, 13344473600123456, 0))
        for visit, time in [(11, 13344473600123456), (15, 13344473600999999)]:
            db.execute('INSERT INTO visits VALUES(?,?,?,?,?,?)', (805306368, time, visit, 7, 0, 0))
        db.commit()
        db.execute('UPDATE urls SET title=? WHERE id=7', ('عنوان معدّل من WAL',))
        db.commit()
        image = db.serialize()
        encode = lambda b: base64.b64encode(b).decode('ascii')
        out = {'sqliteVersion': sqlite3.sqlite_version, 'mainBase64': encode(path.read_bytes()), 'walBase64': encode(Path(str(path)+'-wal').read_bytes()), 'snapshotBase64': encode(image), 'snapshotSha256': hashlib.sha256(image).hexdigest(), 'expected': [list(r) for r in db.execute('SELECT v.id,u.id,u.url,u.title,v.visit_time,u.visit_count,u.typed_count,u.last_visit_time,v.transition,v.from_visit FROM visits v JOIN urls u ON u.id=v.url ORDER BY v.id')]}
        db.close()
    Path(__file__).with_name('reference.json').write_text(json.dumps(out, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')


if __name__ == '__main__':
    main()
