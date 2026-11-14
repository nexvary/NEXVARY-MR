import json
import re
import unittest
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]

class DeviceStatusLocalizationTests(unittest.TestCase):
    def test_all_literal_operator_statuses_have_both_languages(self):
        text = (ROOT / 'qml/StatusText.js').read_text(encoding='utf-8')
        messages = json.loads(text.split('var messages=', 1)[1].split(';\nvar aliases=', 1)[0])
        aliases = json.loads(text.split('var aliases=', 1)[1].split(';\nvar prefixes=', 1)[0])
        prefixes = json.loads(text.split('var prefixes=', 1)[1].split(';\nfunction translate', 1)[0])
        covered = {value for pair in messages for value in pair} | set(aliases)
        for arabic, english in messages:
            self.assertRegex(arabic, r'[\u0600-\u06ff]')
            self.assertFalse(re.search(r'[\u0600-\u06ff]', english))
        for name in ('ToolBridge.cpp', 'DeviceServices.cpp', 'MirrorSession.cpp'):
            source = (ROOT / 'cpp/src' / name).read_text(encoding='utf-8')
            for literal in re.findall(r'(?:setStatus|message)\(\s*(?:QStringLiteral\()?"([^"\n]*)"', source):
                if literal.startswith('فشل الفحص (رمز '):
                    continue
                self.assertTrue(literal in covered or any(literal.startswith(p) for pair in prefixes for p in pair), (name, literal))
