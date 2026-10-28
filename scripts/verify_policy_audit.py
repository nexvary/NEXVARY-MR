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



def case_intent_record(r, old, actor, role, result):
    keys = {"schema", "operationId", "requestId", "caseId", "evidenceId", "expectedRevision", "sourceSha256", "caseBeforeSha256", "reportSha256", "analysisId", "kind", "preparedUtc", "requester", "requesterRole", "recoveryState"}
    external = r.get("schema") == "nexvary-case-operation/v2"
    if external:
        keys.add("toolSha256")
    state = r.get("recoveryState")
    digests = {"sourceSha256", "caseBeforeSha256"}
    if external:
        digests.add("toolSha256")
    if not external or state in {"candidate", "completed"} or r.get("reportSha256") != "":
        digests.add("reportSha256")
    if (set(r) != keys or (not external and r["schema"] != "nexvary-case-operation/v1")
            or any(not uuid(r[k]) or r[k] != r[k].lower() for k in ("operationId", "requestId", "caseId", "evidenceId", "analysisId", "requester"))
            or any(not isinstance(r[k], str) or not re.fullmatch(r"[0-9a-f]{64}", r[k]) for k in digests)
            or (external and state == "prepared" and r["reportSha256"] != "")
            or type(r["expectedRevision"]) is not int or not 1 <= r["expectedRevision"] < 10000
            or r["kind"] not in ({"partition-analysis"} if external else {"classic-pcap", "jpeg-exif", "workbench-export-manifest", "chromium-bookmarks", "dc-image-partitions", "browser-sqlite-family", "chromium-history"})
            or r["requesterRole"] not in {"reviewer", "investigator"}
            or state not in ({"prepared", "candidate", "completed", "failed", "unknown"} if external else {"prepared", "completed", "failed", "unknown"})
            or not isinstance(r["preparedUtc"], str) or not r["preparedUtc"].endswith("Z")
            or result != state):
        raise ValueError("Case operation intent fields rejected")
    from datetime import datetime
    datetime.fromisoformat(r["preparedUtc"].replace("Z", "+00:00"))
    if result == "prepared":
        if old or actor != r["requester"] or role != r["requesterRole"]:
            raise ValueError("Case operation preparation attribution rejected")
        return
    if (old.get("recoveryState") not in ({"prepared", "candidate"} if external else {"prepared"})
            or actor or role != "system-observation"
            or (external and result == "completed" and old.get("recoveryState") != "candidate")
            or (result == "candidate" and old.get("recoveryState") != "prepared")):
        raise ValueError("Case operation recovery lifecycle rejected")
    mutable = {"recoveryState", "reportSha256"} if external and result == "candidate" else {"recoveryState"}
    if {k: v for k, v in r.items() if k not in mutable} != {k: v for k, v in old.items() if k not in mutable}:
        raise ValueError("Case operation recovery binding rejected")


def case_intent_authority(r, operation, request, closure):
    if r["schema"] != "nexvary-case-operation/v2" or r["recoveryState"] not in {"prepared", "candidate"}:
        return
    from datetime import datetime
    if (closure or request.get("status") != "active" or operation.get("status") != "started"
            or operation.get("kind") != "partition-analysis" or operation.get("capability") != "external-analysis"
            or operation.get("operationId") != r["operationId"] or operation.get("requestId") != r["requestId"]
            or request.get("id") != r["requestId"] or operation.get("requester") != r["requester"]
            or operation.get("requesterRole") != r["requesterRole"] or operation.get("caseRef") != r["caseId"]
            or request.get("caseRef") != r["caseId"]
            or datetime.fromisoformat(r["preparedUtc"]) < datetime.fromisoformat(operation["startedUtc"])):
        raise ValueError("Case operation original approval/start rejected")


def operation_record(r, request, decision, old, closure, actor, role, time, result, start):
    keys = {"capability", "caseRef", "completedUtc", "decisionHash", "decisionSequence", "failureClass", "kind", "operationId", "outputSha256", "requestId", "requester", "requesterRole", "schema", "sourceSha256", "startedUtc", "status"}
    digest = lambda x: isinstance(x, str) and (x == "" or re.fullmatch(r"[0-9a-f]{64}", x))
    if (set(r) != keys or r["schema"] != "nexvary-operation-result/v1" or not uuid(r["operationId"])
            or not request or r["requestId"] != request["id"] or r["requester"] != request["requester"]
            or r["requesterRole"] != request["requesterRole"] or r["caseRef"] != request["caseRef"]
            or r["capability"] != request["capability"] or not decision or type(r["decisionSequence"]) not in {int, float}
            or r["decisionSequence"] != decision["sequence"] or r["decisionHash"] != decision["hash"]
            or not digest(r["sourceSha256"]) or not digest(r["outputSha256"]) or result != r["status"]
            or (r["kind"], r["capability"]) not in {("security-scan", "external-analysis"), ("partition-analysis", "external-analysis"), ("signature-update", "signature-update")}):
        raise ValueError("Operation schema/approval/scope/digest rejected")
    from datetime import datetime
    def utc(value):
        return isinstance(value, str) and value.endswith("Z") and datetime.fromisoformat(value).utcoffset().total_seconds() == 0
    if not utc(r["startedUtc"]):
        raise ValueError("Invalid operation start time")
    if start:
        valid = (not old and request["status"] == "active" and actor == r["requester"] and role == r["requesterRole"]
                 and r["status"] == "started" and r["startedUtc"] == time and r["completedUtc"] == r["failureClass"] == r["outputSha256"] == "")
    else:
        mutable = {"status", "completedUtc", "failureClass", "sourceSha256", "outputSha256"}
        valid = (old.get("status") == "started" and role == "system-observation" and not actor
                 and r["completedUtc"] == time and utc(time)
                 and {k:v for k,v in old.items() if k not in mutable} == {k:v for k,v in r.items() if k not in mutable}
                 and (not old["sourceSha256"] or old["sourceSha256"] == r["sourceSha256"]))
        expected = ("failed", "process-interrupted") if r["failureClass"] == "process-interrupted" else None
        expected = expected or {"expire": ("expired", "permission-expired"), "session-close": ("cancelled", "session-ended")}.get(closure)
        if not expected and (closure == "restart-close" or r["failureClass"] == "process-interrupted"):
            expected = ("failed", "process-interrupted")
        if not expected and closure:
            expected = ("revoked", "permission-revoked")
        if expected:
            valid = valid and (r["status"], r["failureClass"]) == expected
        else:
            valid = valid and request["status"] == "active" and ((r["status"],r["failureClass"]) in {("failed","engine-failure"),("cancelled","operator-cancelled")} or (r["status"] == "completed" and r["failureClass"] == "none" and r["outputSha256"] and (r["kind"] == "signature-update" or r["sourceSha256"])))
    if not valid:
        raise ValueError("Operation lifecycle/attribution rejected")


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
    decisions, operations, closures = {}, {}, {}
    case_operations = {}
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
            if action == "approve":
                decisions[request_id] = {"sequence": sequence, "hash": digest}
            if action in {"expire", "session-close", "restart-close", "emergency-stop", "revoke"}:
                closures[request_id] = action
        elif action == "case-operation":
            if r.get("recoveryState") == "prepared" and (
                    sum(v["recoveryState"] in {"prepared", "candidate"} for v in case_operations.values()) >= 64
                    or any((r.get("schema") == "nexvary-case-operation/v1" and v["requestId"] == r.get("requestId")) or v["analysisId"] == r.get("analysisId") for v in case_operations.values())):
                raise ValueError("Case operation invocation/analysis UUID reused or unfinished capacity exceeded")
            case_intent_record(r, case_operations.get(r.get("operationId"), {}), actor, role, p["result"])
            case_intent_authority(r, operations.get(r["operationId"], {}), states.get(r["requestId"], {}), closures.get(r["requestId"], ""))
            case_operations[r["operationId"]] = r
        elif action in {"operation-start", "operation-result"}:
            request_id = r.get("requestId")
            operation_record(r, states.get(request_id, {}), decisions.get(request_id, {}), operations.get(r.get("operationId"), {}), closures.get(request_id, ""), actor, role, p["timeUtc"], p["result"], action == "operation-start")
            case = case_operations.get(r["operationId"], {})
            if case.get("schema") == "nexvary-case-operation/v2" and r["status"] == "completed" and (case["recoveryState"] != "completed" or r["sourceSha256"] != case["sourceSha256"] or r["outputSha256"] != case["reportSha256"]):
                raise ValueError("Case operation receipt digest/outcome mismatch")
            operations[r["operationId"]] = r
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
            "caseOperations": {k: sum(v["recoveryState"] == k for v in case_operations.values()) for k in {v["recoveryState"] for v in case_operations.values()}},
            "head": previous, "retainedPrefixVerified": floor,
            "identityScope": "local keys/declared roles; not agency identity or independent people",
            "timeScope": "recorded local UTC; no external trusted time",
            "operations": {k: sum(v["status"] == k for v in operations.values()) for k in {v["status"] for v in operations.values()}},
            "operationScope": "signed native worker receipts for integrated kinds; no external OS/tool truth or universal completion proof"}


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
