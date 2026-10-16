import importlib.util
import unittest
from pathlib import Path

spec = importlib.util.spec_from_file_location("recovery_assessment", Path(__file__).with_name("recovery_assessment.py"))
assessment = importlib.util.module_from_spec(spec)
import sys
sys.modules[spec.name] = assessment
spec.loader.exec_module(assessment)


class AssessmentTests(unittest.TestCase):
    def test_denied_authorization(self):
        result = assessment.assess(assessment.Source.EXTERNAL_CARD, factory_reset=False, authorized=False, readable=True)
        self.assertEqual(result.verdict, assessment.Verdict.NOT_SUPPORTED)

    def test_reset_internal_storage(self):
        result = assessment.assess(assessment.Source.INTERNAL_STORAGE, factory_reset=True, authorized=True)
        self.assertEqual(result.verdict, assessment.Verdict.NOT_SUPPORTED)
        self.assertIn("Shizuku", " ".join(result.limitations))

    def test_external_card(self):
        source = assessment.Source.EXTERNAL_CARD
        self.assertEqual(assessment.assess(source, factory_reset=True, authorized=True).verdict,
                         assessment.Verdict.NEED_MORE_INFORMATION)
        self.assertEqual(assessment.assess(source, factory_reset=True, authorized=True, readable=True).verdict,
                         assessment.Verdict.ACQUIRE_COPY)
        self.assertEqual(assessment.assess(source, factory_reset=True, authorized=True, readable=False).verdict,
                         assessment.Verdict.NOT_SUPPORTED)

    def test_backups(self):
        self.assertEqual(assessment.assess(assessment.Source.ACCESSIBLE_BACKUP, factory_reset=True, authorized=True).verdict,
                         assessment.Verdict.VERIFY_BACKUP)
        self.assertEqual(assessment.assess(assessment.Source.CLOUD_EXPORT, factory_reset=True, authorized=True).verdict,
                         assessment.Verdict.REQUEST_EXPORT)


if __name__ == "__main__":
    unittest.main()
