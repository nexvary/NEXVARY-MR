import hashlib
import importlib.util
from pathlib import Path
import stat
import tempfile
import unittest
from unittest.mock import patch
import zipfile

spec = importlib.util.spec_from_file_location('screen_tools', Path(__file__).parents[1] / 'scripts/prepare_screen_tools.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class ScreenToolsTests(unittest.TestCase):
    def fixture(self, path, extra=None):
        with zipfile.ZipFile(path, 'w') as archive:
            for name in sorted(module.REQUIRED):
                archive.writestr(module.ROOT + '/' + name, b'SYNTHETIC runtime only')
            if extra:
                archive.writestr(*extra)

    def test_rejects_changed_bytes_and_preserves_existing_destination(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            source = root / 'fixture.zip'
            self.fixture(source)
            with self.assertRaises(ValueError):
                module.prepare(source, root / 'not-created')
            self.assertFalse((root / 'not-created').exists())
            destination = root / 'existing'
            destination.mkdir()
            marker = destination / 'original'
            marker.write_text('SYNTHETIC original')
            with patch.object(module, 'SHA256', hashlib.sha256(source.read_bytes()).hexdigest()):
                with self.assertRaises(ValueError):
                    module.prepare(source, destination)
            self.assertEqual(marker.read_text(), 'SYNTHETIC original')

    def test_complete_fixture_and_receipt(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            source = root / 'fixture.zip'
            self.fixture(source)
            with patch.object(module, 'SHA256', hashlib.sha256(source.read_bytes()).hexdigest()):
                receipt = module.prepare(source, root / 'new')
            self.assertEqual(set(receipt['files']), module.REQUIRED)
            self.assertTrue((root / 'new/SOURCE_PROVENANCE.json').is_file())

    def test_paths_symlinks_duplicates_missing_and_expansion_limits(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            for index, name in enumerate(['../outside', module.ROOT + '/../../outside',
                                          '/absolute', module.ROOT + '/C:bad',
                                          module.ROOT + '/nested/file', module.ROOT + '/SCRCPY.EXE']):
                source = root / f'bad{index}.zip'
                self.fixture(source, (name, b'SYNTHETIC'))
                with zipfile.ZipFile(source) as bundle:
                    with self.assertRaises(ValueError):
                        module.checked_members(bundle)
            link = zipfile.ZipInfo(module.ROOT + '/link')
            link.create_system = 3
            link.external_attr = (stat.S_IFLNK | 0o777) << 16
            source = root / 'symlink.zip'
            self.fixture(source, (link, b'outside'))
            with zipfile.ZipFile(source) as bundle:
                with self.assertRaises(ValueError):
                    module.checked_members(bundle)
            source = root / 'incomplete.zip'
            with zipfile.ZipFile(source, 'w') as archive:
                archive.writestr(module.ROOT + '/scrcpy.exe', b'SYNTHETIC')
            with zipfile.ZipFile(source) as bundle:
                with self.assertRaises(ValueError):
                    module.checked_members(bundle)
            self.fixture(root / 'bounded.zip')
            with patch.object(module, 'MAX_EXPANDED', 1), zipfile.ZipFile(root / 'bounded.zip') as bundle:
                with self.assertRaises(ValueError):
                    module.checked_members(bundle)
