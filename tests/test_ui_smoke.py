"""Qt smoke tests run headlessly in CI with QT_QPA_PLATFORM=offscreen."""
import os
import unittest

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

try:
    from PySide6.QtWidgets import QApplication, QPushButton, QComboBox
    from nexvary_mr.app import Window
except ImportError:
    QApplication = None

@unittest.skipIf(QApplication is None, "PySide6 not installed in test environment")
class UiSmokeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.app = QApplication.instance() or QApplication([])

    def test_navigation_pages(self):
        window = Window()
        self.assertEqual(window.stack.count(), 6)
        buttons = window.findChildren(QPushButton)
        self.assertTrue(any("OEM Service" in b.text() for b in buttons))
        window.close()

    def test_transport_action_filter(self):
        window = Window()
        window.stack.setCurrentIndex(3)
        page = window.stack.currentWidget()
        combos = page.findChildren(QComboBox)
        transport = next(c for c in combos if c.findText("fastboot") >= 0)
        action = next(c for c in combos if c.findData("read_info") >= 0)
        transport.setCurrentText("fastboot")
        self.assertEqual(action.findData("read_info"), -1)
        self.assertGreaterEqual(action.findData("fastboot_check"), 0)
        window.close()

if __name__ == "__main__":
    unittest.main()
