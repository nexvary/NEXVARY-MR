from contextlib import closing
import tempfile
import unittest
from pathlib import Path
from nexvary_mr.core import CATALOG, RULES, init_db, save_case, readiness

class CoreTests(unittest.TestCase):
    def test_catalog_and_rules(self):
        self.assertIn("Samsung", CATALOG)
        self.assertIn("Bootloop", RULES)

    def test_readiness(self):
        self.assertIn("python", readiness())

    def test_case_persistence(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "repair.db"
            case_id = save_case("Samsung", "A52", "Bootloop", "test", path)
            with closing(init_db(path)) as connection:
                row = connection.execute("SELECT brand,model,symptom FROM cases WHERE id=?", (case_id,)).fetchone()
            self.assertEqual(row, ("Samsung", "A52", "Bootloop"))

    def test_required_fields(self):
        with self.assertRaises(ValueError):
            save_case("", "A52", "Bootloop")
if __name__ == "__main__":
    unittest.main()
