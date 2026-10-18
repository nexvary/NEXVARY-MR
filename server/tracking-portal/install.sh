#!/usr/bin/env bash
# NEXVARY MR status portal: Ubuntu 22.04/24.04; Debian 12/13. No SSH/firewall rewrites.
set -Eeuo pipefail
umask 077
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
domain=''; email=''; upgrade=0; dry=0; local_test=0
usage() { cat <<'TXT'
Usage: sudo bash install.sh --domain tracking.example.com --email admin@example.com [--upgrade]
       bash install.sh --domain tracking.example.com --email admin@example.com --dry-run
CI only: --local-test --domain portal.localhost --email test@example.com
Requires a dedicated free TCP 80/443 (9080/9443 for local test), DNS A/AAAA pointing here,
Python 3.10+, systemd, apt, and outgoing HTTPS. Never changes firewall or SSH settings.
Existing unmanaged sites/services are not replaced. Upgrade requires --upgrade.
TXT
}
while (($#)); do
 case "$1" in
  --domain) domain=${2:?Missing domain}; shift 2;;
  --email) email=${2:?Missing email}; shift 2;;
  --upgrade) upgrade=1; shift;;
  --dry-run) dry=1; shift;;
  --local-test) local_test=1; shift;;
  --help|-h) usage; exit 0;;
  *) usage >&2; exit 2;;
 esac
done
[[ -n "$domain" && -n "$email" ]] || { usage >&2; exit 2; }
python3 - "$SCRIPT_DIR" "$domain" "$email" "$local_test" <<'PY'
import re,sys
sys.path.insert(0,sys.argv[1]);from render import valid_domain
if not valid_domain(sys.argv[2]) or not re.fullmatch(r'[A-Za-z0-9._+%-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,63}',sys.argv[3]):
 raise SystemExit('Invalid domain/email. No settings changed.')
if sys.argv[4]=='1' and sys.argv[2]!='portal.localhost':raise SystemExit('Local test mode requires portal.localhost.')
PY
for file in portal_app.py portal_admin.py render.py gunicorn.conf.py requirements.lock; do
 [[ -f "$SCRIPT_DIR/$file" && ! -L "$SCRIPT_DIR/$file" ]] || { echo "Missing bundled file: $file" >&2; exit 1; }
done
if ((dry)); then
 printf 'Validated installer. Target: https://%s\n' "$domain"
 printf '%s\n' 'Would create dedicated system users, isolated versioned venv, verified Caddy binary, HTTPS/API services and daily backups.' 'No changes made. Credentials are generated only during installation; never printed.'
 exit 0
fi
[[ $EUID -eq 0 ]] || { echo 'Run this installer with sudo/root.' >&2; exit 1; }
[[ -d /run/systemd/system ]] || { echo 'A systemd server is required.' >&2; exit 1; }
# Read standard OS identity; support is deliberately explicit.
. /etc/os-release
case "$ID:$VERSION_ID" in ubuntu:22.04|ubuntu:24.04|debian:12|debian:13) ;; *) echo 'Supported: Ubuntu 22.04/24.04, Debian 12/13.' >&2; exit 1;; esac
prefix=/opt/nexvary-portal
config=/etc/nexvary-portal
previous=''
if [[ -e "$prefix/current" || -L "$prefix/current" ]]; then
 [[ -f "$prefix/.nexvary-managed" && $upgrade -eq 1 ]] || { echo 'Existing install: use --upgrade; unmanaged paths are never overwritten.' >&2; exit 1; }
 previous=$(readlink -f "$prefix/current")
 [[ "$previous" == "$prefix/releases/"* && -d "$previous" ]] || { echo 'Invalid managed release path.' >&2; exit 1; }
 [[ -f "$config/settings.json" && -f "$config/Caddyfile" ]] || { echo 'Existing configuration missing; restore it before upgrade.' >&2; exit 1; }
else
 [[ $upgrade -eq 0 && ! -e "$prefix" && ! -e "$config" ]] || { echo 'Refusing to overwrite existing/unmanaged paths.' >&2; exit 1; }
 for unit in nexvary-portal.service nexvary-portal-tls.service nexvary-portal-backup.service nexvary-portal-backup.timer; do
  [[ ! -e "/etc/systemd/system/$unit" ]] || { echo 'Conflicting service unit exists.' >&2; exit 1; }
 done
 python3 - "$local_test" <<'PY'
import socket,sys
ports=[9080,9443,9085] if sys.argv[1]=='1' else [80,443,9085]
for port in ports:
 with socket.socket() as s:
  try:s.bind(('127.0.0.1' if port==9085 or sys.argv[1]=='1' else '0.0.0.0',port))
  except OSError:raise SystemExit(f'Port {port} is occupied. Existing service was not stopped or changed.')
PY
fi
if ((upgrade==0)); then
 for user in nexvary-portal nexvary-portal-tls; do
  if id "$user" >/dev/null 2>&1; then echo 'Conflicting service account exists; refusing reuse.' >&2;exit 1;fi
 done
fi
command -v apt-get >/dev/null
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends python3 python3-venv curl ca-certificates tar util-linux
for user in nexvary-portal nexvary-portal-tls; do
 if ! id "$user" >/dev/null 2>&1; then useradd --system --user-group --no-create-home --shell /usr/sbin/nologin "$user"; fi
done
install -d -m 0755 "$prefix/releases" "$config"
install -d -m 0700 -o nexvary-portal -g nexvary-portal /var/lib/nexvary-portal
install -d -m 0700 -o nexvary-portal-tls -g nexvary-portal-tls /var/lib/nexvary-portal-tls
install -d -m 0700 -o nexvary-portal -g nexvary-portal /var/backups/nexvary-portal
stage=$(mktemp -d "$prefix/releases/release-$(date -u +%Y%m%dT%H%M%S)-XXXXXX")
chmod 0755 "$stage"
install -d -m 0755 "$stage/bin"
for file in portal_app.py portal_admin.py render.py gunicorn.conf.py requirements.lock; do install -m 0644 "$SCRIPT_DIR/$file" "$stage/$file"; done
python3 -m venv "$stage/venv"
"$stage/venv/bin/pip" install --require-hashes --only-binary=:all: -r "$stage/requirements.lock"
# umask 077 protects generated credentials, but the unprivileged API must read its runtime.
# This release directory contains code/dependencies only; secrets live in /etc and /root.
chmod -R a+rX "$stage/venv"
case "$(uname -m)" in
 x86_64) arch=amd64; digest=727b91701a392de6ebc5027509f548bf39979e5216340d0faed8fa5e69c84f8b;;
 aarch64|arm64) arch=arm64; digest=d8fc6d179a5d283028a472a5618564f6ad8a86fed513e64f032b3b0b7cc45e42;;
 *) echo 'Supported server CPU: x86_64/arm64.' >&2; exit 1;;
esac
archive=$(mktemp)
rendered=$(mktemp -d)
trap 'rm -f -- "$archive"; rm -rf -- "$rendered"' EXIT
curl --proto '=https' --tlsv1.2 --fail --location --retry 3 --connect-timeout 10 --max-time 180 "https://github.com/caddyserver/caddy/releases/download/v2.11.7/caddy_2.11.7_linux_${arch}.tar.gz" -o "$archive"
printf '%s  %s\n' "$digest" "$archive" | sha256sum --check --status
# Extract only the verified binary, never arbitrary archive paths.
tar -xzf "$archive" -C "$stage/bin" caddy
chmod 0755 "$stage/bin/caddy"
args=(--domain "$domain" --email "$email" --destination "$rendered")
if ((upgrade)); then args+=(--existing "$config/settings.json"); fi
if ((local_test)); then args+=(--local-test); fi
python3 "$stage/render.py" "${args[@]}"
install -m 0644 "$rendered/Caddyfile" "$stage/validate.Caddyfile"
runuser -u nexvary-portal-tls -- "$stage/bin/caddy" validate --config "$stage/validate.Caddyfile" --adapter caddyfile >/dev/null
# Stage a recovery copy before modifying this application's own configuration.
old_config=''
if ((upgrade)); then
 old_config=$(mktemp -d "$prefix/config-before-upgrade-XXXXXX")
 install -d -m 0700 "$old_config/config" "$old_config/units"
 cp -a "$config/." "$old_config/config/"
 for unit in nexvary-portal.service nexvary-portal-tls.service nexvary-portal-backup.service nexvary-portal-backup.timer; do cp -a "/etc/systemd/system/$unit" "$old_config/units/";done
 systemctl stop nexvary-portal-tls.service nexvary-portal.service
fi
activated=$upgrade
rollback() {
 if ((activated)); then
  systemctl stop nexvary-portal-tls.service nexvary-portal.service || true
  if [[ -n "$previous" ]]; then
   ln -sfn "$previous" "$prefix/rollback-link"
   mv -Tf "$prefix/rollback-link" "$prefix/current"
   cp -a "$old_config/config/." "$config/"
   cp -a "$old_config/units/." /etc/systemd/system/
   systemctl daemon-reload
   systemctl restart nexvary-portal.service nexvary-portal-tls.service || true
  fi
 fi
 echo 'Installation did not pass health checks. Data/configuration were preserved. Check DNS, ports and systemctl status; do not share credentials.' >&2
}
trap 'rollback' ERR
install -m 0640 -o root -g nexvary-portal "$rendered/settings.json" "$config/settings.json"
install -m 0644 "$rendered/Caddyfile" "$config/Caddyfile"
for unit in nexvary-portal.service nexvary-portal-tls.service nexvary-portal-backup.service nexvary-portal-backup.timer; do install -m 0644 "$rendered/$unit" "/etc/systemd/system/$unit"; done
if [[ -f "$rendered/client-credentials.json" ]]; then install -m 0600 "$rendered/client-credentials.json" /root/nexvary-portal-client-credentials.json; fi
ln -sfn "$stage" "$prefix/next-link"
mv -Tf "$prefix/next-link" "$prefix/current"
printf 'NEXVARY portal schema=1\n' > "$prefix/.nexvary-managed"
activated=1
systemctl daemon-reload
systemctl enable nexvary-portal.service nexvary-portal-tls.service nexvary-portal-backup.timer
systemctl restart nexvary-portal.service nexvary-portal-tls.service
healthy=0
for attempt in $(seq 1 30); do
 if curl --fail --silent --max-time 3 http://127.0.0.1:9085/healthz >/dev/null; then healthy=1; break; fi
 sleep 1
done
[[ $healthy -eq 1 ]]
https_ok=0
for attempt in $(seq 1 20); do
 if ((local_test)); then
  if curl --fail --silent --max-time 5 --resolve portal.localhost:9443:127.0.0.1 --cacert /var/lib/nexvary-portal-tls/pki/authorities/local/root.crt https://portal.localhost:9443/healthz >/dev/null 2>&1; then https_ok=1;break;fi
 else
  if curl --proto '=https' --tlsv1.2 --fail --silent --max-time 5 "https://$domain/healthz" >/dev/null 2>&1; then https_ok=1;break;fi
 fi
 sleep 3
done
[[ $https_ok -eq 1 ]]
systemctl start nexvary-portal-backup.timer
trap - ERR
printf 'Portal ready: https://%s' "$domain"
if ((local_test)); then printf ':9443 (LOCAL TEST ONLY)'; fi
printf '\nCredentials: /root/nexvary-portal-client-credentials.json (root-only; import securely into Windows).\n'
printf 'Backup: systemctl start nexvary-portal-backup.service\nStatus: systemctl status nexvary-portal.service nexvary-portal-tls.service\n'
