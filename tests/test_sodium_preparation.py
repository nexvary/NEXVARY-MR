import hashlib
import io
from pathlib import Path
import tarfile
import tempfile
import unittest
from unittest import mock
import zipfile

from scripts import prepare_sodium as dependency


class SodiumPreparationTests(unittest.TestCase):
    def test_bad_cached_archive_never_downloaded_or_extracted(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            archive = root / "fixture.zip"
            archive.write_bytes(b"altered archive")
            with mock.patch.object(dependency.urllib.request, "urlopen") as network:
                with self.assertRaisesRegex(ValueError, "SHA256"):
                    dependency.fetch_archive(root, archive.name, "0" * 64)
                network.assert_not_called()
            self.assertEqual(archive.read_bytes(), b"altered archive")
            dependency.checked_archive(archive, hashlib.sha256(archive.read_bytes()).hexdigest())

    @staticmethod
    def archive(path, extras=()):
        with zipfile.ZipFile(path, "w") as bundle:
            bundle.writestr("libsodium/include/sodium.h", b"synthetic header")
            bundle.writestr("libsodium/x64/Release/v143/dynamic/libsodium.lib", b"synthetic import")
            bundle.writestr("libsodium/x64/Release/v143/dynamic/libsodium.dll", b"synthetic release")
            bundle.writestr("libsodium/x64/Debug/v143/dynamic/libsodium.dll", b"do not deploy")
            bundle.writestr("libsodium/ARM64/Release/v143/dynamic/libsodium.dll", b"wrong architecture")
            for name, value in extras:
                bundle.writestr(name, value)

    def test_only_approved_release_x64_members_extracted(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            archive = root / "fixture.zip"
            self.archive(archive)
            target = root / "output"
            dependency.extract_windows(archive, target)
            self.assertEqual((target / "bin/libsodium.dll").read_bytes(), b"synthetic release")
            self.assertEqual({p.relative_to(target).as_posix() for p in target.rglob("*") if p.is_file()},
                             {"bin/libsodium.dll", "lib/libsodium.lib", "include/sodium.h"})

    def test_zip_traversal_and_normalized_duplicates_rejected_before_writing(self):
        for name in ("../escape", "/absolute", "C:/escape", "libsodium/include/../escape",
                     "libsodium\\include\\escape", "libsodium/include//sodium.h",
                     "libsodium/include/esc\x00ape", "libsodium/include/esc\tape"):
            with self.subTest(name=name), tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                archive = root / "fixture.zip"
                # ZipInfo normalizes os.sep on Windows while writing. Preserve
                # the actual hostile name in BOTH local/central headers so the
                # extractor, rather than the fixture writer, rejects it.
                stored_name = name.replace("\\", "/").replace("\x00", "X")
                self.archive(archive, [(stored_name, b"synthetic")])
                if stored_name != name:
                    archive.write_bytes(archive.read_bytes().replace(stored_name.encode("ascii"), name.encode("ascii")))
                target = root / "output"
                with self.assertRaises(ValueError):
                    dependency.extract_windows(archive, target)
                self.assertFalse(target.exists())

    def test_parser_normalization_never_hides_unsafe_original_name(self):
        # Exercise Windows reader normalization on every host, in addition to
        # the real hostile ZIPs above. No archive bytes may be read or written.
        for original in ("libsodium\\include\\escape", "libsodium/include/sodium.h\x00ignored"):
            with self.subTest(original=original), tempfile.TemporaryDirectory() as temporary:
                entry = zipfile.ZipInfo("libsodium/include/sodium.h")
                entry.orig_filename = original
                with mock.patch.object(dependency.zipfile, "ZipFile") as archive:
                    bundle = archive.return_value.__enter__.return_value
                    bundle.infolist.return_value = [entry]
                    with self.assertRaises(ValueError):
                        dependency.extract_windows(Path("synthetic.zip"), Path(temporary) / "output")
                    bundle.read.assert_not_called()
                self.assertFalse((Path(temporary) / "output").exists())

    def test_zip_symlink_header_refused(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            archive = root / "fixture.zip"
            link = zipfile.ZipInfo("libsodium/include/sodium.h")
            link.external_attr = 0o120777 << 16
            with zipfile.ZipFile(archive, "w") as bundle:
                bundle.writestr(link, b"../../escape")
            with self.assertRaises(ValueError):
                dependency.extract_windows(archive, root / "output")
            self.assertFalse((root / "output").exists())

    def test_missing_release_runtime_refused(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            archive = root / "fixture.zip"
            with zipfile.ZipFile(archive, "w") as bundle:
                bundle.writestr("libsodium/include/sodium.h", b"synthetic")
            with self.assertRaisesRegex(ValueError, "missing"):
                dependency.extract_windows(archive, root / "output")
            self.assertFalse((root / "output").exists())

    def test_source_links_refused_before_running_configure(self):
        for kind in (tarfile.SYMTYPE, tarfile.LNKTYPE):
            with self.subTest(kind=kind), tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                archive = root / "fixture.tar.gz"
                with tarfile.open(archive, "w:gz") as bundle:
                    entry = tarfile.TarInfo("source/escape")
                    entry.type = kind
                    entry.linkname = "../../outside"
                    bundle.addfile(entry, io.BytesIO())
                target = root / "output"
                with mock.patch.object(dependency.subprocess, "Popen") as child:
                    with self.assertRaisesRegex(ValueError, "nonregular"):
                        dependency.build_source(archive, target)
                    child.assert_not_called()


if __name__ == "__main__":
    unittest.main()
