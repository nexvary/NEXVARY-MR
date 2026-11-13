import pathlib
import re
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]

class IntelligenceBrandingTests(unittest.TestCase):
    def test_qml_has_no_provider_display_strings_or_urls(self):
        for path in (ROOT / "qml").glob("*.qml"):
            for line in path.read_text(encoding="utf-8").splitlines():
                line = re.sub(r'objectName\s*:\s*"[^"]*"', '', line)
                # Internal object names and receipt lookup keys remain compatible.
                for text in re.findall(r'"([^"\\]*(?:\\.[^"\\]*)*)"', line):
                    if text == "lampyre-mcp":
                        continue
                    self.assertNotIn("lampyre", text.lower(), str(path))

    def test_destination_is_not_a_qml_configuration_surface(self):
        panel = (ROOT / "qml/LampyreConnectionPanel.qml").read_text(encoding="utf-8")
        header = (ROOT / "cpp/src/LampyreConnection.hpp").read_text(encoding="utf-8")
        self.assertNotIn("connection.endpoint", panel)
        self.assertNotIn("id:endpoint", panel)
        self.assertNotRegex(header, r'Q_PROPERTY\(QString endpoint')
        self.assertNotIn("Q_INVOKABLE bool configure(", header)
        self.assertIn("Q_INVOKABLE bool configureCredential", header)

    def test_packaged_help_and_exports_have_no_destination_metadata(self):
        guide = (ROOT / "docs/WORKSPACE_QUICK_START_AR.md").read_text(encoding="utf-8")
        self.assertNotIn("lampyre", guide.lower())
        source = (ROOT / "cpp/src/LampyreConnection.cpp").read_text(encoding="utf-8")
        self.assertNotIn('"endpointHost"', source)
        self.assertIn('consumeSubmission()', source)
        self.assertIn('redactedDisplayCopy', source)
        self.assertIn('originalResultSha256', source)

    def test_investigator_can_reach_native_setup(self):
        workspace = (ROOT / "qml/OsintWorkspace.qml").read_text(encoding="utf-8")
        self.assertIn('active: workspace.intelligenceOperator && workspace.showAdvanced', workspace)
        self.assertNotIn('active: workspace.administrator && workspace.showAdvanced', workspace)
        self.assertIn('["administrator", "investigator", "reviewer"]', workspace)
