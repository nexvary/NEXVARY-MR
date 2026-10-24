import json
import unittest
from datetime import datetime, timezone
from nexvary_mr.usb_studio_import import parse_report, UsbStudioImportError

class ImportTests(unittest.TestCase):
    def test_report(self):
        data = dict(schema_version='1', tool_version='0.10.4', collection_time_utc='2026-10-24T11:00:00Z', device_scope='selected-device', evidence_status='unknown', limitations=[], findings=[])
        now = datetime(2026, 10, 24, 12, tzinfo=timezone.utc)
        self.assertEqual(parse_report(json.dumps(data).encode(), now=now)['schema_version'], '1')
        data['schema_version'] = '2'
        with self.assertRaises(UsbStudioImportError):
            parse_report(json.dumps(data).encode(), now=now)

if __name__ == '__main__':
    unittest.main()
