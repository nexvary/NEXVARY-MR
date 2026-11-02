"""Independent stdlib SQLite WAL oracle; no MR parser is used.

WAL salts vary on regeneration. Committed bytes and SQL expectations are obtained
from SQLite's own serialize() while its original writer remains open.
"""
import base64
import hashlib
import json
from pathlib import Path
import sqlite3
import struct
import tempfile


def sha(b):
    return hashlib.sha256(b).hexdigest()


def checksummed(header, frames, big):
    h = bytearray(header)
    h[:4] = struct.pack('>I', 0x377f0683 if big else 0x377f0682)
    endian = '>' if big else '<'
    state = [0, 0]

    def update(b):
        values = struct.unpack(endian + str(len(b)//4) + 'I', b)
        for i in range(0, len(values), 2):
            state[0] = (state[0] + values[i] + state[1]) & 0xffffffff
            state[1] = (state[1] + values[i+1] + state[0]) & 0xffffffff
        return struct.pack('>II', *state)

    h[24:32] = update(h[:24])
    result = bytes(h)
    for page, commit, data in frames:
        frame = struct.pack('>II', page, commit) + bytes(h[16:24])
        update(frame[:8])
        result += frame + update(data) + data
    return result


def main():
    with tempfile.TemporaryDirectory() as temp:
        path = Path(temp) / 'History'
        db = sqlite3.connect(path)
        db.execute('PRAGMA page_size=1024')
        db.execute('PRAGMA journal_mode=WAL')
        db.execute('PRAGMA wal_autocheckpoint=0')
        db.executescript('CREATE TABLE urls(id INTEGER PRIMARY KEY,url TEXT,title TEXT,last_visit_time INTEGER); CREATE TABLE visits(id INTEGER PRIMARY KEY,url INTEGER,visit_time INTEGER,from_visit INTEGER,transition INTEGER);')
        db.commit()
        db.execute('PRAGMA wal_checkpoint(TRUNCATE)')
        db.execute("INSERT INTO urls VALUES(1,'https://example.invalid/sqlite-family','Synthetic reference',13300000000000000)")
        db.commit()
        db.execute('INSERT INTO visits VALUES(7,1,13300000000000000,0,1)')
        db.commit()
        source = path.read_bytes()
        wal = path.with_name('History-wal').read_bytes()
        shm = path.with_name('History-shm').read_bytes()
        image = db.serialize()
        page_size = 1024
        frames = []
        for off in range(32, len(wal), page_size+24):
            page, commit = struct.unpack('>II', wal[off:off+8])
            frames.append((page, commit, wal[off+24:off+24+page_size]))
        assert checksummed(wal[:32], frames, struct.unpack('>I',wal[:4])[0] == 0x377f0683) == wal
        tail = frames + [(frames[-1][0], 0, frames[-1][2])]
        output = {'schema':'nexvary-sqlite-family-oracle/v1','sqliteVersion':sqlite3.sqlite_version,
                  'pageSize':page_size,'mainBase64':base64.b64encode(source).decode(),
                  'walBase64':base64.b64encode(wal).decode(),'shmBase64':base64.b64encode(shm).decode(),
                  'bigEndianWalBase64':base64.b64encode(checksummed(wal[:32],frames,True)).decode(),
                  'uncommittedWalBase64':base64.b64encode(checksummed(wal[:32],tail,False)).decode(),
                  'missingGrowthWalBase64':base64.b64encode(checksummed(wal[:32],[(1,5,source[:page_size])],False)).decode(),
                  'shrinkGrowWalBase64':base64.b64encode(checksummed(wal[:32],[(1,2,source[:page_size]),(2,3,source[page_size:2*page_size])],False)).decode(),
                  'snapshotBase64':base64.b64encode(image).decode(),'snapshotSha256':sha(image),
                  'mainSha256':sha(source),'walSha256':sha(wal),'shmSha256':sha(shm),
                  'frames':len(frames),'commits':sum(c!=0 for _,c,_ in frames),
                  'expectedUrls':[list(r) for r in db.execute('SELECT id,url,title,last_visit_time FROM urls ORDER BY id')],
                  'expectedVisits':[list(r) for r in db.execute('SELECT * FROM visits ORDER BY id')]}
        db.close()
    Path(__file__).with_name('reference.json').write_text(json.dumps(output,indent=2)+'\n')
    print({k:output[k] for k in ['sqliteVersion','snapshotSha256','frames','commits']})

if __name__ == '__main__':
    main()
