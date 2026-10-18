#!/usr/bin/env bash
# CI fixture only: the same pinned upstream runtimes used by the server installer.
set -Eeuo pipefail
runtime=${1:?Specify a fixture directory}
source_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
mkdir -p "$runtime"
runtime=$(cd -- "$runtime" && pwd)
python3 -m venv "$runtime/venv"
"$runtime/venv/bin/pip" install --require-hashes --only-binary=:all: -r "$source_dir/requirements.lock"
curl --proto '=https' --tlsv1.2 --fail --location --retry 3 --max-time 180 https://github.com/caddyserver/caddy/releases/download/v2.11.7/caddy_2.11.7_linux_amd64.tar.gz -o "$runtime/caddy.tar.gz"
printf '%s  %s\n' '727b91701a392de6ebc5027509f548bf39979e5216340d0faed8fa5e69c84f8b' "$runtime/caddy.tar.gz" | sha256sum --check --status
tar -xzf "$runtime/caddy.tar.gz" -C "$runtime" caddy
chmod 755 "$runtime/caddy"
