import importlib.util
import json
import io
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location(
    "ui_diagnostics", Path(__file__).parents[1] / "scripts/diagnose_ui_failure.py")
diagnostics = importlib.util.module_from_spec(spec)
spec.loader.exec_module(diagnostics)


class UiFailureDiagnosticsTests(unittest.TestCase):
    def test_success_does_not_replay_stale_failures(self):
        with tempfile.TemporaryDirectory() as root:
            with patch.object(diagnostics.subprocess, "run", return_value=subprocess.CompletedProcess([], 0)) as run:
                self.assertEqual(diagnostics.run_suite(root), 0)
                self.assertEqual(run.call_count, 1)

    def test_passing_diagnostic_cannot_clear_original_failure(self):
        with tempfile.TemporaryDirectory() as root:
            build = Path(root)
            logs = build / "Testing/Temporary"
            logs.mkdir(parents=True)
            (logs / "LastTest.log").write_text("original segmentation fault\n")
            (logs / "LastTestsFailed.log").write_text("46:mr_ui_scale_1.25\n8:mr_policy_audit_tests\n")
            tests = {"tests": [{"name": "mr_ui_scale_1.25", "command": ["/synthetic/mr_ui_tests", "WorkstationNavigation::test_governmentAdbBackupControls", "-o", "/original/ui-results.txt,txt"],
                                "properties": [{"name": "ENVIRONMENT", "value": ["QT_SCALE_FACTOR=1.25", "QT_QUICK_BACKEND=software"]}]}]}
            responses = [subprocess.CompletedProcess([], 8),
                         subprocess.CompletedProcess([], 0, stdout=json.dumps(tests))]
            with patch.object(diagnostics.subprocess, "run", side_effect=responses) as run, patch.object(diagnostics, "replay", return_value=0) as replay:
                self.assertEqual(diagnostics.run_suite(build), 8)
                self.assertEqual(run.call_count, 2)
                command = replay.call_args.args[0]
                self.assertIn("--return-child-result", command)
                self.assertIn("thread apply all bt", command)
                self.assertNotIn("bt full", command)
                self.assertNotIn("/original/ui-results.txt,txt", command)
                self.assertEqual(command[-2:], ["-o", "-,txt"])
                self.assertEqual(replay.call_args.args[2]["QT_SCALE_FACTOR"], "1.25")
            self.assertEqual((build / "full-suite-last-test.log").read_text(), "original segmentation fault\n")
            receipt = json.loads((build / "ui-crash-diagnostics.json").read_text())
            self.assertEqual(len(receipt), 1)
            self.assertFalse(receipt[0]["clearsOriginalFailure"])

    def test_timeout_kills_only_the_owned_group(self):
        output = io.StringIO()
        # This helper runs only in Linux CI. Simulate its POSIX APIs explicitly
        # so the same timeout regression also executes in Windows Python CI.
        with patch.object(diagnostics.subprocess, "Popen") as popen, \
                patch.object(diagnostics.os, "killpg", create=True) as kill, \
                patch.object(diagnostics.signal, "SIGKILL", 9, create=True):
            child = popen.return_value
            child.pid = 4242
            child.wait.side_effect = [subprocess.TimeoutExpired("synthetic-gdb", 180), -9]
            self.assertEqual(diagnostics.replay(["synthetic-gdb"], "/tmp", {}, output), "timeout")
            self.assertTrue(popen.call_args.kwargs["start_new_session"])
            kill.assert_called_once_with(4242, 9)
            self.assertEqual(child.wait.call_count, 2)

    def test_unavailable_diagnostic_cannot_replace_failure(self):
        with tempfile.TemporaryDirectory() as root:
            logs = Path(root) / "Testing/Temporary"
            logs.mkdir(parents=True)
            (logs / "LastTestsFailed.log").write_text("46:mr_ui_scale_1.25\n")
            with patch.object(diagnostics.subprocess, "run", side_effect=[subprocess.CompletedProcess([], 8), FileNotFoundError()]):
                self.assertEqual(diagnostics.run_suite(root), 8)
            self.assertEqual((Path(root) / "ui-crash-diagnostic-error.txt").read_text(), "FileNotFoundError\n")


if __name__ == "__main__":
    unittest.main()
