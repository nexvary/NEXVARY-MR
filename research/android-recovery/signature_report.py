"""Offline JSON report for signature candidates in an existing evidence image."""
import argparse
import hashlib
import json
from pathlib import Path
from signature_scan import scan_image


def report(path: Path, *, max_bytes: int = 4 * 1024 * 1024 * 1024) -> dict:
    candidates = scan_image(path, max_bytes=max_bytes)
    digest = hashlib.sha256()
    with Path(path).open("rb") as evidence:
        for chunk in iter(lambda: evidence.read(1024 * 1024), b""):
            digest.update(chunk)
    return {
        "schema": 1,
        "source": str(Path(path).name),
        "sha256": digest.hexdigest(),
        "candidate_count": len(candidates),
        "candidates": [{"format": candidate.format, "offset": candidate.offset}
                       for candidate in candidates],
        "warning": "Signature matches are unverified candidates, not recovered files."
    }


def main() -> int:
    parser = argparse.ArgumentParser(description="Read-only synthetic evidence signature scan")
    parser.add_argument("image", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    try:
        if args.output is not None and args.output.resolve() == args.image.resolve():
            raise ValueError("Output cannot overwrite the evidence image")
        result = report(args.image)
        payload = json.dumps(result, indent=2, ensure_ascii=False) + "\n"
        if args.output:
            args.output.write_text(payload, encoding="utf-8")
        else:
            print(payload, end="")
        return 0
    except (OSError, ValueError) as error:
        parser.exit(2, f"Error: {error}\n")


if __name__ == "__main__":
    raise SystemExit(main())
