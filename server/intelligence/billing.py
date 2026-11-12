"""Server-side Intelligence pricing and prepaid settlement.

The provider adapter must enforce the quoted ceiling and execute at most one
provider request per search. Only a definitive provider failure permits a refund.
An ambiguous outcome stays pending for reconciliation.
"""

from contextlib import contextmanager
from dataclasses import dataclass
import hashlib
import json
import sqlite3
import time
from typing import Iterable, Protocol
from uuid import uuid4

DEFAULT_PRICE_CENTS = 1400
MAX_CENTS = 100_000_000
QUOTE_LIFETIME_SECONDS = 300


class BillingError(Exception):
    def __init__(self, code):
        super().__init__(code)
        self.code = code


@dataclass(frozen=True)
class Cost:
    # All values must be verified against the provider account and stated in EUR cents.
    provider_cents: int
    operating_cents: int


@dataclass(frozen=True)
class Outcome:
    state: str
    actual_provider_cents: int | None = None


@dataclass(frozen=True)
class Capability:
    category: str
    label_ar: str
    label_en: str


class Provider(Protocol):
    def capabilities(self) -> Iterable[Capability]: ...
    def available(self, category: str) -> bool: ...
    def estimate(self, category: str, arguments: dict) -> Cost | None: ...
    def execute(self, category: str, arguments: dict, operation_id: str,
                max_provider_cents: int) -> Outcome: ...


def cents(value, *, allow_zero=False):
    if type(value) is not int or not (0 if allow_zero else 1) <= value <= MAX_CENTS:
        raise BillingError("invalid-amount")
    return value


def request_digest(category, arguments):
    encoded = json.dumps([category, arguments], sort_keys=True, allow_nan=False,
                         separators=(",", ":"), ensure_ascii=False).encode()
    if len(encoded) > 32768:
        raise BillingError("invalid-request")
    return hashlib.sha256(encoded).hexdigest()


class Billing:
    def __init__(self, path, provider: Provider, may_search, is_admin, *, clock=time.time):
        self.path = path
        self.provider = provider
        self.may_search = may_search
        self.is_admin = is_admin
        self.clock = clock
        with self.transaction() as db:
            db.executescript("""
            CREATE TABLE IF NOT EXISTS intelligence_price (
                singleton INTEGER PRIMARY KEY CHECK(singleton=1),
                price INTEGER NOT NULL, cost_limit INTEGER NOT NULL);
            CREATE TABLE IF NOT EXISTS intelligence_balances (
                subject TEXT PRIMARY KEY, balance INTEGER NOT NULL CHECK(balance>=0));
            CREATE TABLE IF NOT EXISTS intelligence_quotes (
                id TEXT PRIMARY KEY, subject TEXT NOT NULL, category TEXT NOT NULL,
                digest TEXT NOT NULL, price INTEGER NOT NULL, provider_cost INTEGER NOT NULL,
                operating_cost INTEGER NOT NULL, expires INTEGER NOT NULL);
            CREATE TABLE IF NOT EXISTS intelligence_operations (
                id TEXT PRIMARY KEY, subject TEXT NOT NULL, idem_key TEXT NOT NULL,
                quote_id TEXT NOT NULL UNIQUE, category TEXT NOT NULL, digest TEXT NOT NULL,
                price INTEGER NOT NULL, operating_cost INTEGER NOT NULL,
                state TEXT NOT NULL, actual_provider_cost INTEGER,
                UNIQUE(subject, idem_key));
            CREATE TABLE IF NOT EXISTS intelligence_credit_receipts (
                reference TEXT PRIMARY KEY, subject TEXT NOT NULL, amount INTEGER NOT NULL);
            CREATE TABLE IF NOT EXISTS intelligence_billing_audit (
                sequence INTEGER PRIMARY KEY, actor TEXT NOT NULL, action TEXT NOT NULL,
                at_epoch INTEGER NOT NULL, details TEXT NOT NULL);
            """)
            db.execute("INSERT OR IGNORE INTO intelligence_price VALUES (1, ?, ?)",
                       (DEFAULT_PRICE_CENTS, DEFAULT_PRICE_CENTS))

    @contextmanager
    def transaction(self):
        db = sqlite3.connect(self.path, timeout=10, isolation_level=None)
        db.row_factory = sqlite3.Row
        try:
            db.execute("BEGIN IMMEDIATE")
            yield db
            db.commit()
        except BaseException:
            db.rollback()
            raise
        finally:
            db.close()

    def _audit(self, db, actor, action, details):
        db.execute("INSERT INTO intelligence_billing_audit(actor,action,at_epoch,details) VALUES (?,?,?,?)",
                   (actor, action, int(self.clock()), json.dumps(details, sort_keys=True)))

    def _admin(self, actor):
        if not actor or not self.is_admin(actor):
            raise BillingError("administrator-required")

    def _authorized(self, actor, category, arguments):
        if not actor or not self.may_search(actor, category, arguments):
            raise BillingError("search-authorization-required")

    @staticmethod
    def _price(db):
        return db.execute("SELECT price,cost_limit FROM intelligence_price WHERE singleton=1").fetchone()

    def public_price(self):
        with self.transaction() as db:
            return {"currency": "EUR", "price_cents": self._price(db)["price"]}

    def catalog(self):
        price = self.public_price()
        cards = []
        for capability in self.provider.capabilities():
            available = self.provider.available(capability.category)
            card = {"category": capability.category, "label_ar": capability.label_ar,
                    "label_en": capability.label_en, "available": available}
            if available:
                card.update(price)
            cards.append(card)
        return cards

    def configure_price(self, actor, price_cents, cost_limit_cents):
        self._admin(actor)
        price, limit = cents(price_cents), cents(cost_limit_cents)
        with self.transaction() as db:
            previous = dict(self._price(db))
            db.execute("UPDATE intelligence_price SET price=?,cost_limit=? WHERE singleton=1", (price, limit))
            self._audit(db, actor, "price-updated", {"previous": previous, "price": price, "cost_limit": limit})

    def grant_credit(self, actor, subject, amount, reference):
        self._admin(actor)
        amount = cents(amount)
        if not subject or not reference or len(reference) > 256:
            raise BillingError("invalid-credit-reference")
        with self.transaction() as db:
            # An external credit receipt is applied once, including on request retries.
            old = db.execute("SELECT subject,amount FROM intelligence_credit_receipts WHERE reference=?",
                             (reference,)).fetchone()
            if old:
                if tuple(old) != (subject, amount):
                    raise BillingError("credit-reference-conflict")
                return
            db.execute("INSERT INTO intelligence_credit_receipts VALUES (?,?,?)", (reference, subject, amount))
            db.execute("""INSERT INTO intelligence_balances VALUES (?,?)
                          ON CONFLICT(subject) DO UPDATE SET balance=balance+excluded.balance""",
                       (subject, amount))
            self._audit(db, actor, "credit-granted", {"subject": subject, "amount": amount, "reference": reference})

    def offer(self, actor, category, arguments):
        self._authorized(actor, category, arguments)
        if not self.provider.available(category):
            raise BillingError("search-unavailable")
        digest = request_digest(category, arguments)
        estimate = self.provider.estimate(category, arguments)
        if estimate is None:
            raise BillingError("cost-unverified")
        provider = cents(estimate.provider_cents, allow_zero=True)
        operating = cents(estimate.operating_cents, allow_zero=True)
        with self.transaction() as db:
            price, limit = self._price(db)
            if provider + operating > min(price, limit):
                raise BillingError("cost-limit-exceeded")
            quote = uuid4().hex
            expires = int(self.clock()) + QUOTE_LIFETIME_SECONDS
            db.execute("INSERT INTO intelligence_quotes VALUES (?,?,?,?,?,?,?,?)",
                       (quote, actor, category, digest, price, provider, operating, expires))
        return {"quote_id": quote, "category": category, "currency": "EUR",
                "price_cents": price, "expires_at": expires}

    @staticmethod
    def _receipt(row):
        return {"id": row["id"], "category": row["category"], "currency": "EUR",
                "price_cents": row["price"], "state": row["state"],
                "credit_restored": row["state"] == "failed"}

    def receipt(self, actor, operation):
        with self.transaction() as db:
            row = db.execute("SELECT * FROM intelligence_operations WHERE id=? AND subject=?",
                             (operation, actor)).fetchone()
            if row is None:
                raise BillingError("receipt-unavailable")
            return self._receipt(row)

    def confirm(self, actor, category, arguments, quote_id, idempotency_key, confirmed):
        if confirmed is not True or not isinstance(idempotency_key, str) or not 16 <= len(idempotency_key) <= 128:
            raise BillingError("confirmation-required")
        self._authorized(actor, category, arguments)
        digest = request_digest(category, arguments)
        # BEGIN IMMEDIATE serializes the idempotency lookup, quote use and debit.
        with self.transaction() as db:
            previous = db.execute("SELECT * FROM intelligence_operations WHERE subject=? AND idem_key=?",
                                  (actor, idempotency_key)).fetchone()
            if previous:
                if previous["quote_id"] != quote_id or previous["digest"] != digest:
                    raise BillingError("idempotency-conflict")
                return self._receipt(previous)
            quote = db.execute("SELECT * FROM intelligence_quotes WHERE id=? AND subject=?",
                               (quote_id, actor)).fetchone()
            if not quote or quote["digest"] != digest or quote["expires"] <= int(self.clock()):
                raise BillingError("quote-invalid-or-expired")
            price, limit = self._price(db)
            if quote["price"] != price or quote["provider_cost"] + quote["operating_cost"] > min(price, limit):
                raise BillingError("quote-changed")
            if not self.provider.available(category):
                raise BillingError("search-unavailable")
            if db.execute("SELECT 1 FROM intelligence_operations WHERE quote_id=?", (quote_id,)).fetchone():
                raise BillingError("quote-already-used")
            debit = db.execute("""UPDATE intelligence_balances SET balance=balance-?
                                  WHERE subject=? AND balance>=?""", (price, actor, price))
            if debit.rowcount != 1:
                raise BillingError("insufficient-balance")
            operation = uuid4().hex
            db.execute("INSERT INTO intelligence_operations VALUES (?,?,?,?,?,?,?,?,?,NULL)",
                       (operation, actor, idempotency_key, quote_id, category, digest,
                        price, quote["operating_cost"], "pending"))
            self._audit(db, actor, "search-reserved", {"operation": operation, "price": price})
        try:
            outcome = self.provider.execute(category, arguments, operation, quote["provider_cost"])
        except Exception:
            # A response could be lost after dispatch. Never replay or refund on uncertainty.
            return self.receipt(actor, operation)
        if (isinstance(outcome, Outcome) and outcome.state in ("completed", "failed")
                and type(outcome.actual_provider_cents) is int
                and 0 <= outcome.actual_provider_cents <= MAX_CENTS):
            self._settle(actor, operation, outcome)
        return self.receipt(actor, operation)

    def reconcile(self, administrator, operation, outcome):
        self._admin(administrator)
        self._settle(administrator, operation, outcome)

    def _settle(self, actor, operation, outcome):
        actual = cents(outcome.actual_provider_cents, allow_zero=True)
        if outcome.state not in ("completed", "failed"):
            raise BillingError("invalid-settlement")
        with self.transaction() as db:
            row = db.execute("SELECT * FROM intelligence_operations WHERE id=?", (operation,)).fetchone()
            if row is None:
                raise BillingError("receipt-unavailable")
            if row["state"] != "pending":
                if row["state"] != outcome.state or row["actual_provider_cost"] != actual:
                    raise BillingError("settlement-conflict")
                return
            if outcome.state == "failed":
                db.execute("UPDATE intelligence_balances SET balance=balance+? WHERE subject=?",
                           (row["price"], row["subject"]))
            db.execute("UPDATE intelligence_operations SET state=?,actual_provider_cost=? WHERE id=?",
                       (outcome.state, actual, operation))
            ceiling = db.execute("SELECT provider_cost FROM intelligence_quotes WHERE id=?",
                                 (row["quote_id"],)).fetchone()["provider_cost"]
            if actual > ceiling:
                self._audit(db, actor, "provider-cost-overrun",
                            {"operation": operation, "quoted": ceiling, "actual": actual})
            revenue = row["price"] if outcome.state == "completed" else 0
            self._audit(db, actor, "search-settled",
                        {"operation": operation, "state": outcome.state, "provider_cost": actual,
                         "operating_cost": row["operating_cost"],
                         "margin": revenue - actual - row["operating_cost"]})
