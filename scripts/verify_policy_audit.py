#!/usr/bin/env python3
"""Verify exact Ed25519 event bytes without Qt, the case database or signing keys.

Requires cryptography (Ed25519 support). Trust starts with an independently retained
anchor, not the public key bundled inside the export. Local account roles and UTC
values are declarations; this verifier establishes neither agency identity nor time.
"""
import argparse
import base64
import hashlib
import json
import os
import stat
import re
from pathlib import Path

from cryptography.exceptions import InvalidSignature
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PublicKey

DOMAIN = b"NEXVARY-MR/policy-audit/v1\n"
ACTOR_DOMAIN = b"NEXVARY-MR/local-attestation/v1\0"
SCHEMA = "nexvary-policy-audit/v1"
CAPABILITIES = {"cloud-publish", "tracking-portal", "signature-update", "device-tools", "external-analysis", "external-links"}
CLOSED = {"revoked", "rejected", "expired"}


def regular_bytes(path, limit):
    if path.is_symlink():
        raise ValueError("Symbolic links are not accepted")
    flags = os.O_RDONLY | getattr(os, "O_BINARY", 0) | getattr(os, "O_NOFOLLOW", 0) | getattr(os, "O_NONBLOCK", 0)
    fd = os.open(path, flags)
    with os.fdopen(fd, "rb") as stream:
        info = os.fstat(stream.fileno())
        if not stat.S_ISREG(info.st_mode) or not 0 < info.st_size <= limit:
            raise ValueError("Expected a nonempty bounded regular file")
        data = stream.read(limit + 1)
        if len(data) != info.st_size or len(data) > limit:
            raise ValueError("File changed size or exceeds limit")
        return data


def unique(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError("Duplicate JSON field")
        result[key] = value
    return result


def decode(data):
    value = json.loads(data, object_pairs_hook=unique,
                       parse_constant=lambda _: (_ for _ in ()).throw(ValueError("Nonfinite JSON")))
    if not isinstance(value, dict):
        raise ValueError("Expected JSON object")
    return value


def un64(value):
    if not isinstance(value, str):
        raise ValueError("Expected base64 string")
    return base64.b64decode(value, validate=True)


def signed(message, signature, key):
    Ed25519PublicKey.from_public_bytes(un64(key)).verify(un64(signature), message)


def uuid(value):
    return isinstance(value, str) and re.fullmatch(r"[0-9a-fA-F]{8}(?:-[0-9a-fA-F]{4}){3}-[0-9a-fA-F]{12}", value) and value != "00000000-0000-0000-0000-000000000000"


def verify(data, anchor):
    if len(data) > 16 * 1024 * 1024:
        raise ValueError("Audit size limit")
    doc = decode(data)
    key = anchor["publicKey"]
    floor = anchor["sequence"]
    if (doc.get("schema") != SCHEMA or not uuid(doc.get("journalId"))
            or doc.get("journalId") != anchor.get("journalId") or doc.get("publicKey") != key
            or type(floor) is not int or floor < 0 or len(un64(key)) != 32):
        raise ValueError("Pinned journal identity/key/sequence rejected")
    rows = doc["events"]
    if not isinstance(rows, list) or not floor <= len(rows) <= 8192:
        raise ValueError("Retained anchor is newer than history or event limit exceeded")
    previous = "0" * 64
    if floor == 0 and anchor["hash"] != previous:
        raise ValueError("Empty history anchor mismatch")
    states, actions, principals = {}, {}, {}
    for sequence, event in enumerate(rows, 1):
        raw, sig = un64(event["payload"]), un64(event["signature"])
        if not 0 < len(raw) <= 32 * 1024:
            raise ValueError("Event size limit")
        signed(DOMAIN + raw, event["signature"], key)
        digest = hashlib.sha256(raw + sig).hexdigest()
        p = decode(raw)
        if (p.pop("sequence") != sequence or p.pop("previous") != previous
                or p.pop("journalId") != doc["journalId"] or event["hash"] != digest):
            raise ValueError("Event order/link/digest rejected")
        proof = p.pop("actorProof")
        actor, role, action, r = p["principalId"], p["principalRole"], p["action"], p["record"]
        if proof:
            message = un64(proof["message"])
            if not message.startswith(ACTOR_DOMAIN):
                raise ValueError("Actor signature domain rejected")
            signed(message, proof["signature"], proof["publicKey"])
            a = decode(message[len(ACTOR_DOMAIN):])
            if (a.get("schema") != 1 or a.get("purpose") != "government-policy-event"
                    or a.get("payload") != p or a.get("signerId") != actor
                    or a.get("role") != role or a.get("publicKey") != proof["publicKey"]
                    or not uuid(actor)):
                raise ValueError("Actor binding rejected")
            if actor in principals and principals[actor] != proof["publicKey"]:
                raise ValueError("Unreviewed principal key replacement")
            principals[actor] = proof["publicKey"]
        elif actor or role != "system-observation":
            raise ValueError("Missing actor attestation")
        request_id = r.get("id")
        old = states.get(request_id, {})
        status = old.get("status")
        if action == "request":
            if (not uuid(request_id) or old or actor != r.get("requester")
                    or role not in {"administrator", "investigator", "repair-technician"}
                    or r.get("status") != "requested" or r.get("capability") not in CAPABILITIES
                    or r.get("requesterRole") != role or type(r.get("durationMs")) is not int
                    or not 1000 <= r["durationMs"] <= 600000
                    or r.get("scope") != "category-and-requester; not destination containment"
                    or (r.get("caseRef") and not uuid(r["caseRef"]))):
                raise ValueError("Request authority/scope rejected")
            states[request_id] = r
        elif action in {"review", "approve", "reject", "activate", "revoke", "expire", "session-close", "restart-close", "emergency-stop"}:
            mutable = {"status"}
            if action == "review":
                mutable |= {"reviewer", "reviewReason"}
            elif action in {"approve", "reject"}:
                mutable.add("decisionReason")
            elif action == "activate":
                mutable |= {"activatedUtc", "expiresUtc"}
            if action in {"reject", "revoke", "expire", "session-close", "restart-close", "emergency-stop"}:
                mutable.add("closedUtc")
            if not old or {k: v for k, v in old.items() if k not in mutable} != {k: v for k, v in r.items() if k not in mutable}:
                raise ValueError("Request identity/scope changed")
            if action == "review" and (status != "requested" or actor == old["requester"] or role not in {"administrator", "reviewer"} or r.get("reviewer") != actor or r["status"] != "in-review"):
                raise ValueError("Separate review rejected")
            if action in {"approve", "reject"} and (status != "in-review" or actor != old.get("reviewer") or actor == old["requester"] or role not in {"administrator", "reviewer"} or r.get("reviewer") != old.get("reviewer") or r["status"] != ("approved" if action == "approve" else "rejected")):
                raise ValueError("Decision authority/state rejected")
            if action == "activate" and (status != "approved" or actor != old["requester"] or r.get("reviewer") != old.get("reviewer") or r["status"] != "active"):
                raise ValueError("Requester activation rejected")
            if action in {"revoke", "expire", "session-close", "restart-close", "emergency-stop"} and (status in CLOSED or r["status"] != ("expired" if action == "expire" else "revoked")):
                raise ValueError("Closure state rejected")
            if action == "revoke" and role not in {"system-observation", "administrator"} and actor not in {old["requester"], old.get("reviewer")}:
                raise ValueError("Revocation authority rejected")
            states[request_id] = r
        elif action not in {"operation-allowed", "operation-denied", "browser-result", "audit-verification", "policy-rejection"}:
            raise ValueError("Unknown event action")
        if action in {"operation-allowed", "browser-result"} and (not old or r != old or status != "active" or role != "system-observation"):
            raise ValueError("Operation gate has no matching active exception")
        if action in {"expire", "session-close", "restart-close", "emergency-stop"} and role != "system-observation":
            raise ValueError("System closure attribution rejected")
        if action == "audit-verification" and role not in {"administrator", "reviewer", "auditor"}:
            raise ValueError("Audit verifier authority rejected")
        actions[action] = actions.get(action, 0) + 1
        previous = digest
        if sequence == floor and previous != anchor["hash"]:
            raise ValueError("Retained history prefix mismatch")
    return {"valid": True, "events": len(rows), "requests": len(states), "actions": actions,
            "head": previous, "retainedPrefixVerified": floor,
            "identityScope": "local keys/declared roles; not agency identity or independent people",
            "timeScope": "recorded local UTC; no external trusted time",
            "operationScope": "gate decisions; operation completion only where explicitly recorded"}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("audit", type=Path)
    parser.add_argument("--anchor", type=Path, required=True, help="independently retained JSON anchor")
    args = parser.parse_args()
    try:
        result = verify(regular_bytes(args.audit, 16 * 1024 * 1024),
                        decode(regular_bytes(args.anchor, 4096)))
    except (ValueError, KeyError, TypeError, OSError, InvalidSignature, RecursionError) as error:
        print(json.dumps({"valid": False, "error": type(error).__name__ + ": " + str(error)}))
        return 1
    print(json.dumps(result, ensure_ascii=False, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
