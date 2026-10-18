"""Render reviewed configuration. Never interpolate unchecked shell/Caddy fragments."""
import argparse
import hashlib
import json
import os
import re
import secrets
from pathlib import Path


def valid_domain(value):
    labels = value.split('.')
    return (len(value) <= 253 and len(labels) >= 2 and not value.replace('.', '').isdigit()
            and all(re.fullmatch(r'[a-z0-9](?:[a-z0-9-]{0,61}[a-z0-9])?', label) for label in labels))


def render(domain, email, destination, existing=None, local_test=False):
    if not valid_domain(domain) or not re.fullmatch(r'[A-Za-z0-9._+%-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,63}', email):
        raise ValueError('Use an ASCII public domain and valid certificate contact email')
    if local_test and domain != 'portal.localhost':
        raise ValueError('Local test mode requires portal.localhost')
    target = Path(destination)
    target.mkdir(parents=True, exist_ok=True)
    os.chmod(target, 0o700)
    if existing:
        config = json.loads(Path(existing).read_text())
        if config.get('httpsPort',443) != (9443 if local_test else 443):
            raise ValueError('Preserve TLS mode when upgrading')
        if config['domain'] != domain:
            raise ValueError('An upgrade cannot silently change the portal domain')
    else:
        key = secrets.token_hex(32)
        config = {'domain': domain, 'database': '/var/lib/nexvary-portal/portal.sqlite3',
                  'httpsPort': 9443 if local_test else 443, 'sourceId': secrets.token_hex(16), 'apiKeySha256': hashlib.sha256(key.encode()).hexdigest()}
        credentials = {'schemaVersion': 1, 'serverUrl': 'https://' + domain + (':9443' if local_test else ''), 'sourceId': config['sourceId'], 'apiKey': key}
        write(target / 'client-credentials.json', json.dumps(credentials, indent=2))
    write(target / 'settings.json', json.dumps(config, indent=2))
    caddy = '''{
    admin off
    persist_config off
    email EMAIL
    storage file_system /var/lib/nexvary-portal-tls
    log {
        format filter {
            wrap json
            fields {
                request>uri delete
                request>headers delete
            }
        }
    }
    servers {
        timeouts {
            read_header 10s
            read_body 20s
            write 30s
            idle 30s
        }
        protocols h1 h2
        max_header_size 16KB
    }
}
DOMAIN {
    request_body {
        max_size 4MB
    }
    header {
        Strict-Transport-Security "max-age=31536000"
        -Server
    }
    reverse_proxy 127.0.0.1:9085 {
        request_buffers 4MB
        header_up X-Nexvary-Client-IP {remote_host}
        header_up X-Forwarded-Proto https
    }
}
'''.replace('EMAIL', email).replace('DOMAIN', domain)
    if local_test:
        caddy = caddy.replace('    admin off', '    admin off\n    skip_install_trust\n    http_port 9080\n    https_port 9443\n    default_bind 127.0.0.1').replace(domain+' {', domain+':9443 {\n    tls internal')
    if local_test:
        caddy += '\nhttp://portal.localhost:9080 {\n    bind 127.0.0.1\n    redir https://portal.localhost:9443{uri} 308\n}\n'
    write(target / 'Caddyfile', caddy)
    common = '''[Unit]
Description=DESCRIPTION
After=network-online.target
Wants=network-online.target
StartLimitIntervalSec=60
StartLimitBurst=5

[Service]
Type=simple
User=USER
Group=USER
WorkingDirectory=/opt/nexvary-portal/current
UMask=0077
Restart=on-failure
RestartSec=3
NoNewPrivileges=true
PrivateTmp=true
ProtectSystem=strict
ProtectHome=true
ProtectKernelTunables=true
ProtectKernelModules=true
ProtectControlGroups=true
RestrictSUIDSGID=true
RestrictAddressFamilies=AF_INET AF_INET6 AF_UNIX
LockPersonality=true
LimitNOFILE=4096
MemoryMax=512M
TasksMax=128
READWRITE
COMMAND

[Install]
WantedBy=multi-user.target
'''
    service = common.replace('DESCRIPTION', 'NEXVARY repair status API').replace('USER', 'nexvary-portal')
    service = service.replace('READWRITE', 'ReadWritePaths=/var/lib/nexvary-portal\nIPAddressDeny=any\nIPAddressAllow=localhost')
    service = service.replace('COMMAND', 'Environment=PYTHONDONTWRITEBYTECODE=1\nEnvironment=NEXVARY_PORTAL_CONFIG=/etc/nexvary-portal/settings.json\nExecStart=/opt/nexvary-portal/current/venv/bin/gunicorn --config /opt/nexvary-portal/current/gunicorn.conf.py portal_app:create_app()')
    write(target / 'nexvary-portal.service', service)
    tls = common.replace('DESCRIPTION', 'NEXVARY portal HTTPS and certificate renewal').replace('USER', 'nexvary-portal-tls')
    tls = tls.replace('READWRITE', 'ReadWritePaths=/var/lib/nexvary-portal-tls\nAmbientCapabilities=CAP_NET_BIND_SERVICE\nCapabilityBoundingSet=CAP_NET_BIND_SERVICE')
    tls = tls.replace('COMMAND', 'Environment=XDG_DATA_HOME=/var/lib/nexvary-portal-tls\nExecStart=/opt/nexvary-portal/current/bin/caddy run --config /etc/nexvary-portal/Caddyfile --adapter caddyfile')
    write(target / 'nexvary-portal-tls.service', tls)
    write(target / 'nexvary-portal-backup.service', '''[Unit]
Description=NEXVARY portal online SQLite backup and expired-record cleanup
[Service]
Type=oneshot
User=nexvary-portal
Group=nexvary-portal
UMask=0077
NoNewPrivileges=true
PrivateTmp=true
ProtectHome=true
ProtectSystem=strict
ReadWritePaths=/var/lib/nexvary-portal /var/backups/nexvary-portal
ExecStart=/opt/nexvary-portal/current/venv/bin/python /opt/nexvary-portal/current/portal_admin.py --config /etc/nexvary-portal/settings.json backup --directory /var/backups/nexvary-portal
''')
    write(target / 'nexvary-portal-backup.timer', '''[Unit]
Description=Daily NEXVARY portal backup
[Timer]
OnCalendar=daily
RandomizedDelaySec=3600
Persistent=true
[Install]
WantedBy=timers.target
''')


def write(path, value):
    path.write_text(value + '\n')
    os.chmod(path, 0o600)


if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('--domain', required=True)
    p.add_argument('--email', required=True)
    p.add_argument('--destination', required=True)
    p.add_argument('--existing')
    p.add_argument('--local-test', action='store_true')
    args = p.parse_args()
    render(args.domain, args.email, args.destination, args.existing, args.local_test)
