import copy
import json
import tempfile
import unittest
from pathlib import Path
from scripts.validate_research_catalog import ROOT, validate


class ResearchCatalogTests(unittest.TestCase):
    def test_checked_in_catalog(self):
        self.assertEqual(validate(), [])

    def test_hostile_and_overclaiming_data(self):
        original = json.loads((ROOT / 'research/program/catalog.json').read_text(encoding='utf-8'))
        changes = [
            lambda d: d['atlas'][0].update(status='Lab Verified'),
            lambda d: d['tools'][0].update(sourceIds=['absent']),
            lambda d: d['sources'][0].update(url='file:///etc/passwd'),
            lambda d: d['sources'][0].update(rights=''),
            lambda d: d['sources'][0].update(reviewedOn='2026-02-30'),
            lambda d: d['sources'][0].update(url='https://user:secret@example.com/'),
            lambda d: d['experiments'][0].update(methodology=''),
            lambda d: d['sources'].append(d['sources'][0]),
            lambda d: d['tools'][1].update(id=d['tools'][0]['id']),
            lambda d: d.update(compatibility=[{'status': 'Lab Verified'}]),
            lambda d: d.update(atlas='bad'),
            lambda d: d['tools'].__setitem__(0, 'bad'),
        ]
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / 'research/program/catalog.json'
            path.parent.mkdir(parents=True)
            for change in changes:
                data = copy.deepcopy(original)
                change(data)
                path.write_text(json.dumps(data), encoding='utf-8')
                with self.subTest(change=change):
                    self.assertTrue(validate(temp))


if __name__ == '__main__':
    unittest.main()
