#!/usr/bin/env python3
"""Build the reviewed API-only GPL engine; fail closed on security/test errors."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tarfile

UPSTREAM = "55807b05b753dab7bc6fe67403ca385668dda179"
ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--govulncheck", required=True)
    args = parser.parse_args()
    source, output = args.source.resolve(), args.output.resolve()
    if subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=source, text=True).strip() != UPSTREAM:
        raise RuntimeError("Unreviewed upstream revision")
    if subprocess.check_output(["git", "status", "--porcelain"], cwd=source).strip():
        raise RuntimeError("Use a clean dedicated engine checkout")
    output.mkdir(parents=True, exist_ok=False)
    overlay = ROOT / "third_party" / "phoneinfoga"
    patch = overlay / "localhost-session.patch"
    subprocess.run(["git", "apply", "--check", str(patch)], cwd=source, check=True)
    subprocess.run(["git", "apply", str(patch)], cwd=source, check=True)
    for name in ("mod", "sum"):
        shutil.copyfile(overlay / ("engine.go." + name), source / ("go." + name))
    environment = dict(os.environ, GOTOOLCHAIN="local", CGO_ENABLED="0")
    go_version = subprocess.check_output(["go", "version"], env=environment, text=True).strip()
    if "go1.26.10 " not in go_version:
        raise RuntimeError("Use the pinned Go 1.26.10 toolchain")

    def run(command, log):
        with (output / log).open("wb") as stream:
            result = subprocess.run(command, cwd=source, env=environment, stdout=stream,
                                    stderr=subprocess.STDOUT, timeout=600)
        if result.returncode:
            print((output / log).read_text(encoding="utf-8", errors="replace")[-16000:], file=sys.stderr)
            raise subprocess.CalledProcessError(result.returncode, command)

    run(["go", "mod", "verify"], "module-verification.txt")
    run(["go", "test", "./lib/...", "./web/v2/api/..."], "upstream-scanner-tests.txt")
    run(["go", "test", "./web", "-run", "^TestNexvary"], "session-security-tests.txt")
    # A nonzero vulnerability result blocks engine packaging.
    run([args.govulncheck, "-json", "./..."], "govulncheck.json")
    binary = output / ("phoneinfoga.exe" if os.name == "nt" else "phoneinfoga")
    run(["go", "build", "-trimpath", "-o", str(binary), "."], "engine-build.txt")
    run(["go", "mod", "vendor"], "vendor-source.txt")
    (source / "NEXVARY_BUILD.md").write_text(
        "# Modified PhoneInfoga engine\n\nGPL-3.0; upstream copyright notices retained.\n"
        "NEXVARY modifications: localhost-only API, private session authentication, "
        "no embedded browser, reviewed dependency updates and SDK-error semantic tests.\n"
        "Build with Go 1.26.10: `CGO_ENABLED=0 go build -mod=vendor -trimpath .`\n"
        "Serve with --no-client. Set NEXVARY_PHONEINFOGA_TOKEN to a private random "
        "64-character hex token. NEXVARY-MR manages its lifetime.\n", encoding="utf-8")
    archive = output / "phoneinfoga-corresponding-source.tar.gz"
    with tarfile.open(archive, "w:gz") as tar:
        for path in sorted(source.rglob("*")):
            relative = path.relative_to(source)
            if ".git" not in relative.parts and path.is_file():
                tar.add(path, arcname=Path("phoneinfoga") / relative, recursive=False)
    shutil.copyfile(overlay / "LICENSE", output / "LICENSE")
    receipt = {
        "schema": "nexvary-phoneinfoga-build/v1", "upstreamCommit": UPSTREAM,
        "goVersion": go_version, "patchSha256": hashlib.sha256(patch.read_bytes()).hexdigest(),
        "moduleManifestSha256": hashlib.sha256((source / "go.mod").read_bytes()).hexdigest(),
        "executableSha256": hashlib.sha256(binary.read_bytes()).hexdigest(),
        "sourceArchiveSha256": hashlib.sha256(archive.read_bytes()).hexdigest(),
        "securityScan": "govulncheck passed; not a guarantee of absence of vulnerabilities",
        "license": "GPL-3.0", "binary": binary.name,
    }
    (output / "build-receipt.json").write_text(json.dumps(receipt, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(receipt))


if __name__ == "__main__":
    try:
        main()
    except (OSError, RuntimeError, subprocess.SubprocessError) as error:
        print(str(error), file=sys.stderr)
        sys.exit(1)
