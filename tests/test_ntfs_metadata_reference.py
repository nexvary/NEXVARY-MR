"""Fixture checks independent of the C++ parser; no device access/formatter invocation."""
import base64
import hashlib
import json
from pathlib import Path
import runpy
import struct
import tempfile
import unittest
import zlib

ROOT = Path(__file__).resolve().parents[1]
FIXTURES = ROOT / 'samples/ntfs-metadata'


def image(fixture):
    packed = base64.b64decode(fixture['qCompressBase64'], validate=True)
    expected = struct.unpack('>I', packed[:4])[0]
    assert expected == 4 * 1024 * 1024
    data = zlib.decompress(packed[4:])
    assert len(data) == expected
    return data


class NtfsMetadataReferenceTests(unittest.TestCase):
    def test_synthetic_fixture_reproduction(self):
        expected = json.loads((FIXTURES / 'reference.json').read_text(encoding='utf-8'))
        namespace = runpy.run_path(str(FIXTURES / 'generate_reference.py'))
        with tempfile.TemporaryDirectory() as temp:
            namespace['main'].__globals__['__file__'] = str(Path(temp) / 'generate_reference.py')
            namespace['main']()
            actual = json.loads((Path(temp) / 'reference.json').read_text(encoding='utf-8'))
            # Compression versions may encode identical images differently.
            for key in list(actual):
                if key.lower().endswith('qcompressbase64'):
                    a = base64.b64decode(actual.pop(key), validate=True)
                    e = base64.b64decode(expected[key], validate=True)
                    self.assertEqual(a[:4], e[:4])
                    self.assertEqual(zlib.decompress(a[4:]), zlib.decompress(e[4:]))
            self.assertEqual(actual, {k: v for k, v in expected.items() if not k.lower().endswith('qcompressbase64')})
        data = image(expected)
        self.assertEqual(hashlib.sha256(data).hexdigest(), expected['imageSha256'])
        self.assertEqual(data[3:11], b'NTFS    ')
        self.assertEqual(struct.unpack_from('<H', data, 11)[0], 512)
        self.assertEqual(expected['expectedRecords'], [16, 17, 18, 19])

    def test_independent_image_and_bootloader_exclusion(self):
        fixture = json.loads((FIXTURES / 'ntfs3g-reference.json').read_text(encoding='utf-8'))
        data = image(fixture)
        self.assertEqual(hashlib.sha256(data).hexdigest(), fixture['imageSha256'])
        self.assertEqual(data[84:510], bytes(426))
        self.assertEqual(data[512:8192], bytes(7680))
        self.assertEqual(data[-428:-2], bytes(426))
        self.assertEqual(data[510:512], b'\x55\xaa')
        self.assertEqual(fixture['expectedNamedUserRecords'][-1], {'record': 64, 'name': 'known.txt'})
        self.assertEqual(fixture['knownContentSha256'], hashlib.sha256(b'abc').hexdigest())
