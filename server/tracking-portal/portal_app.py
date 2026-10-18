"""Single-workshop, bounded public repair-status service. No customer database upload."""
from __future__ import annotations

import hashlib
import hmac
import html
import ipaddress
import json
import os
import re
import sqlite3
import threading
import time
from collections import OrderedDict
from contextlib import contextmanager
from datetime import datetime, timezone
from pathlib import Path

MAX_BODY = 4 * 1024 * 1024
MAX_RECORDS = 2000
STATES = {"Received": "تم الاستلام", "Diagnosing": "التشخيص", "Awaiting parts": "انتظار القطع",
          "Repairing": "قيد الإصلاح", "Ready": "جاهز للاستلام", "Delivered": "تم التسليم", "Cancelled": "ملغي"}
HEX64 = re.compile(r"[a-f0-9]{64}\Z")
HEX32 = re.compile(r"[a-f0-9]{32}\Z")
HEADERS = [("Cache-Control", "no-store"), ("Referrer-Policy", "no-referrer"),
           ("X-Content-Type-Options", "nosniff"), ("X-Robots-Tag", "noindex, nofollow"),
           ("Content-Security-Policy", "default-src 'none'; style-src 'unsafe-inline'; frame-ancestors 'none'; base-uri 'none'; form-action 'none'")]


class Invalid(ValueError):
    pass


def utc(value):
    return datetime.fromtimestamp(value, timezone.utc).isoformat(timespec="seconds").replace("+00:00", "Z")


def timestamp(value):
    if not isinstance(value, str) or len(value) > 40 or not value.endswith("Z"):
        raise Invalid("UTC timestamp required")
    try:
        parsed = datetime.fromisoformat(value[:-1] + "+00:00")
        return int(parsed.timestamp())
    except (ValueError, OverflowError):
        raise Invalid("Invalid UTC timestamp") from None


def text(value, maximum, required=True):
    if not isinstance(value, str) or len(value) > maximum or "\0" in value or (required and not value.strip()):
        raise Invalid("Invalid text field")
    return value


def exact_keys(value, keys):
    if not isinstance(value, dict) or set(value) != set(keys):
        raise Invalid("Unexpected fields")


def validate_record(record, now):
    exact_keys(record, ("id", "tokenHash", "expiresAtUtc", "updatedAtUtc", "brand", "model", "status", "message", "history"))
    if type(record["id"]) is not int or not 1 <= record["id"] <= 2147483647:
        raise Invalid("Invalid order reference")
    if not isinstance(record["tokenHash"], str) or not HEX64.fullmatch(record["tokenHash"]):
        raise Invalid("Invalid token digest")
    expiry = timestamp(record["expiresAtUtc"])
    if not now < expiry <= now + 91 * 86400:
        raise Invalid("Invalid expiry")
    if timestamp(record["updatedAtUtc"]) > now + 300:
        raise Invalid("Order time is in the future")
    text(record["brand"], 240)
    text(record["model"], 240)
    text(record["message"], 1000, required=False)
    if record["status"] not in STATES:
        raise Invalid("Invalid status")
    history = record["history"]
    if not isinstance(history, list) or len(history) > 100:
        raise Invalid("History limit exceeded")
    previous = 0
    for event in history:
        exact_keys(event, ("status", "atUtc"))
        at = timestamp(event["atUtc"])
        if event["status"] not in STATES or at < previous or at > now + 300:
            raise Invalid("Invalid history")
        previous = at
    return expiry


@contextmanager
def connect(path):
    db = sqlite3.connect(path, timeout=5)
    db.execute("PRAGMA foreign_keys=ON")
    db.execute("PRAGMA trusted_schema=OFF")
    try:
        with db:
            yield db
    finally:
        db.close()


def initialize(path, source_id):
    Path(path).parent.mkdir(parents=True, exist_ok=True)
    with connect(path) as db:
        version = db.execute("PRAGMA user_version").fetchone()[0]
        if version not in (0, 1):
            raise RuntimeError("Unsupported portal database version")
        db.execute("PRAGMA journal_mode=WAL")
        db.executescript("""
        CREATE TABLE IF NOT EXISTS state(id INTEGER PRIMARY KEY CHECK(id=1),source_id TEXT NOT NULL,revision INTEGER NOT NULL,synced_at INTEGER NOT NULL);
        CREATE TABLE IF NOT EXISTS entries(token_hash TEXT PRIMARY KEY,expires_at INTEGER NOT NULL,payload TEXT NOT NULL);
        PRAGMA user_version=1;
        """)
        db.execute("INSERT OR IGNORE INTO state VALUES(1,?,0,0)", (source_id,))
        if db.execute("SELECT source_id FROM state WHERE id=1").fetchone()[0] != source_id:
            raise RuntimeError("Publisher identity does not match database")
    os.chmod(path, 0o600)


class Limiter:
    """One Gunicorn worker with bounded per-IP sliding-window counters."""
    def __init__(self):
        self.items = OrderedDict()
        self.lock = threading.Lock()

    def allow(self, ip, lane, now):
        key = (ip, lane)
        limit = 120 if lane == "public" else 60
        with self.lock:
            start, count = self.items.pop(key, (now, 0))
            if now - start >= 60:
                start, count = now, 0
            self.items[key] = (start, count + 1)
            while len(self.items) > 4096:
                self.items.popitem(last=False)
            return count < limit


def page(snapshot=None, synced=0):
    esc = lambda value: html.escape(str(value), quote=True)
    if snapshot is None:
        content = "<h1>NEXVARY MR</h1><p>الرابط غير متاح أو انتهت صلاحيته.</p><p>Tracking link unavailable or expired.</p>"
    else:
        status = snapshot["status"]
        content = (f"<h1>NEXVARY MR — متابعة الصيانة</h1><p>أمر الصيانة / Repair order #{snapshot['id']}</p>"
                   f"<p>{esc(snapshot['brand'])} {esc(snapshot['model'])}</p>"
                   f"<h2>{STATES[status]} / {status}</h2><p>{esc(snapshot['message'])}</p>"
                   f"<p class='muted'>آخر مزامنة من المركز / Last workshop sync:<br><bdi>{utc(synced)}</bdi></p><ol>")
        for event in snapshot["history"]:
            content += f"<li>{STATES[event['status']]} / {event['status']}<br><bdi>{esc(event['atUtc'])}</bdi></li>"
        content += "</ol><p class='muted'>تتحدث الصفحة كل 30 ثانية. احتفظ بالرابط لنفسك.<br>Refreshes every 30 seconds. Keep your link private.</p>"
    return ("<!doctype html><html lang='ar' dir='rtl'><head><meta charset='utf-8'>"
            "<meta name='viewport' content='width=device-width,initial-scale=1'><meta http-equiv='refresh' content='30'>"
            "<title>NEXVARY — متابعة الصيانة</title><style>body{margin:0;padding:24px;background:#0a0a0a;color:#eee;font:17px system-ui}"
            "main{max-width:680px;margin:auto;padding:28px;border:1px solid #555;border-radius:18px;background:linear-gradient(155deg,#343434,#111 25%,#080808)}"
            "h1{font-size:25px}h2{color:#64dbb0}.muted{color:#bbb;font-size:14px}p,li{white-space:pre-wrap;overflow-wrap:anywhere}li{padding:10px}</style>"
            f"</head><body><main>{content}</main></body></html>").encode()


class Portal:
    def __init__(self, config, clock=time.time):
        self.db = str(config["database"])
        self.domain = config["domain"].lower()
        self.port = config.get("httpsPort", 443)
        self.source_id = config["sourceId"]
        self.key_hash = config["apiKeySha256"]
        if not HEX32.fullmatch(self.source_id) or not HEX64.fullmatch(self.key_hash):
            raise ValueError("Invalid service credentials")
        self.clock = clock
        self.limiter = Limiter()
        initialize(self.db, self.source_id)

    def authorized(self, environ):
        value = environ.get("HTTP_AUTHORIZATION", "")
        key = value[7:] if value.startswith("Bearer ") else ""
        return len(key) == 64 and hmac.compare_digest(hashlib.sha256(key.encode()).hexdigest(), self.key_hash)

    def response(self, start_response, code, body, content_type="application/json; charset=utf-8"):
        if isinstance(body, dict):
            body = json.dumps(body, ensure_ascii=False, separators=(",", ":")).encode()
        statuses = {200: "OK", 400: "Bad Request", 401: "Unauthorized", 404: "Not Found", 405: "Method Not Allowed",
                    409: "Conflict", 413: "Content Too Large", 415: "Unsupported Media Type", 429: "Too Many Requests", 503: "Service Unavailable"}
        start_response(f"{code} {statuses[code]}", HEADERS + [("Content-Type", content_type), ("Content-Length", str(len(body)))])
        return [body]

    def __call__(self, environ, start_response):
        # No environment, request path, Authorization header or customer payload is logged.
        try:
            return self.dispatch(environ, start_response)
        except (Invalid, ValueError, TypeError, KeyError, UnicodeError, RecursionError):
            return self.response(start_response, 400, {"error": "Invalid request"})
        except Exception:
            return self.response(start_response, 503, {"error": "Service unavailable"})

    def dispatch(self, env, reply):
        method = env.get("REQUEST_METHOD", "")
        path = env.get("PATH_INFO", "")
        now = int(self.clock())
        if path == "/healthz" and method == "GET":
            with connect(self.db) as db:
                db.execute("SELECT revision FROM state WHERE id=1").fetchone()
            return self.response(reply, 200, {"ok": True})
        if env.get("HTTP_HOST", "").lower() not in ((self.domain if self.port == 443 else self.domain + ":" + str(self.port)), self.domain + ":" + str(self.port)) or env.get("wsgi.url_scheme") != "https":
            return self.response(reply, 400, {"error": "Invalid origin"})
        ip = env.get("REMOTE_ADDR", "unknown")
        if ip in ("127.0.0.1", "::1"):
            candidate = env.get("HTTP_X_NEXVARY_CLIENT_IP", ip)
            try:
                ip = str(ipaddress.ip_address(candidate))
            except ValueError:
                pass
        lane = "api" if path.startswith("/api/") else "public"
        if not self.limiter.allow(ip, lane, now):
            return self.response(reply, 429, {"error": "Retry later"})
        if env.get("QUERY_STRING"):
            return self.response(reply, 404, {"error": "Not found"})
        if path == "/robots.txt" and method == "GET":
            return self.response(reply, 200, b"User-agent: *\nDisallow: /\n", "text/plain; charset=utf-8")
        if path == "/api/v1/state":
            if not self.authorized(env):
                return self.response(reply, 401, {"error": "Unauthorized"})
            if method != "GET":
                return self.response(reply, 405, {"error": "Method not allowed"})
            with connect(self.db) as db:
                source, revision, _ = db.execute("SELECT source_id,revision,synced_at FROM state WHERE id=1").fetchone()
            return self.response(reply, 200, {"schemaVersion": 1, "sourceId": source, "revision": revision})
        if path == "/api/v1/snapshot":
            if not self.authorized(env):
                return self.response(reply, 401, {"error": "Unauthorized"})
            if method != "POST":
                return self.response(reply, 405, {"error": "Method not allowed"})
            if env.get("CONTENT_TYPE", "").split(";", 1)[0].strip() != "application/json":
                return self.response(reply, 415, {"error": "JSON required"})
            length = env.get("CONTENT_LENGTH", "")
            if not length.isdecimal() or not 0 < int(length) <= MAX_BODY:
                return self.response(reply, 413, {"error": "Body limit exceeded"})
            data = env["wsgi.input"].read(int(length))
            if len(data) != int(length):
                raise Invalid("Incomplete body")
            # Reject duplicate JSON keys instead of accepting ambiguous data.
            def pairs(items):
                out = {}
                for key, value in items:
                    if key in out:
                        raise Invalid("Duplicate JSON key")
                    out[key] = value
                return out
            doc = json.loads(data, object_pairs_hook=pairs)
            exact_keys(doc, ("schemaVersion", "sourceId", "expectedRevision", "records"))
            if type(doc["schemaVersion"]) is not int or doc["schemaVersion"] != 1 or type(doc["expectedRevision"]) is not int or not 0 <= doc["expectedRevision"] < 2**53:
                raise Invalid("Invalid revision")
            if doc["sourceId"] != self.source_id:
                return self.response(reply, 409, {"error": "Publisher identity mismatch"})
            records = doc["records"]
            if not isinstance(records, list) or len(records) > MAX_RECORDS:
                raise Invalid("Record limit exceeded")
            seen_hashes, seen_ids, prepared = set(), set(), []
            for record in records:
                expiry = validate_record(record, now)
                if record["tokenHash"] in seen_hashes or record["id"] in seen_ids:
                    raise Invalid("Duplicate record")
                seen_hashes.add(record["tokenHash"])
                seen_ids.add(record["id"])
                prepared.append((record["tokenHash"], expiry, json.dumps(record, ensure_ascii=False, separators=(",", ":"))))
            with connect(self.db) as db:
                db.execute("BEGIN IMMEDIATE")
                revision = db.execute("SELECT revision FROM state WHERE id=1").fetchone()[0]
                if revision != doc["expectedRevision"]:
                    return self.response(reply, 409, {"error": "Revision changed; fetch state and rebuild snapshot"})
                db.execute("DELETE FROM entries")
                db.executemany("INSERT INTO entries VALUES(?,?,?)", prepared)
                db.execute("UPDATE state SET revision=revision+1,synced_at=? WHERE id=1", (now,))
            return self.response(reply, 200, {"revision": revision + 1, "count": len(prepared), "syncedAtUtc": utc(now)})
        match = re.fullmatch(r"/t/([a-f0-9]{64})", path)
        if match and method == "GET":
            token_hash = hashlib.sha256(match[1].encode()).hexdigest()
            with connect(self.db) as db:
                synced = db.execute("SELECT synced_at FROM state WHERE id=1").fetchone()[0]
                row = db.execute("SELECT expires_at,payload FROM entries WHERE token_hash=?", (token_hash,)).fetchone()
            # Status becomes unavailable after six hours without successful workstation sync.
            if row and row[0] > now and 0 <= now - synced < 6 * 3600:
                return self.response(reply, 200, page(json.loads(row[1]), synced), "text/html; charset=utf-8")
        return self.response(reply, 404, page(), "text/html; charset=utf-8")


def create_app(config=None):
    if config is None:
        config = json.loads(Path(os.environ["NEXVARY_PORTAL_CONFIG"]).read_text())
    return Portal(config)
