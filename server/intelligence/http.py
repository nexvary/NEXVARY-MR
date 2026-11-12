"""WSGI transport for the Intelligence service.

Mount behind an HTTPS authentication gateway that supplies WSGI REMOTE_USER
from verified credentials. The application ignores HTTP identity headers.
"""

import json

from .billing import BillingError, Outcome


class IntelligenceAPI:
    def __init__(self, billing):
        self.billing = billing

    def __call__(self, environ, start_response):
        try:
            actor = environ.get("REMOTE_USER")
            if environ.get("wsgi.url_scheme") != "https" or not actor:
                return self._send(start_response, "401 Unauthorized", {"error": "authentication-required"})
            route = environ["REQUEST_METHOD"], environ["PATH_INFO"]
            if route == ("GET", "/v1/price"):
                result = self.billing.public_price()
            elif route == ("GET", "/v1/catalog"):
                result = {"cards": self.billing.catalog()}
            elif route == ("POST", "/v1/quote"):
                body = self._body(environ)
                result = self.billing.offer(actor, *self._search(body))
            elif route == ("POST", "/v1/confirm"):
                body = self._body(environ)
                result = self.billing.confirm(actor, *self._search(body), self._text(body, "quote_id"),
                                              self._text(body, "idempotency_key"), body.get("confirmed"))
            elif route[0] == "GET" and route[1].startswith("/v1/receipts/"):
                result = self.billing.receipt(actor, route[1].removeprefix("/v1/receipts/"))
            elif route == ("PUT", "/v1/admin/price"):
                body = self._body(environ)
                self.billing.configure_price(actor, body.get("price_cents"), body.get("cost_limit_cents"))
                result = self.billing.public_price()
            elif route == ("POST", "/v1/admin/credit"):
                body = self._body(environ)
                self.billing.grant_credit(actor, self._text(body, "subject"), body.get("amount_cents"),
                                          self._text(body, "reference"))
                result = {"accepted": True}
            elif route == ("POST", "/v1/admin/reconcile"):
                body = self._body(environ)
                self.billing.reconcile(actor, self._text(body, "operation_id"),
                                       Outcome(self._text(body, "state"), body.get("actual_provider_cents")))
                result = {"accepted": True}
            else:
                return self._send(start_response, "404 Not Found", {"error": "unknown-route"})
            return self._send(start_response, "200 OK", result)
        except BillingError as exc:
            denied = exc.code in ("administrator-required", "search-authorization-required")
            return self._send(start_response, "403 Forbidden" if denied else "409 Conflict", {"error": exc.code})
        except (KeyError, TypeError, ValueError, UnicodeError):
            return self._send(start_response, "400 Bad Request", {"error": "invalid-request"})

    @staticmethod
    def _send(start_response, status, body):
        payload = json.dumps(body, allow_nan=False).encode()
        start_response(status, [("Content-Type", "application/json"), ("Cache-Control", "no-store"),
                                ("Content-Length", str(len(payload)))])
        return [payload]

    @staticmethod
    def _body(environ):
        if environ.get("CONTENT_TYPE", "").split(";")[0] != "application/json":
            raise ValueError("Expected JSON")
        size = int(environ.get("CONTENT_LENGTH", "0"))
        if not 0 < size <= 40000:
            raise ValueError("Invalid body length")
        value = json.loads(environ["wsgi.input"].read(size))
        if not isinstance(value, dict):
            raise ValueError("Expected object")
        return value

    @staticmethod
    def _text(body, key):
        value = body[key]
        if not isinstance(value, str) or not 1 <= len(value) <= 256:
            raise ValueError("Invalid text field")
        return value

    @classmethod
    def _search(cls, body):
        arguments = body["arguments"]
        if not isinstance(arguments, dict):
            raise ValueError("Expected search arguments")
        return cls._text(body, "category"), arguments
