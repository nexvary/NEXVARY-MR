import unittest
from nexvary_mr.oem_services import (
    OEM, COMMON, CHIPSETS, RESTRICTED, Risk, capability_status, services_for,
    SUPPORTED_LANGUAGES,
)

class OEMServicesTest(unittest.TestCase):
    def test_six_oems(self):
        self.assertEqual(set(OEM), {"Samsung", "LG", "Huawei", "Oppo", "Vivo", "ZTE"})

    def test_qualcomm_is_chipset(self):
        self.assertIn("Qualcomm", CHIPSETS)
        self.assertNotIn("Qualcomm", OEM)

    def test_all_oems_have_common_actions(self):
        for brand in OEM:
            self.assertEqual(services_for(brand), COMMON)

    def test_restricted_actions(self):
        for action in RESTRICTED:
            self.assertEqual(capability_status("Samsung", action), "official_recovery_only")

    def test_destructive_actions_require_confirmation(self):
        for action in COMMON:
            if action.risk == Risk.DATA_LOSS:
                self.assertTrue(action.requires_confirmation)

    def test_language_codes(self):
        self.assertEqual(SUPPORTED_LANGUAGES, ("ar", "en", "fr", "tr"))

if __name__ == "__main__":
    unittest.main()
