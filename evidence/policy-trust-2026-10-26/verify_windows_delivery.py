"""Read-only local verification of downloaded evaluation artifacts; executes nothing."""
from pathlib import Path
import hashlib
import io
import json
import re
import sys
import zipfile


def sha(data):
    return hashlib.sha256(data).hexdigest()


def verify(root, source):
    root = Path(root)
    meta = json.loads((root / "artifact-metadata.json").read_text())
    archives = {}
    hashes = {}
    for label in ("portable", "installer", "evidence"):
        data = (root / (label + ".zip")).read_bytes()
        digest = sha(data)
        assert digest == meta[label]["digest"].removeprefix("sha256:"), label
        hashes[label] = digest
        archives[label] = zipfile.ZipFile(io.BytesIO(data))
    portable_outer = archives["portable"]
    nested = [x for x in portable_outer.namelist() if x.endswith("-portable-unsigned.zip")]
    assert len(nested) == 1
    portable_data = portable_outer.read(nested[0])
    bundle = zipfile.ZipFile(io.BytesIO(portable_data))
    members = {}
    for item in bundle.infolist():
        name = item.filename.replace("\\", "/")
        assert not name.startswith("/") and ":" not in name and ".." not in name.split("/")
        assert name.casefold() not in {n.casefold() for n in members}
        if not item.is_dir():
            assert item.file_size <= 256 * 1024 * 1024
            members[name] = item
    profile_names = [n for n in members if n.endswith("/BUILD_PROFILE.json")]
    assert len(profile_names) == 1
    prefix = profile_names[0].removesuffix("BUILD_PROFILE.json")
    profile = json.loads(bundle.read(members[profile_names[0]]).decode("utf-8-sig"))
    assert profile["commit"] == source and profile["buildProfile"] == "production-read-only"
    assert profile["signed"] is False
    manifest = bundle.read(members[prefix + "SHA256SUMS.txt"]).decode("utf-8-sig")
    declared = {}
    for line in manifest.splitlines():
        match = re.fullmatch(r"([0-9a-f]{64})  (.+)", line)
        assert match, line
        name = prefix + match[2].replace("\\", "/")
        assert name in members and name not in declared, name
        declared[name] = match[1]
    assert set(declared) == set(members) - {prefix + "SHA256SUMS.txt"}
    assert sum(x.file_size for x in members.values()) < 2 * 1024**3
    for name, digest in declared.items():
        with bundle.open(members[name]) as stream:
            assert hashlib.file_digest(stream, "sha256").hexdigest() == digest, name
    exe_hash = declared[prefix + "NEXVARY-MR.exe"]
    worker_hash = declared[prefix + "mr_evidence_import_worker.exe"]
    evidence = archives["evidence"]
    evidence_prefixes = [n.removesuffix("portable.result.json") for n in evidence.namelist() if n.endswith("portable.result.json")]
    assert len(evidence_prefixes) == 1
    evidence_prefix = evidence_prefixes[0]

    def report(name):
        return json.loads(evidence.read(evidence_prefix + name).decode("utf-8-sig"))

    for label, mode in (("portable", "isolated-copy"), ("installed", "installed-in-place")):
        receipt = report(label + ".result.json")
        assert receipt["sourceCommit"] == source and receipt["executableSha256"] == exe_hash
        assert receipt["importWorkerSha256"] == worker_hash
        assert receipt["exitCode"] == 0 and receipt["mode"] == mode
        assert receipt["qtSdkExcludedFromPath"] is True and receipt["testFontOverrideCleared"] is True
        audit = report(label + "-government-policy-independent-verification.json")
        assert audit["valid"] is True and audit["retainedPrefixVerified"] > 0
        for language in ("ar", "en"):
            frame = evidence.read(evidence_prefix + label + "-government-offline-" + language + ".png")
            assert frame.startswith(b"\x89PNG\r\n\x1a\n") and len(frame) > 1000
    lifecycle = report("installer-lifecycle.json")
    assert lifecycle["installExit"] == lifecycle["uninstallExit"] == 0
    assert lifecycle["installedExecutableRemoved"] is True
    assert lifecycle["perUserUninstallRegistrationRemoved"] is True
    installer = archives["installer"]
    setup_names = [n for n in installer.namelist() if n.endswith("-setup-unsigned.exe")]
    checksum_names = [n for n in installer.namelist() if n.endswith("SHA256SUMS.txt")]
    assert len(setup_names) == len(checksum_names) == 1
    checksums = installer.read(checksum_names[0]).decode("utf-8-sig")
    expected = dict((m[2], m[1]) for line in checksums.splitlines() if (m := re.fullmatch(r"([0-9a-f]{64})  (.+)", line)))
    setup_hash = sha(installer.read(setup_names[0]))
    assert setup_hash == expected[Path(setup_names[0]).name]
    assert sha(portable_data) == expected[Path(nested[0]).name]
    result = {"source": source, "downloadedProviderDigestsVerified": hashes,
              "portableZipSha256": sha(portable_data), "setupSha256": setup_hash,
              "testedExecutableSha256": exe_hash, "testedImportWorkerSha256": worker_hash,
              "bundleManifestFilesVerified": len(declared), "portableAndInstalledReceiptsMatch": True,
              "installerLifecycleVerified": True, "signed": False,
              "scope": "Downloaded unsigned CI evaluation artifacts; supplied provider digests and source metadata, byte integrity and receipt consistency. No publisher signature, reproducible-build, clean physical Windows or institutional accreditation proof."}
    (root / "delivery-verification.json").write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    verify(sys.argv[1], sys.argv[2])
