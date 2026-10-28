"""Negative and positive checks for the three-source integration gate."""
import importlib.util
import json
from copy import deepcopy
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("forensic_gate", ROOT / "scripts" / "check_forensic_engine_integration.py")
gate = importlib.util.module_from_spec(spec)
spec.loader.exec_module(gate)

class ForensicIntegrationGateTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.original = json.loads((ROOT / "research" / "forensic-engine-integration.json").read_text(encoding="utf-8"))

    def test_current_manifest(self):
        gate.validate(deepcopy(self.original))

    def test_no_destructive_permission(self):
        data = deepcopy(self.original)
        data["sources"][2]["permission"] = "write"
        with self.assertRaises(ValueError):
            gate.validate(data)

    def test_no_unlicensed_enable(self):
        data = deepcopy(self.original)
        data["sources"][0].update(enabled=True, license_verified=False)
        with self.assertRaises(ValueError):
            gate.validate(data)

    def test_no_unpinned_enable(self):
        data = deepcopy(self.original)
        data["sources"][0]["enabled"] = True
        data["sources"][0]["license_verified"] = True
        data["sources"][0]["source_sha"] = None
        with self.assertRaises(ValueError):
            gate.validate(data)

    def test_pinned_licensed_read_only_candidate(self):
        data = deepcopy(self.original)
        data["sources"][0].update(enabled=True, license_verified=True, source_sha="a" * 40)
        gate.validate(data)

    def test_no_unrecorded_permission(self):
        data = deepcopy(self.original)
        data["sources"][0]["permission_record"] = "README.md"
        with self.assertRaises(ValueError):
            gate.validate(data)

    def test_no_destructive_adapter(self):
        data = deepcopy(self.original)
        data["sources"][2]["adapter"] = "dc-format"
        with self.assertRaises(ValueError):
            gate.validate(data)

    def test_gate_survives_python_optimization(self):
        import subprocess
        import sys
        code = "import runpy; g=runpy.run_path('scripts/check_forensic_engine_integration.py'); g['validate']({})"
        result = subprocess.run([sys.executable, "-O", "-c", code], cwd=ROOT, capture_output=True, text=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("unknown manifest version", result.stderr)

    def test_no_unapproved_source(self):
        data = deepcopy(self.original)
        data["sources"][0]["repository"] = "unknown/unknown"
        with self.assertRaises(ValueError):
            gate.validate(data)

if __name__ == "__main__":
    unittest.main()
