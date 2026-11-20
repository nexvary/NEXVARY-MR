import unittest
from unittest.mock import patch, Mock
from nexvary_mr.device_actions import connected_devices, execute

class DeviceActionsTests(unittest.TestCase):
    @patch("nexvary_mr.device_actions.tool_path", return_value="adb")
    @patch("nexvary_mr.device_actions.subprocess.run")
    def test_only_authorized_adb_devices(self, run, tool):
        run.return_value = Mock(returncode=0, stdout="List of devices attached\nABC123 device\nXYZ unauthorized\n", stderr="")
        self.assertEqual(connected_devices("adb"), ["ABC123"])

    def test_unknown_operation_rejected(self):
        with self.assertRaises(ValueError):
            execute("frp_bypass", "ABC123", True)

    def test_invalid_serial_rejected(self):
        with self.assertRaises(ValueError):
            execute("read_info", "ABC;echo unsafe", True)

    @patch("nexvary_mr.device_actions.connected_devices", return_value=["ABC123"])
    def test_reboot_needs_confirmation(self, devices):
        with self.assertRaises(PermissionError):
            execute("reboot_normal", "ABC123")

    @patch("nexvary_mr.device_actions.tool_path", return_value="adb")
    @patch("nexvary_mr.device_actions.connected_devices", return_value=["ABC123"])
    @patch("nexvary_mr.device_actions.subprocess.run")
    def test_read_info_targets_exact_serial(self, run, devices, tool):
        run.return_value = Mock(returncode=0, stdout="Galaxy Test", stderr="")
        self.assertEqual(execute("read_info", "ABC123"), "Galaxy Test")
        self.assertEqual(run.call_args.args[0], ["adb", "-s", "ABC123", "shell", "getprop", "ro.product.model"])

if __name__ == "__main__":
    unittest.main()
