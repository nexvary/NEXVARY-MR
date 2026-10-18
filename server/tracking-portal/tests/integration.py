"""Actual Gunicorn -> Caddy verified TLS -> desktop client / public status workflow."""
import argparse
import hashlib
import http.client
import json
import os
from pathlib import Path
import secrets
import signal
import socket
import ssl
import subprocess
import sys
import tempfile
import time
import urllib.error
import urllib.request
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from render import render
from portal_app import utc


def free_port():
    with socket.socket() as sock:
        sock.bind(('127.0.0.1', 0))
        return sock.getsockname()[1]


def run(caddy, native=None):
    root = Path(__file__).resolve().parents[1]
    with tempfile.TemporaryDirectory(prefix='nexvary-tls-test-') as td:
        folder = Path(td)
        render('portal.localhost', 'test@example.com', folder / 'cfg', local_test=True)
        ports = set()
        while len(ports) < 3:
            ports.add(free_port())
        http_port, tls_port, api_port = ports
        config = json.loads((folder / 'cfg/settings.json').read_text())
        config.update(domain='localhost', httpsPort=tls_port, database=str(folder / 'data/status.sqlite3'))
        (folder / 'cfg/settings.json').write_text(json.dumps(config))
        credentials = json.loads((folder / 'cfg/client-credentials.json').read_text())
        credentials['serverUrl'] = f'https://localhost:{tls_port}'
        (folder / 'cfg/client-credentials.json').write_text(json.dumps(credentials))
        caddyfile = (folder / 'cfg/Caddyfile').read_text().replace('portal.localhost', 'localhost').replace('9443', str(tls_port)).replace('9080', str(http_port)).replace('9085', str(api_port)).replace('/var/lib/nexvary-portal-tls', str(folder / 'tls'))
        (folder / 'Caddyfile').write_text(caddyfile)
        subprocess.run([str(caddy), 'validate', '--config', str(folder / 'Caddyfile'), '--adapter', 'caddyfile'], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
        env = os.environ | {'NEXVARY_PORTAL_CONFIG': str(folder / 'cfg/settings.json'), 'PYTHONDONTWRITEBYTECODE': '1'}
        with (folder / 'gunicorn.log').open('wb') as api_log, (folder / 'caddy.log').open('wb') as tls_log:
            api = subprocess.Popen([sys.executable, '-m', 'gunicorn', '--config', str(root / 'gunicorn.conf.py'), '--bind', f'127.0.0.1:{api_port}', 'portal_app:create_app()'], cwd=root, env=env, stdout=api_log, stderr=api_log)
            proxy = subprocess.Popen([str(caddy), 'run', '--config', str(folder / 'Caddyfile'), '--adapter', 'caddyfile'], env=env, stdout=tls_log, stderr=tls_log)
            try:
                ca = folder / 'tls/pki/authorities/local/root.crt'
                ready = False
                for _ in range(100):
                    if api.poll() is not None or proxy.poll() is not None:
                        raise AssertionError('Fixture service exited; inspect sanitized service logs')
                    if ca.exists():
                        context = ssl.create_default_context(cafile=str(ca))
                        try:
                            if urllib.request.urlopen(credentials['serverUrl'] + '/healthz', context=context, timeout=1).status == 200:
                                ready = True
                                break
                        except (OSError, urllib.error.URLError):
                            pass
                    time.sleep(.1)
                assert ready, 'TLS services did not become ready'
                def request(path, data=None):
                    req = urllib.request.Request(credentials['serverUrl'] + path, data=json.dumps(data).encode() if data is not None else None, headers={'Authorization': 'Bearer ' + credentials['apiKey'], 'Content-Type': 'application/json'})
                    try:
                        with urllib.request.urlopen(req, context=context, timeout=5) as reply:
                            return reply.status, reply.read()
                    except urllib.error.HTTPError as error:
                        return error.code, error.read()
                assert request('/api/v1/state')[0] == 200
                token = secrets.token_hex(32)
                now = int(time.time())
                record = {'id': 1, 'tokenHash': hashlib.sha256(token.encode()).hexdigest(), 'expiresAtUtc': utc(now + 86400), 'updatedAtUtc': utc(now), 'brand': 'Synthetic', 'model': 'TLS-Control', 'status': 'Received', 'message': 'Synthetic test only', 'history': []}
                snapshot = {'schemaVersion': 1, 'sourceId': credentials['sourceId'], 'expectedRevision': 0, 'records': [record]}
                assert request('/api/v1/snapshot', snapshot)[0] == 200
                assert request('/t/' + token)[0] == 200
                snapshot.update(expectedRevision=1, records=[])
                assert request('/api/v1/snapshot', snapshot)[0] == 200
                assert request('/t/' + token)[0] == 404
                if native:
                    process = subprocess.run([str(Path(native).resolve()), 'realServerIntegration', '-o', '-,txt'], env=env | {'MR_PORTAL_TEST_CREDENTIALS': str(folder / 'cfg/client-credentials.json'), 'MR_PORTAL_TEST_CA': str(ca), 'QT_QPA_PLATFORM': 'offscreen'}, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=45)
                    output = process.stdout.decode(errors='replace')
                    assert process.returncode == 0 and 'PASS   : CloudTests::realServerIntegration()' in output, output
                    print('Native Qt -> verified TLS -> real Python service -> status/revocation passed')
                # Provoke a proxy error, then prove neither tokens nor API keys were logged.
                api.send_signal(signal.SIGQUIT)
                api.wait(timeout=15)
                request('/t/' + token)
                time.sleep(.2)
                for name in ('gunicorn.log', 'caddy.log'):
                    logged = (folder / name).read_text(errors='replace')
                    assert credentials['apiKey'] not in logged and token not in logged, 'Sensitive request data leaked into logs'
                print('Verified TLS, authenticated publish, customer page, revocation and log redaction passed')
            finally:
                for process in (api, proxy):
                    if process.poll() is None:
                        process.terminate()
                        try:
                            process.wait(timeout=10)
                        except subprocess.TimeoutExpired:
                            process.kill();process.wait()


if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('--caddy', required=True)
    p.add_argument('--native-tests')
    args = p.parse_args()
    run(Path(args.caddy).resolve(), args.native_tests)
