import importlib.util
import sys
import unittest
from pathlib import Path

root = Path(__file__).parent
sys.path.insert(0, str(root))
spec = importlib.util.spec_from_file_location("assessment_cli", root / "assessment_cli.py")
cli = importlib.util.module_from_spec(spec)
spec.loader.exec_module(cli)


class AssessmentCliTests(unittest.TestCase):
    def test_authorized_card(self):
        output = cli.evaluate({"source": "external_card", "factory_reset": True,
                               "authorized": True, "readable": True})
        self.assertEqual(output["schema"], 1)
        self.assertEqual(output["verdict"], "acquire_copy")

    def test_no_consent(self):
        output = cli.evaluate({"source": "accessible_backup", "factory_reset": False,
                               "authorized": False})
        self.assertEqual(output["verdict"], "not_supported")

    def test_reject_unknown_field(self):
        with self.assertRaises(ValueError):
            cli.evaluate({"source": "cloud_export", "factory_reset": True,
                          "authorized": True, "command": "anything"})

    def test_reject_string_boolean(self):
        with self.assertRaises(ValueError):
            cli.evaluate({"source": "external_card", "factory_reset": "false",
                          "authorized": True})

    def test_reject_invalid_source(self):
        with self.assertRaises(ValueError):
            cli.evaluate({"source": "root_bypass", "factory_reset": False,
                          "authorized": True})


if __name__ == "__main__":
    unittest.main()
