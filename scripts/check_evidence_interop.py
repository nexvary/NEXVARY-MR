"""Verify native Qt export through the separate standard-library verifier."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile
from verify_evidence_package import verify_package


def main():
    with tempfile.TemporaryDirectory(prefix="mr-evidence-interop-") as directory:
        package = Path(directory) / "package"
        env = dict(os.environ, MR_EVIDENCE_FIXTURE_DIR=str(package))
        completed = subprocess.run([sys.argv[1], "workingCopyIntegrityAndAudit", "-o", "-,txt"], env=env, timeout=45, check=False)
        if completed.returncode:
            return completed.returncode
        digest = Path(str(package) + ".sha256").read_text(encoding="ascii")
        result = verify_package(package, digest)
        assert result["integrityVerified"] and result["synthetic"] and not result["externalAuthenticityVerified"]
        evidence = package / "evidence.bin"
        content = evidence.read_bytes()
        evidence.write_bytes(bytes([content[0] ^ 1]) + content[1:])
        try:
            verify_package(package, digest)
        except ValueError:
            print("Native export verified independently; same-size corruption rejected.")
            return 0
        raise AssertionError("Verifier accepted corrupted native evidence")


if __name__ == "__main__":
    sys.exit(main())
