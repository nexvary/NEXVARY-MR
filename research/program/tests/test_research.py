import json
import tempfile
import unittest
from pathlib import Path
from tools.validate_research import ROOT, validate

class RegistryTests(unittest.TestCase):
    def test_initial_registry(self):
        self.assertEqual(validate(ROOT), [])

    def test_invalid_source_is_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            base = Path(tmp)
            for name in ('research/sources/registry.json','research/knowledge/initial-research.json','research/compatibility/matrix.json'):
                path = base / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes((ROOT / name).read_bytes())
            path = base / 'research/sources/registry.json'
            data = json.loads(path.read_text())
            data['sources'][0]['url'] = 'http://unsafe.invalid'
            path.write_text(json.dumps(data))
            self.assertTrue(any('invalid source URL' in e for e in validate(base)))

if __name__ == '__main__':
    unittest.main()
