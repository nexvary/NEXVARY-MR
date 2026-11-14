import re
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

class InstitutionalInstallerTests(unittest.TestCase):
    def test_each_language_has_its_own_terms_and_description(self):
        setup = (ROOT / 'packaging/NEXVARY-MR.iss').read_text(encoding='utf-8-sig')
        self.assertIn('ShowLanguageDialog=yes', setup)
        for language in ('ar', 'en'):
            row = next(line for line in setup.splitlines() if line.startswith(f'Name: "{language}";'))
            for kind in ('LicenseFile', 'InfoBeforeFile'):
                name = re.search(kind + r': "([^"]+)"', row).group(1)
                text = (ROOT / 'packaging' / name).read_text(encoding='utf-8-sig')
                self.assertGreater(len(text), 600)
                if language == 'en':
                    self.assertFalse(re.search(r'[\u0600-\u06ff]', text))
        self.assertNotIn('program-description.rtf', setup)

    def test_unattended_install_cannot_silently_skip_authority_declarations(self):
        setup = (ROOT / 'packaging/NEXVARY-MR.iss').read_text(encoding='utf-8-sig')
        self.assertIn('AuthorityPage.Values[0] := False', setup)
        self.assertIn('AuthorityPage.Values[1] := False', setup)
        self.assertIn('if WizardSilent then', setup)
        for key in ('NEXVARY_ACCEPT_LICENSE', 'NEXVARY_AUTHORIZED_ORG', 'NEXVARY_LAWFUL_USE'):
            self.assertIn('{param:' + key + '|0}', setup)
        self.assertIn('WizardForm.LicenseAcceptedRadio.Checked', setup)

    def test_candidate_version_is_consistent_across_binary_and_interface(self):
        brand = (ROOT / 'qml/Brand.js').read_text(encoding='utf-8-sig')
        version = re.search(r'version="([^"]+)"', brand).group(1)
        for name in ('cpp/src/main.cpp', 'packaging/app.rc', 'packaging/NEXVARY-MR.iss', 'CMakeLists.txt'):
            self.assertIn(version, (ROOT / name).read_text(encoding='utf-8-sig'))
        for name in ('qml/HelpPage.qml', 'qml/SettingsPage.qml', 'qml/AppShell.qml'):
            self.assertNotRegex((ROOT / name).read_text(encoding='utf-8-sig'), r'0\.4\.[135]')
