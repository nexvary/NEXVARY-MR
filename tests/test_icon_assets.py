import hashlib
import json
import struct
import unittest
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

class ProfessionalIconTests(unittest.TestCase):
    def test_all_active_icons_are_exact_licensed_upstream_vectors(self):
        folder = ROOT / 'assets/icons-professional'
        manifest = json.loads((folder / 'PROVENANCE.json').read_text(encoding="utf-8"))
        self.assertGreaterEqual(len(manifest['files']), 27)
        self.assertEqual(set(p.name for p in folder.glob('*.svg')), {v['file'] for v in manifest['files']})
        self.assertIn('Permission is hereby granted', (ROOT / 'packaging/licenses/FLUENT-ICONS-MIT.txt').read_text(encoding="utf-8"))
        for item in manifest['files']:
            data = (folder / item['file']).read_bytes()
            self.assertEqual(hashlib.sha256(data).hexdigest(), item['sha256'])
            self.assertEqual(data, (ROOT / 'assets/icon-sources/fluent' / item.get('raw_file', item.get('alias_of', item['file']))).read_bytes())
            node = ET.fromstring(data)
            self.assertEqual(node.tag, '{http://www.w3.org/2000/svg}svg')
            self.assertNotIn(b'<image', data)
            self.assertNotIn(b'<script', data)

    def test_brand_seals_are_self_contained_vector_artwork(self):
        for name in ('nexvary-eye-seal.svg', 'nexvary-eye-symbol.svg'):
            data = (ROOT / 'assets/branding' / name).read_bytes()
            root = ET.fromstring(data)
            self.assertEqual(root.attrib['viewBox'], '0 0 512 512')
            self.assertGreater(len(root.findall('.//{http://www.w3.org/2000/svg}path')), 3)
            for forbidden in (b'<image', b'<script', b'<text', b'<foreignObject'):
                self.assertNotIn(forbidden, data)
        self.assertIn('nexvary-eye-seal.svg', (ROOT / 'qml/TopBar.qml').read_text(encoding='utf-8'))

    def test_windows_ico_has_all_required_native_sizes(self):
        data = (ROOT / 'packaging/app.ico').read_bytes()
        reserved, kind, count = struct.unpack_from('<HHH', data)
        self.assertEqual((reserved, kind, count), (0, 1, 7))
        sizes = set()
        for i in range(count):
            w, h, colors, unused, planes, bits, length, offset = struct.unpack_from('<BBBBHHII', data, 6+16*i)
            sizes.add((w or 256, h or 256))
            self.assertGreater(length, 0)
            self.assertLessEqual(offset+length, len(data))
        self.assertEqual(sizes, {(s,s) for s in (16,24,32,48,64,128,256)})

    def test_windows_icon_is_wired_to_native_and_installer_surfaces(self):
        source = (ROOT / 'cpp/src/main.cpp').read_text(encoding="utf-8")
        cmake = (ROOT / 'CMakeLists.txt').read_text(encoding="utf-8")
        setup = (ROOT / 'packaging/NEXVARY-MR.iss').read_text(encoding="utf-8")
        self.assertIn(':/packaging/app.ico', source)
        self.assertIn('packaging/app.ico', cmake)
        self.assertIn('SetupIconFile=app.ico', setup)
        self.assertEqual(setup.count('IconIndex: 0'), 2)
        self.assertIn('OutputBaseFilename=NEXVARY-MR-Government-Setup', setup)
