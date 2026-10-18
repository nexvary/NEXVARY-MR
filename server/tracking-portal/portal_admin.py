"""Local administrator only. Never prints API keys or customer link tokens."""
import argparse
import hashlib
import json
import os
import secrets
import sqlite3
import subprocess
import tempfile
import time
from datetime import datetime, timezone
from pathlib import Path
from portal_app import connect, initialize


def backup(config, destination):
    owner = Path(config['database']).stat()
    if os.geteuid() != owner.st_uid:
        raise ValueError('Run backups through the service timer or as the database service account')
    folder = Path(destination)
    folder.mkdir(mode=0o700, parents=True, exist_ok=True)
    os.chmod(folder, 0o700)
    name = 'portal-' + datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S') + '-' + secrets.token_hex(4) + '.sqlite3'
    target = folder / name
    with connect(config['database']) as db:
        db.execute('DELETE FROM entries WHERE expires_at<=?', (int(time.time()),))
    with connect(config['database']) as source:
        with sqlite3.connect(target) as out:
            source.backup(out)
    os.chmod(target, 0o600)
    for old in sorted(folder.glob('portal-*.sqlite3'), reverse=True)[14:]:
        if old.is_file() and not old.is_symlink():
            old.unlink()
    return target


def restore(config, source, confirmed):
    if not confirmed:
        raise ValueError('Stop nexvary-portal.service and pass --confirmed-service-stopped')
    original = Path(source)
    if not original.is_file() or original.is_symlink() or original.stat().st_size > 256 * 1024 * 1024:
        raise ValueError('Invalid bounded backup file')
    # Stage and validate data before touching the current database. Reject executable schema objects.
    fd, staged = tempfile.mkstemp(prefix='.portal-restore-', dir=Path(config['database']).parent)
    os.close(fd)
    try:
        Path(staged).write_bytes(original.read_bytes())
        with sqlite3.connect(staged) as db:
            db.execute('PRAGMA trusted_schema=OFF')
            if db.execute('PRAGMA quick_check').fetchone()[0] != 'ok' or db.execute('PRAGMA user_version').fetchone()[0] != 1:
                raise ValueError('Backup integrity/version mismatch')
            schema = db.execute("SELECT name,type FROM sqlite_master WHERE name NOT LIKE 'sqlite_%'").fetchall()
            if set(schema) != {('state', 'table'), ('entries', 'table')}:
                raise ValueError('Unexpected backup schema objects')
            state = db.execute('SELECT source_id,revision FROM state WHERE id=1').fetchone()
            if not state or state[0] != config['sourceId'] or type(state[1]) is not int or state[1] < 0:
                raise ValueError('Backup publisher identity/revision mismatch')
            # Rebuild a known schema instead of running imported SQL, and revoke restored public links.
        current = Path(config['database'])
        owner = current.stat() if current.exists() else None
        preserved = current.parent / ('before-restore-' + secrets.token_hex(8))
        preserved.mkdir(mode=0o700)
        for suffix in ('', '-wal', '-shm'):
            f = Path(str(current) + suffix)
            if f.exists():
                import shutil
                shutil.copy2(f, preserved / f.name)
                os.chmod(preserved / f.name, 0o600)
        rebuilt = str(current) + '.restored-' + secrets.token_hex(8)
        initialize(rebuilt, config['sourceId'])
        with connect(rebuilt) as db:
            db.execute('UPDATE state SET revision=?,synced_at=0 WHERE id=1', (state[1] + 1,))
        if owner:
            os.chown(rebuilt, owner.st_uid, owner.st_gid)
        else:
            import pwd
            try:
                account = pwd.getpwnam('nexvary-portal')
                os.chown(rebuilt, account.pw_uid, account.pw_gid)
            except KeyError:
                pass
        for suffix in ('-wal', '-shm'):
            Path(str(current) + suffix).unlink(missing_ok=True)
        os.replace(rebuilt, current)
        # Deliberately empty: only a fresh desktop sync may republish live links.
    finally:
        Path(staged).unlink(missing_ok=True)


def rotate(config_path, destination):
    config = json.loads(Path(config_path).read_text())
    key = secrets.token_hex(32)
    config['apiKeySha256'] = hashlib.sha256(key.encode()).hexdigest()
    folder = Path(destination)
    folder.mkdir(mode=0o700, parents=True, exist_ok=True)
    credentials = folder / 'client-credentials.json'
    if credentials.exists():
        raise ValueError('Choose a new empty credentials destination')
    port = config.get('httpsPort', 443)
    url = 'https://' + config['domain'] + (':' + str(port) if port != 443 else '')
    credentials.write_text(json.dumps({'schemaVersion': 1, 'serverUrl': url, 'sourceId': config['sourceId'], 'apiKey': key}, indent=2))
    os.chmod(credentials, 0o600)
    temp = Path(str(config_path) + '.new')
    with temp.open('x') as f:
        json.dump(config, f, indent=2)
    os.chmod(temp, 0o640)
    os.chown(temp, Path(config_path).stat().st_uid, Path(config_path).stat().st_gid)
    os.replace(temp, config_path)
    return credentials


def require_offline_services():
    # Stopping the API alone is insufficient: the daily backup also writes expiry cleanup.
    # Stop its timer first, then both writers, before replacing SQLite/WAL files.
    for unit in ('nexvary-portal-backup.timer', 'nexvary-portal-backup.service', 'nexvary-portal.service'):
        state = subprocess.run(['systemctl', 'show', '--property=ActiveState', '--value', unit],
                               check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        if state.stdout.strip() not in ('inactive', 'failed'):
            raise ValueError('Stop the backup timer, backup service and API before restoration')


if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('--config', required=True)
    sub = p.add_subparsers(dest='action', required=True)
    b = sub.add_parser('backup');b.add_argument('--directory', required=True)
    r = sub.add_parser('restore');r.add_argument('source');r.add_argument('--confirmed-service-stopped', action='store_true')
    k = sub.add_parser('rotate-key');k.add_argument('--directory', required=True)
    args = p.parse_args()
    cfg = json.loads(Path(args.config).read_text())
    if args.action == 'backup':
        print('Backup saved:', backup(cfg, args.directory))
    elif args.action == 'restore':
        require_offline_services()
        restore(cfg, args.source, args.confirmed_service_stopped)
        print('Public links invalidated. Start service and sync the current desktop.')
    else:
        print('Credentials saved:', rotate(args.config, args.directory))
        print('Restart nexvary-portal.service, then import the new file into Windows.')
