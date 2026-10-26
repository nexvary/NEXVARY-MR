#!/usr/bin/env python3
"""Native producer -> independent Python verifier, including hostile histories."""
import copy
import base64
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
from cryptography.exceptions import InvalidSignature
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey
from cryptography.hazmat.primitives import serialization
from verify_policy_audit import decode, verify, DOMAIN, ACTOR_DOMAIN


def independent_history(change=None):
    # Independently authored producer with known lifecycle answers. The native
    # code does not generate these bytes, keys, actor proofs or expected states.
    keys = [Ed25519PrivateKey.generate() for _ in range(4)]
    ids = [f"00000000-0000-4000-8000-{i:012d}" for i in range(1, 4)]
    b64 = lambda value: base64.b64encode(value).decode("ascii")
    public = lambda key: b64(key.public_key().public_bytes(serialization.Encoding.Raw, serialization.PublicFormat.Raw))
    wire = lambda value: json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":")).encode()
    stamp = "2026-10-26T06:00:00.000Z"
    record = {"id": "00000000-0000-4000-8000-000000000009", "requester": ids[0],
              "requesterActor": ids[0] + " / independent-synthetic", "requesterRole": "administrator",
              "capability": "external-analysis", "scope": "category-and-requester; not destination containment",
              "caseRef": "00000000-0000-4000-8000-000000000079", "reason": "Independent synthetic known-answer corpus", "durationMs": 5000,
              "requestedUtc": stamp, "requestDeadlineUtc": "2026-10-26T06:10:00.000Z", "status": "requested"}
    operations = [("request", copy.deepcopy(record), 0, "administrator")]
    record.update(status="in-review", reviewer=ids[1], reviewReason="Independent review")
    operations.append(("review", copy.deepcopy(record), 1, "reviewer"))
    record.update(status="approved", decisionReason="Independent approval")
    operations.append(("approve", copy.deepcopy(record), 1, "reviewer"))
    record.update(status="active", activatedUtc=stamp, expiresUtc="2026-10-26T06:00:05.000Z")
    operations.append(("activate", copy.deepcopy(record), 0, "administrator"))
    operations.append(("operation-allowed", copy.deepcopy(record), None, "system-observation"))
    receipt = {"schema":"nexvary-operation-result/v1", "operationId":"00000000-0000-4000-8000-000000000042",
               "requestId":record["id"], "requester":ids[0], "requesterRole":"administrator", "caseRef":record["caseRef"],
               "capability":"external-analysis", "kind":"security-scan", "decisionSequence":3, "decisionHash":"__decision__",
               "startedUtc":stamp, "completedUtc":"", "status":"started", "failureClass":"", "sourceSha256":"", "outputSha256":""}
    operations.append(("operation-start",copy.deepcopy(receipt),0,"administrator"))
    receipt.update(status="completed", completedUtc=stamp, failureClass="none", sourceSha256="b"*64, outputSha256="a"*64)
    operations.append(("operation-result",copy.deepcopy(receipt),None,"system-observation"))
    record.update(status="revoked", closedUtc="2026-10-26T06:00:01.000Z")
    operations.append(("emergency-stop", copy.deepcopy(record), None, "system-observation"))
    if change:
        operations = change(operations, ids)
    events, previous = [], "0" * 64
    journal_id = "00000000-0000-4000-8000-000000000010"
    for sequence, (action, r, actor, role) in enumerate(operations, 1):
        if action in {"operation-start","operation-result"} and r["decisionHash"] == "__decision__":
            r = {**r, "decisionHash": events[2]["hash"]}
        body = {"action": action, "record": r, "result": r["status"], "timeUtc": stamp,
                "principalId": ids[actor] if actor is not None else "", "principalRole": role}
        proof = {}
        if actor is not None:
            attestation = {"schema": 1, "purpose": "government-policy-event", "payload": body,
                           "signerId": ids[actor], "role": role, "publicKey": public(keys[actor]), "signedAtUtc": stamp}
            message = ACTOR_DOMAIN + wire(attestation)
            proof = {"message": b64(message), "signature": b64(keys[actor].sign(message)), "publicKey": public(keys[actor])}
        payload = wire({**body, "journalId": journal_id, "sequence": sequence, "previous": previous, "actorProof": proof})
        signature = keys[3].sign(DOMAIN + payload)
        previous = hashlib.sha256(payload + signature).hexdigest()
        events.append({"payload": b64(payload), "signature": b64(signature), "hash": previous})
    return wire({"schema": "nexvary-policy-audit/v1", "journalId": journal_id, "publicKey": public(keys[3]), "events": events}), {"journalId": journal_id, "publicKey": public(keys[3]), "sequence": len(events), "hash": previous}


def main():
    results = []
    with tempfile.TemporaryDirectory(prefix="nexvary-policy-interop-") as temp:
        root = Path(temp)
        subprocess.run([sys.argv[1], "--export-fixture", str(root)], check=True, timeout=40)
        data = (root / "audit.json").read_bytes()
        anchor = decode((root / "anchor.json").read_bytes())
        doc = decode(data)

        def check(name, candidate, retained=anchor, valid=False):
            try:
                result = verify(candidate, retained)
                accepted = result["valid"]
            except (ValueError, TypeError, KeyError, InvalidSignature):
                accepted = False
            if accepted != valid:
                raise AssertionError(name)
            results.append({"check": name, "passed": True})

        check("native signatures, requester, reviewer and full retained anchor", data, valid=True)
        check("empty retained prefix with independently pinned journal key", data, {**anchor, "sequence": 0, "hash": "0" * 64}, valid=True)
        d = copy.deepcopy(doc)
        d["events"][0], d["events"][1] = d["events"][1], d["events"][0]
        check("reordered events", json.dumps(d).encode())
        d = copy.deepcopy(doc)
        d["events"].pop()
        check("valid signed history truncated below retained anchor", json.dumps(d).encode())
        d = copy.deepcopy(doc)
        d["events"].pop(1)
        check("deleted interior event", json.dumps(d).encode())
        d = copy.deepcopy(doc)
        d["events"].insert(1, d["events"][0])
        check("duplicated event", json.dumps(d).encode())
        d = copy.deepcopy(doc)
        sig = d["events"][0]["signature"]
        d["events"][0]["signature"] = ("A" if sig[0] != "A" else "B") + sig[1:]
        check("changed signature", json.dumps(d).encode())
        d = copy.deepcopy(doc)
        d["events"][0]["payload"] = "e30="
        check("changed decision payload", json.dumps(d).encode())
        d = copy.deepcopy(doc)
        d["publicKey"] = "A" * 43 + "="
        check("substituted bundled public key", json.dumps(d).encode())
        check("wrong independent key", data, {**anchor, "publicKey": "A" * 43 + "="})
        check("wrong independent digest", data, {**anchor, "hash": "f" * 64})
        check("wrong independent journal identity", data, {**anchor, "journalId": "00000000-0000-4000-8000-000000000001"})
        check("future retained sequence", data, {**anchor, "sequence": anchor["sequence"] + 1})
        check("negative retained sequence", data, {**anchor, "sequence": -1})
        check("fractional retained sequence", data, {**anchor, "sequence": 1.5})
        check("ambiguous duplicate JSON keys", data[:-1] + b',"events":[]}')
        check("over-limit input", b" " * (16 * 1024 * 1024 + 1))
        cli = subprocess.run([sys.executable, str(Path(__file__).with_name("verify_policy_audit.py")), str(root / "audit.json"), "--anchor", str(root / "anchor.json")], capture_output=True, check=True, timeout=15, text=True)
        assert json.loads(cli.stdout)["valid"]
        results.append({"check": "standalone CLI needs no Qt/database/private key", "passed": True})
    with tempfile.TemporaryDirectory(prefix="nexvary-policy-independent-") as temp:
        root = Path(temp)
        def protocol(name, change=None, expected=False):
            data, retained = independent_history(change)
            try:
                accepted = verify(data, retained)["valid"]
            except (ValueError, TypeError, KeyError, InvalidSignature):
                accepted = False
            (root / "audit.json").write_bytes(data)
            (root / "anchor.json").write_text(json.dumps(retained), encoding="utf-8")
            native = subprocess.run([sys.argv[1], "--verify-fixture", str(root)], capture_output=True, timeout=15)
            assert accepted == expected and (native.returncode == 0) == expected, name + ": " + native.stderr.decode(errors="replace")
            results.append({"check": name, "passed": True})
        protocol("independent producer accepted by both verifier implementations", expected=True)
        def self_review(rows, ids):
            rows[1] = ("review", {**rows[1][1], "reviewer": ids[0]}, 0, "administrator")
            return rows
        protocol("fully signed self-review is rejected", self_review)
        def wrong_reviewer(rows, ids):
            rows[2] = ("approve", rows[2][1], 2, "reviewer")
            return rows
        protocol("fully signed unassigned reviewer is rejected", wrong_reviewer)
        def changed_scope(rows, ids):
            rows[3][1]["capability"] = "external-links"
            return rows
        protocol("fully signed scope change is rejected", changed_scope)
        def changed_reason(rows, ids):
            rows[3][1]["reviewReason"] = "Requester overwrote reviewer reason"
            return rows
        protocol("fully signed previous review mutation is rejected", changed_reason)
        def wrong_requester(rows, ids):
            rows[3] = ("activate", rows[3][1], 1, "reviewer")
            return rows
        protocol("fully signed reviewer activation is rejected", wrong_requester)
        def premature_gate(rows, ids):
            return rows[:3] + [("operation-allowed", copy.deepcopy(rows[2][1]), None, "system-observation")]
        protocol("fully signed gate without active permission is rejected", premature_gate)
        def false_system(rows, ids):
            rows[-1] = ("emergency-stop", rows[-1][1], 0, "administrator")
            return rows
        protocol("fully signed false system attribution is rejected", false_system)
        def wrong_decision(rows, ids):
            rows[5][1]["decisionHash"] = "f"*64
            return rows
        protocol("signed operation cannot cite a different approval digest", wrong_decision)
        def wrong_sequence(rows, ids):
            rows[5][1]["decisionSequence"] = 4
            return rows
        protocol("signed operation cannot cite activation as approval", wrong_sequence)
        def wrong_identity(rows, ids):
            rows[5][1]["requester"] = ids[1]
            return rows
        protocol("signed operation requester must match approval", wrong_identity)
        def leak_path(rows, ids):
            rows[5][1]["sourcePath"] = "synthetic-sensitive-path"
            return rows
        protocol("operation receipts reject unapproved content fields", leak_path)
        def premature_result(rows, ids):
            return rows[:5] + rows[6:]
        protocol("operation result without a start is rejected", premature_result)
        def duplicate_result(rows, ids):
            return rows[:7] + [copy.deepcopy(rows[6])] + rows[7:]
        protocol("duplicate terminal operation result is rejected", duplicate_result)
        def completed_after_revoke(rows, ids):
            return rows[:6]+[rows[-1],rows[6]]
        protocol("success after authorization revocation is rejected", completed_after_revoke)
        def wrong_case(rows, ids):
            rows[6][1]["caseRef"]="00000000-0000-4000-8000-000000000080"
            return rows
        protocol("operation result cannot change the declared case reference", wrong_case)
        def bad_digest(rows, ids):
            rows[6][1]["sourceSha256"]="not-a-digest"
            return rows
        protocol("completed scan requires a well-formed source digest", bad_digest)
    report = {"schema": "nexvary-policy-audit-interop/v1", "checks": results, "passed": len(results), "scope": "Internally authored synthetic interoperability corpus, not independent institutional review"}
    Path("policy-audit-interop-results.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report))


if __name__ == "__main__":
    main()
