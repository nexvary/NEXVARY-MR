#!/usr/bin/env python3
"""Independent Python tarfile fixtures against the native, inventory-only worker.

All bytes are synthetic. No Android device, restore, decryption or extraction.
"""
import base64
import hashlib
import io
import json
from pathlib import Path
import subprocess
import sys
import tarfile
import tempfile

HEADER = b"ANDROID BACKUP\n5\n0\nnone\n"
REFERENCE = Path(__file__).resolve().parents[1] / "samples/authorized-import/adb-v5-reference.json"


def archive(entries):
    stream = io.BytesIO()
    with tarfile.open(fileobj=stream, mode="w", format=tarfile.USTAR_FORMAT, encoding="utf-8") as tar:
        for name, payload, kind in entries:
            info = tarfile.TarInfo(name)
            info.type = kind
            info.mode = 0o700 if kind == tarfile.DIRTYPE else 0o600
            info.mtime = 1761393600
            info.size = len(payload)
            if kind in (tarfile.SYMTYPE, tarfile.LNKTYPE):
                info.linkname = "apps/com.nexvary.synthetic/f/note.txt"
            tar.addfile(info, io.BytesIO(payload))
    return HEADER + stream.getvalue()


def reference():
    entries = [
        ("apps/com.nexvary.synthetic/_manifest", b"1\ncom.nexvary.synthetic\n1\n28\n\n0\n0\n", tarfile.REGTYPE),
        ("apps/com.nexvary.synthetic/f/", b"", tarfile.DIRTYPE),
        ("apps/com.nexvary.synthetic/f/note.txt", "دليل تدريبي اصطناعي فقط\n".encode(), tarfile.REGTYPE),
        ("apps/com.nexvary.synthetic/db/opaque.bin", bytes(range(256)), tarfile.REGTYPE),
    ]
    blob = archive(entries)
    expected = []
    # Independent stdlib reader supplies expected offsets/content hashes.
    with tarfile.open(fileobj=io.BytesIO(blob[len(HEADER):]), mode="r:", encoding="utf-8") as tar:
        for member in tar:
            payload = tar.extractfile(member).read() if member.isfile() else b""
            expected.append(dict(reference=member.name.rstrip("/"), memberSize=member.size,
                                 memberSha256=hashlib.sha256(payload).hexdigest() if member.isfile() else "",
                                 entryType="file" if member.isfile() else "directory",
                                 rawMtimeEpochSeconds=str(member.mtime), sourceByteOffset=len(HEADER) + member.offset_data))
    return dict(schema="nexvary-synthetic-adb-fixture/v1", generator="Python stdlib tarfile USTAR_FORMAT",
                synthetic=True, backupBase64=base64.b64encode(blob).decode(),
                sourceSha256=hashlib.sha256(blob).hexdigest(), expected=expected)


def rewrite(blob, offset, value):
    data = bytearray(blob)
    start = len(HEADER)
    data[start + offset:start + offset + len(value)] = value
    data[start + 148:start + 156] = b" " * 8
    checksum = sum(data[start:start + 512])
    data[start + 148:start + 156] = f"{checksum:06o}".encode() + b"\0 "
    return bytes(data)


def main():
    generated = reference()
    if sys.argv[1:] == ["--write-reference"]:
        REFERENCE.parent.mkdir(parents=True, exist_ok=True)
        REFERENCE.write_text(json.dumps(generated, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        return
    assert generated == json.loads(REFERENCE.read_text(encoding="utf-8")), "Fixture regeneration differs"
    worker = Path(sys.argv[1]).resolve()
    checks = []
    with tempfile.TemporaryDirectory(prefix="nexvary-adb-interop-") as directory:
        root = Path(directory)

        def run(name, blob, valid):
            path = root / "synthetic.ab"
            path.write_bytes(blob)
            digest = hashlib.sha256(blob).hexdigest()
            result = subprocess.run([str(worker), "--input", str(path), "--sha256", digest,
                                     "--adapter", "adb-backup-v5"], cwd=root, capture_output=True, timeout=10)
            assert path.read_bytes() == blob, name + ": source changed"
            assert sorted(p.name for p in root.iterdir()) == ["synthetic.ab"], name + ": archive member extracted"
            if valid:
                assert result.returncode == 0, (name, result.returncode, result.stderr.decode(errors="replace"))
                report = json.loads(result.stdout)
                assert report["sourceSha256"] == digest and report["sourceSize"] == len(blob)
                assert report["backup"]["inventoryOnly"] is True
            else:
                assert result.returncode != 0 and not result.stdout, name + ": accepted unsupported or malformed data"
                report = None
            checks.append(dict(name=name, passed=True, sourceSha256=digest))
            return report

        blob = base64.b64decode(generated["backupBase64"])
        report = run("populated-python-ustar", blob, True)
        assert report == run("deterministic-repeat", blob, True)
        assert report["observedCount"] == 4 and report["findingsTruncated"] is False
        for got, expected in zip(report["findings"], generated["expected"], strict=True):
            for key, value in expected.items():
                assert got[key] == value, (key, got[key], value)
        one = archive([("apps/com.nexvary.synthetic/f/note.txt", b"test", tarfile.REGTYPE)])
        for name, bad in [
            ("wrong-magic", blob.replace(b"ANDROID BACKUP", b"ANDROID BACXUP", 1)),
            ("older-version", blob.replace(b"\n5\n", b"\n4\n", 1)),
            ("compressed", blob.replace(b"\n0\n", b"\n1\n", 1)),
            ("encrypted", blob.replace(b"none\n", b"AES-256\n", 1)),
            ("checksum", one[:len(HEADER)] + b"X" + one[len(HEADER) + 1:]),
            ("size-overflow", rewrite(one, 124, b"77777777777\0")),
            ("base256-size", rewrite(one, 124, b"\x80" + b"\0" * 11)),
            ("bad-octal", rewrite(one, 136, b"00000000009\0")),
            ("gnu-magic", rewrite(one, 257, b"ustar  \0")),
            ("nonzero-padding", one[:len(HEADER) + 516] + b"X" + one[len(HEADER) + 517:]),
            ("truncated-payload", one[:len(HEADER) + 514]),
            ("missing-end-blocks", one[:len(HEADER) + 1024]),
            ("single-end-block", one[:len(HEADER) + 1536]),
            ("trailing-payload", one + b"X" * 512),
            ("hidden-name-suffix", rewrite(one, 0, b"apps/com.nexvary.synthetic/f/x\0hidden")),
            ("invalid-utf8-name", rewrite(one, 0, b"apps/com.nexvary.synthetic/f/\xc0\x80\0")),
            ("incomplete-utf8-name", rewrite(one, 0, b"apps/com.nexvary.synthetic/f/\xe2\0")),
            ("empty-archive", HEADER + b"\0" * 1024),
            ("no-block-alignment", one[:-1]),
        ]:
            run(name, bad, False)
        for name in ["../escape", "/apps/com.nexvary.synthetic/f/x", "apps/com.nexvary.synthetic/f/../x",
                     "apps/com.nexvary.synthetic/f//x", "apps/com.nexvary.synthetic/f/./x",
                     "apps/com.nexvary.synthetic/f/C:x", "apps/com.nexvary.synthetic/f/a\\b",
                     "shared/0/x", "apps/bad-package/f/x", "apps/com.nexvary.synthetic/unknown",
                     "apps/com.nexvary.synthetic/f/\u202ex", "apps/com.nexvary.synthetic/f/\x01x"]:
            run("unsafe-path-" + repr(name), archive([(name, b"test", tarfile.REGTYPE)]), False)
        for kind in [tarfile.SYMTYPE, tarfile.LNKTYPE, tarfile.FIFOTYPE, tarfile.CHRTYPE, tarfile.BLKTYPE,
                     tarfile.XHDTYPE, tarfile.XGLTYPE, tarfile.GNUTYPE_LONGNAME, tarfile.GNUTYPE_SPARSE]:
            run("unsupported-type-" + repr(kind), archive([("apps/com.nexvary.synthetic/f/x", b"", kind)]), False)
        for name, entries in [
            ("duplicate-member", [("apps/com.nexvary.synthetic/f/x", b"test", tarfile.REGTYPE)] * 2),
            ("file-ancestor", [("apps/com.nexvary.synthetic/f/x", b"test", tarfile.REGTYPE),
                               ("apps/com.nexvary.synthetic/f/x/y", b"test", tarfile.REGTYPE)]),
            ("nonempty-directory", [("apps/com.nexvary.synthetic/f/", b"test", tarfile.DIRTYPE)]),
        ]:
            run(name, archive(entries), False)
        many = [(f"apps/com.nexvary.synthetic/f/{i}.bin", bytes([i % 256]), tarfile.REGTYPE) for i in range(130)]
        limited = run("bounded-findings-full-scan", archive(many), True)
        assert limited["observedCount"] == 130 and len(limited["findings"]) == 128 and limited["findingsTruncated"]
        run("malformed-after-retention-limit", archive(many + [("../escape", b"bad", tarfile.REGTYPE)]), False)
        run("header-member-limit", archive([(f"apps/com.nexvary.synthetic/f/{i}", b"", tarfile.REGTYPE)
                                            for i in range(4097)]), False)
        at_limit = run("exact-header-limit", archive([(f"apps/com.nexvary.synthetic/f/{i}", b"", tarfile.REGTYPE)
                                                     for i in range(4096)]), True)
        assert at_limit["observedCount"] == 4096 and at_limit["backup"]["files"] == 4096
        for name in ["apps/com.nexvary.synthetic/f/دليل.txt", "apps/com.nexvary.synthetic/f/" + "a" * 90 + "/leaf.txt"]:
            accepted = run("unicode-or-ustar-prefix-" + name, archive([(name, b"", tarfile.REGTYPE)]), True)
            assert accepted["findings"][0]["reference"] == name
            assert accepted["findings"][0]["memberSha256"] == hashlib.sha256(b"").hexdigest()
        run("input-byte-limit", blob + b"\0" * (4 * 1024 * 1024), False)
    receipt = dict(schema="nexvary-adb-interop-results/v1", checks=checks, passed=len(checks), failed=0,
                   scope="Synthetic Python/native inventory interoperability; no hardware, app decoding or forensic accreditation")
    Path("adb-backup-interop-results.json").write_text(json.dumps(receipt, indent=2) + "\n", encoding="utf-8")
    print(f"ADB backup interoperability: {len(checks)} checks passed")


if __name__ == "__main__":
    main()
