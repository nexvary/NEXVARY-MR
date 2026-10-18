"""Install the actual single-file bundle on a disposable GitHub Actions systemd VM.

Never run this fixture on an operator's server; it deliberately installs and changes test state.
No credential or raw customer token is printed or uploaded.
"""
import hashlib
import json
import os
from pathlib import Path
import secrets
import socket
import ssl
import subprocess
import sys
import time
import urllib.error
import urllib.request

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from portal_app import utc


def command(*args, **kwargs):
    return subprocess.run(args, check=True, **kwargs)


def run(bundle):
    assert os.geteuid() == 0 and os.environ.get('GITHUB_ACTIONS') == 'true', 'Disposable CI VM only'
    assert not Path('/opt/nexvary-portal').exists(), 'Fresh VM required'
    args = ['bash', str(bundle), '--local-test', '--domain', 'portal.localhost', '--email', 'test@example.com']
    command(*args, '--dry-run')
    assert not Path('/opt/nexvary-portal').exists()
    with socket.socket() as occupied:
        occupied.bind(('127.0.0.1', 9443));occupied.listen()
        refused = subprocess.run(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        assert refused.returncode != 0 and b'occupied' in refused.stdout, 'Existing listener must not be replaced'
        assert not Path('/opt/nexvary-portal').exists()
    with Path('/etc/hosts').open('a') as hosts:
        hosts.write('\n127.0.0.1 portal.localhost\n')
    command(*args)
    credentials_path = Path('/root/nexvary-portal-client-credentials.json')
    credentials = json.loads(credentials_path.read_text())
    assert credentials_path.stat().st_mode & 0o777 == 0o600
    assert credentials['serverUrl'] == 'https://portal.localhost:9443'
    ca = '/var/lib/nexvary-portal-tls/pki/authorities/local/root.crt'
    opener = urllib.request.build_opener(urllib.request.ProxyHandler({}), urllib.request.HTTPSHandler(context=ssl.create_default_context(cafile=ca)))

    def request(path, data=None, key=None):
        headers = {'Authorization': 'Bearer ' + (key or credentials['apiKey']), 'Content-Type': 'application/json'}
        req = urllib.request.Request(credentials['serverUrl'] + path, headers=headers, data=json.dumps(data).encode() if data is not None else None)
        try:
            with opener.open(req, timeout=10) as reply:
                return reply.status, reply.read()
        except urllib.error.HTTPError as error:
            return error.code, error.read()

    def active():
        for name in ('nexvary-portal.service', 'nexvary-portal-tls.service', 'nexvary-portal-backup.timer'):
            command('systemctl', 'is-active', '--quiet', name)
        assert request('/healthz')[0] == 200

    active()
    assert request('/api/v1/state', key='wrong')[0] == 401
    token = secrets.token_hex(32)
    now = int(time.time())
    record = {'id': 1, 'tokenHash': hashlib.sha256(token.encode()).hexdigest(), 'expiresAtUtc': utc(now + 86400), 'updatedAtUtc': utc(now), 'brand': 'SYNTHETIC', 'model': 'VM-Fixture', 'status': 'Received', 'message': 'Synthetic CI test', 'history': []}

    def publish(records):
        status, body = request('/api/v1/state')
        assert status == 200
        snapshot = {'schemaVersion': 1, 'sourceId': credentials['sourceId'], 'expectedRevision': json.loads(body)['revision'], 'records': records}
        assert request('/api/v1/snapshot', snapshot)[0] == 200

    publish([record])
    assert request('/t/' + token)[0] == 200
    command('systemctl', 'start', 'nexvary-portal-backup.service')
    saved = sorted(Path('/var/backups/nexvary-portal').glob('portal-*.sqlite3'))
    assert saved and saved[-1].stat().st_mode & 0o777 == 0o600
    original = credentials_path.read_bytes()
    previous_release = Path('/opt/nexvary-portal/current').resolve()
    command(*args, '--upgrade')
    assert credentials_path.read_bytes() == original
    assert previous_release != Path('/opt/nexvary-portal/current').resolve()
    active()
    assert request('/t/' + token)[0] == 200
    admin = ['/opt/nexvary-portal/current/venv/bin/python', '/opt/nexvary-portal/current/portal_admin.py', '--config', '/etc/nexvary-portal/settings.json']
    refused = subprocess.run(admin + ['restore', str(saved[-1]), '--confirmed-service-stopped'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    assert refused.returncode != 0, 'Restoration must reject active writers/timer'
    assert request('/t/' + token)[0] == 200
    command('systemctl', 'stop', 'nexvary-portal-backup.timer')
    command('systemctl', 'stop', 'nexvary-portal-backup.service', 'nexvary-portal.service')
    command(*admin, 'restore', str(saved[-1]), '--confirmed-service-stopped')
    command('systemctl', 'start', 'nexvary-portal.service')
    command('systemctl', 'start', 'nexvary-portal-backup.timer')
    for _ in range(40):
        try:
            if request('/api/v1/state')[0] == 200:
                break
        except OSError:
            pass
        time.sleep(.25)
    assert request('/t/' + token)[0] == 404, 'Restores must not resurrect revoked links'
    publish([record])
    assert request('/t/' + token)[0] == 200
    publish([])
    assert request('/t/' + token)[0] == 404
    old_key = credentials['apiKey']
    command(*admin, 'rotate-key', '--directory', '/root/nexvary-ci-rotated')
    command('systemctl', 'restart', 'nexvary-portal.service')
    credentials = json.loads(Path('/root/nexvary-ci-rotated/client-credentials.json').read_text())
    for _ in range(40):
        try:
            if request('/api/v1/state')[0] == 200:
                break
        except OSError:
            pass
        time.sleep(.25)
    assert credentials['serverUrl'] == 'https://portal.localhost:9443'
    assert request('/api/v1/state', key=old_key)[0] == 401
    assert request('/api/v1/state')[0] == 200
    command('systemctl', 'start', 'nexvary-portal-backup.service')
    logs = command('journalctl', '-u', 'nexvary-portal.service', '-u', 'nexvary-portal-tls.service', '--no-pager', stdout=subprocess.PIPE).stdout
    for secret in (old_key, credentials['apiKey'], token):
        assert secret.encode() not in logs, 'Secret leaked into service journal'
    active()
    print('PASS: actual systemd install, trusted TLS, private credentials, status publish, backup, upgrade, restore revocation, key rotation and journal privacy')


if __name__ == '__main__':
    run(Path(sys.argv[1]).resolve())
