"""Exercise the real pinned scanner against the reviewed receipt and mutations."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--gitleaks", required=True)
    args = parser.parse_args()
    repo = Path(__file__).resolve().parents[1]
    relative = Path("evidence/identity-2026-10-24/windows-sodium-build-receipt.json")
    original = json.loads((repo / relative).read_text(encoding="utf-8"))
    keys = ["include/sodium/" + name + ".h" for name in ("crypto_auth", "crypto_auth_hmacsha256", "crypto_auth_hmacsha512", "crypto_auth_hmacsha512256", "crypto_onetimeauth", "crypto_onetimeauth_poly1305", "crypto_secretbox")]
    with tempfile.TemporaryDirectory(prefix="mr-public-digest-") as folder:
        root = Path(folder)
        receipt = root / relative
        receipt.parent.mkdir(parents=True)
        def scan(value):
            receipt.write_text(json.dumps(value, indent=2), encoding="utf-8")
            return subprocess.run([str(Path(args.gitleaks).resolve()), "dir", "--config", str(repo / ".gitleaks.toml"), "--ignore-gitleaks-allow", "--redact=100", "--no-banner", str(root)], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=30).returncode
        if scan(original) != 0:
            raise RuntimeError("Reviewed public file digests were not accepted")
        for key in keys:
            modified = json.loads(json.dumps(original))
            # A different synthetic digest at the SAME path/key must be detected.
            modified["files"][key] = hashlib.sha256(("SYNTHETIC changed value: " + key).encode()).hexdigest()
            if scan(modified) != 1:
                raise RuntimeError("Changed value at a reviewed key was not detected: " + key)
        receipt.write_text(json.dumps(original, indent=2), encoding="utf-8")
        (root / "unreviewed-receipt.json").write_text(json.dumps(original, indent=2), encoding="utf-8")
        if scan(original) != 1:
            raise RuntimeError("Public digests outside the exact reviewed path were incorrectly ignored")
        print("Real scanner: reviewed receipt accepted; seven changed values and an unreviewed path detected.")
    # These are public source-file digests, not keys. Verify the producer's
    # pinned Git bytes independently before exercising the exact exceptions.
    manifest_names = ("source-inputs.json", "verified-source-inputs-6597367.json",
                      "verified-source-inputs-8e7e174.json")
    source_paths = ("cpp/src/UpdateKeyStore.cpp", "cpp/src/UpdateKeyStore.hpp",
                    "cpp/tests/update_key_tests.cpp", "qml/DesignTokens.qml")
    for manifest_name in manifest_names:
        relative = Path("evidence/policy-trust-2026-10-26") / manifest_name
        original = json.loads((repo / relative).read_text(encoding="utf-8"))
        source = original["source"]
        if len(source) != 40 or any(c not in "0123456789abcdef" for c in source):
            raise RuntimeError("Invalid pinned source commit")
        for source_path in source_paths:
            result = subprocess.run(["git", "show", source + ":" + source_path],
                                    cwd=repo, capture_output=True, check=True,
                                    timeout=30)
            if hashlib.sha256(result.stdout).hexdigest() != original["inputs"][source_path]:
                raise RuntimeError("Public source digest differs: " + source_path)
        with tempfile.TemporaryDirectory(prefix="mr-source-digest-") as folder:
            root = Path(folder)
            receipt = root / relative
            receipt.parent.mkdir(parents=True)
            def scan_manifest(value):
                receipt.write_text(json.dumps(value, indent=2), encoding="utf-8")
                return subprocess.run(
                    [str(Path(args.gitleaks).resolve()), "dir", "--config",
                     str(repo / ".gitleaks.toml"), "--ignore-gitleaks-allow",
                     "--redact=100", "--no-banner", str(root)],
                    stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                    timeout=30).returncode
            if scan_manifest(original) != 0:
                raise RuntimeError("Reviewed source manifest was not accepted")
            for source_path in source_paths:
                modified = json.loads(json.dumps(original))
                modified["inputs"][source_path] = hashlib.sha256(
                    ("SYNTHETIC changed source: " + source_path).encode()).hexdigest()
                if scan_manifest(modified) != 1:
                    raise RuntimeError("Changed source digest escaped detection")
            receipt.write_text(json.dumps(original, indent=2), encoding="utf-8")
            (root / "unreviewed-source-manifest.json").write_text(
                json.dumps(original, indent=2), encoding="utf-8")
            if scan_manifest(original) != 1:
                raise RuntimeError("Source hashes escaped exact path restriction")
    print("Real scanner: three source manifests verified against pinned Git bytes; "
          "twelve changed values and three unreviewed paths detected.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
