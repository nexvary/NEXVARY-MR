"""Offline billing integration tests. The provider is a synthetic fixture."""
import io
import json
import sqlite3
import tempfile
import threading
import unittest
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

from server.intelligence.billing import Billing, BillingError, Capability, Cost, Outcome
from server.intelligence.http import IntelligenceAPI


class SyntheticProvider:
    def __init__(self):
        self.cost = Cost(800, 150)
        self.outcome = Outcome("completed", 795)
        self.calls = []
        self.release = threading.Event()
        self.release.set()
        self.entered = threading.Event()

    def capabilities(self):
        return [Capability(category, category, category) for category in ("fixture", "second-fixture", "disabled")]

    def available(self, category):
        return category in ("fixture", "second-fixture")

    def estimate(self, category, arguments):
        return self.cost

    def execute(self, category, arguments, operation_id, ceiling):
        self.calls.append((operation_id, ceiling))
        self.entered.set()
        if not self.release.wait(5):
            raise RuntimeError("Fixture dispatch blocked")
        if isinstance(self.outcome, Exception):
            raise self.outcome
        return self.outcome


class IntelligenceBillingTests(unittest.TestCase):
    actor = "case-user"
    args = {"indicator": "test.invalid"}

    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.path = Path(self.temp.name) / "ledger.sqlite"
        self.provider = SyntheticProvider()
        self.now = 1000
        self.billing = Billing(self.path, self.provider,
                               lambda actor, category, args: actor == self.actor,
                               lambda actor: actor == "admin",
                               clock=lambda: self.now)
        self.billing.grant_credit("admin", self.actor, 4200, "payment-1")

    def balance(self):
        with sqlite3.connect(self.path) as db:
            return db.execute("SELECT balance FROM intelligence_balances").fetchone()[0]

    def quote(self):
        return self.billing.offer(self.actor, "fixture", self.args)

    def confirm(self, quote, key="request-key-00000001", args=None, confirmed=True):
        return self.billing.confirm(self.actor, "fixture", self.args if args is None else args,
                                    quote["quote_id"], key, confirmed)

    def test_unified_price_repricing_and_historical_receipt(self):
        quote = self.quote()
        self.assertEqual(quote["currency"], "EUR")
        self.assertEqual(quote["price_cents"], 1400)
        self.assertNotIn("provider_cost", quote)
        old = self.confirm(quote)
        self.assertEqual(self.balance(), 2800)
        stale = self.quote()
        with self.assertRaisesRegex(BillingError, "administrator-required"):
            self.billing.configure_price(self.actor, 1500, 1200)
        self.billing.configure_price("admin", 1500, 1200)
        with self.assertRaisesRegex(BillingError, "quote-changed"):
            self.confirm(stale, "request-key-00000002")
        self.assertEqual(self.quote()["price_cents"], 1500)
        self.assertEqual(self.billing.receipt(self.actor, old["id"])["price_cents"], 1400)
        with sqlite3.connect(self.path) as db:
            actor, ts = db.execute("SELECT actor,at_epoch FROM intelligence_billing_audit WHERE action='price-updated'").fetchone()
        self.assertEqual((actor, ts), ("admin", 1000))

    def test_duplicate_concurrent_confirmation_executes_once(self):
        quote = self.quote()
        self.provider.release.clear()
        try:
            with ThreadPoolExecutor(max_workers=2) as pool:
                first = pool.submit(self.confirm, quote)
                try:
                    self.assertTrue(self.provider.entered.wait(3))
                    duplicate = pool.submit(self.confirm, quote).result(timeout=3)
                    self.assertEqual(duplicate["state"], "pending")
                    self.assertEqual(self.balance(), 2800)
                    with self.assertRaisesRegex(BillingError, "quote-already-used"):
                        self.confirm(quote, "request-key-00000002")
                finally:
                    self.provider.release.set()
                final = first.result(timeout=3)
        finally:
            self.provider.release.set()
        self.assertEqual(final["id"], duplicate["id"])
        self.assertEqual(final["state"], "completed")
        self.assertEqual(self.provider.calls, [(final["id"], 800)])

    def test_failure_refunds_once_and_ambiguous_outcome_waits(self):
        self.provider.outcome = Outcome("failed", 12)
        quote = self.quote()
        failed = self.confirm(quote)
        self.assertTrue(failed["credit_restored"])
        self.assertEqual(self.confirm(quote), failed)
        self.assertEqual(self.balance(), 4200)
        self.provider.outcome = TimeoutError()
        quote = self.quote()
        pending = self.confirm(quote, "request-key-00000002")
        self.assertEqual(pending["state"], "pending")
        self.assertEqual(self.balance(), 2800)
        self.assertEqual(self.confirm(quote, "request-key-00000002"), pending)
        with self.assertRaisesRegex(BillingError, "administrator-required"):
            self.billing.reconcile(self.actor, pending["id"], Outcome("failed", 0))
        self.billing.reconcile("admin", pending["id"], Outcome("failed", 0))
        self.billing.reconcile("admin", pending["id"], Outcome("failed", 0))
        self.assertEqual(self.balance(), 4200)
        self.assertEqual(len(self.provider.calls), 2)

    def test_availability_cost_and_quote_boundaries(self):
        with self.assertRaisesRegex(BillingError, "search-unavailable"):
            self.billing.offer(self.actor, "disabled", self.args)
        self.provider.cost = None
        with self.assertRaisesRegex(BillingError, "cost-unverified"):
            self.quote()
        self.provider.cost = Cost(1300, 101)
        with self.assertRaisesRegex(BillingError, "cost-limit-exceeded"):
            self.quote()
        self.provider.cost = Cost(800, 50)
        quote = self.quote()
        with self.assertRaisesRegex(BillingError, "confirmation-required"):
            self.confirm(quote, confirmed=False)
        with self.assertRaisesRegex(BillingError, "quote-invalid-or-expired"):
            self.confirm(quote, args={"indicator": "other.invalid"})
        self.now += 300
        with self.assertRaisesRegex(BillingError, "quote-invalid-or-expired"):
            self.confirm(quote)
        self.assertEqual(self.provider.calls, [])
        self.assertEqual(self.balance(), 4200)

    def test_insufficient_balance_and_idempotent_topup(self):
        self.billing.grant_credit("admin", self.actor, 4200, "payment-1")
        self.assertEqual(self.balance(), 4200)
        for index in range(3):
            self.confirm(self.quote(), f"request-key-0000000{index}")
        with self.assertRaisesRegex(BillingError, "insufficient-balance"):
            self.confirm(self.quote(), "request-key-00000009")
        self.assertEqual(self.balance(), 0)
        self.assertEqual(len(self.provider.calls), 3)

    def test_provider_overrun_is_audited_without_extra_charge(self):
        self.provider.outcome = Outcome("completed", 1251)
        receipt = self.confirm(self.quote())
        self.assertEqual(receipt["price_cents"], 1400)
        self.assertEqual(self.balance(), 2800)
        with sqlite3.connect(self.path) as db:
            events = {name: json.loads(details) for name, details in db.execute(
                "SELECT action,details FROM intelligence_billing_audit")}
        self.assertEqual(events["provider-cost-overrun"]["actual"], 1251)
        self.assertEqual(events["search-settled"]["margin"], -1)
        self.assertNotIn("margin", receipt)

    def test_https_boundary_and_public_receipt_shape(self):
        api = IntelligenceAPI(self.billing)
        def request(method, path, actor=None, body=None, scheme="https"):
            data = json.dumps(body or {}).encode()
            environ = {"REQUEST_METHOD": method, "PATH_INFO": path, "wsgi.url_scheme": scheme,
                       "HTTP_REMOTE_USER": "admin", "CONTENT_LENGTH": str(len(data)),
                       "CONTENT_TYPE": "application/json", "wsgi.input": io.BytesIO(data)}
            if actor:
                environ["REMOTE_USER"] = actor
            status = []
            reply = api(environ, lambda code, headers: status.append(code))
            return status[0], json.loads(b"".join(reply))

        self.assertTrue(request("GET", "/v1/price")[0].startswith("401"))
        self.assertTrue(request("GET", "/v1/price", self.actor, scheme="http")[0].startswith("401"))
        self.assertTrue(request("PUT", "/v1/admin/price", self.actor, {})[0].startswith("403"))
        self.assertEqual(request("GET", "/v1/price", self.actor)[1], {"price_cents": 1400, "currency": "EUR"})
        cards = request("GET", "/v1/catalog", self.actor)[1]["cards"]
        self.assertEqual([row["price_cents"] for row in cards if row["available"]], [1400, 1400])
        self.assertNotIn("price_cents", cards[-1])
        payload = {"category": "fixture", "arguments": self.args}
        quote = request("POST", "/v1/quote", self.actor, payload)[1]
        payload.update(quote_id=quote["quote_id"], idempotency_key="request-key-00000001",
                       confirmed=True, price_cents=1)
        receipt = request("POST", "/v1/confirm", self.actor, payload)[1]
        self.assertEqual(receipt["price_cents"], 1400)
        self.assertEqual(set(receipt), {"id", "category", "currency", "price_cents", "state", "credit_restored"})
        self.assertTrue(request("GET", f"/v1/receipts/{receipt['id']}", "another-account")[0].startswith("409"))


if __name__ == "__main__":
    unittest.main()
