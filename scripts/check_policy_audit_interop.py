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
        body = {"action": action, "record": r, "result": r["recoveryState"] if action == "case-operation" else r["status"], "timeUtc": stamp,
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
        def partition(rows, ids):
            rows[5][1]["kind"] = rows[6][1]["kind"] = "partition-analysis"
            return rows
        protocol("independent partition result accepted by both verifiers", partition, expected=True)
        def missing_partition_source(rows, ids):
            partition(rows, ids)
            rows[6][1]["sourceSha256"] = ""
            return rows
        protocol("completed partition result requires a source digest", missing_partition_source)
        def wrong_partition_capability(rows, ids):
            partition(rows, ids)
            rows[5][1]["capability"] = "signature-update"
            return rows
        protocol("partition operation cannot use another capability", wrong_partition_capability)
        def changed_partition_kind(rows, ids):
            partition(rows, ids)
            rows[6][1]["kind"] = "security-scan"
            return rows
        protocol("partition terminal result cannot change worker kind", changed_partition_kind)
        def cancelled_partition(rows, ids):
            partition(rows, ids)
            rows[6][1].update(status="cancelled", failureClass="operator-cancelled", sourceSha256="", outputSha256="")
            return rows
        protocol("cancelled partition without invented digests is accepted", cancelled_partition, expected=True)
        def case_transaction(rows, ids):
            r = {"schema": "nexvary-case-operation/v1", "operationId": "00000000-0000-4000-8000-000000000100", "requestId": "00000000-0000-4000-8000-000000000101", "caseId": "00000000-0000-4000-8000-000000000102", "evidenceId": "00000000-0000-4000-8000-000000000103", "expectedRevision": 3, "sourceSha256": "a" * 64, "caseBeforeSha256": "b" * 64, "reportSha256": "c" * 64, "analysisId": "00000000-0000-4000-8000-000000000104", "kind": "classic-pcap", "preparedUtc": "2026-10-27T06:00:00.000Z", "requester": ids[0], "requesterRole": "investigator", "recoveryState": "prepared"}
            return rows + [("case-operation", r, 0, "investigator"), ("case-operation", {**r, "recoveryState": "completed"}, None, "system-observation")]
        protocol("independent signed case preparation and reconciliation", case_transaction, expected=True)
        for adapter in ("workbench-export-manifest", "chromium-bookmarks", "dc-image-partitions", "browser-sqlite-family", "chromium-history", "chromium-downloads"):
            def migrated(rows, ids, adapter=adapter):
                rows = case_transaction(rows, ids)
                for row in rows[-2:]:
                    row[1]["kind"] = adapter
                return rows
            protocol("migrated local case adapter " + adapter, migrated, expected=True)
        def destructive_case(rows, ids):
            rows = case_transaction(rows, ids)
            for row in rows[-2:]:
                row[1]["kind"] = "dc-format"
            return rows
        protocol("destructive adapter cannot enter case journal", destructive_case)
        def case_change(field, value):
            def change(rows, ids):
                rows = case_transaction(rows, ids)
                rows[-1][1][field] = value
                return rows
            return change
        protocol("case outcome cannot change evidence binding", case_change("evidenceId", "00000000-0000-4000-8000-000000000105"))
        protocol("case outcome cannot change report digest", case_change("reportSha256", "d" * 64))
        protocol("case intent cannot contain private paths", case_change("sourcePath", "private-fixture"))
        def missing_prepared(rows, ids):
            rows = case_transaction(rows, ids)
            return rows[:-2] + rows[-1:]
        protocol("case result without durable preparation rejected", missing_prepared)
        def duplicate_terminal(rows, ids):
            rows = case_transaction(rows, ids)
            return rows + [copy.deepcopy(rows[-1])]
        protocol("duplicate case recovery event rejected", duplicate_terminal)
        def false_case_actor(rows, ids):
            rows = case_transaction(rows, ids)
            rows[-2] = ("case-operation", rows[-2][1], 1, "reviewer")
            return rows
        protocol("case preparation cannot forge requester identity", false_case_actor)
        def bad_revision(rows, ids):
            rows = case_transaction(rows, ids)
            rows[-2][1]["expectedRevision"] = True
            return rows
        protocol("case expected revision cannot be boolean", bad_revision)
        def reused_case_request(rows, ids):
            rows = case_transaction(rows, ids)
            r = {**rows[-2][1], "operationId": "00000000-0000-4000-8000-000000000106", "analysisId": "00000000-0000-4000-8000-000000000107"}
            return rows + [("case-operation", r, 0, "investigator")]
        protocol("signed case invocation reference cannot be reused", reused_case_request)
        def reused_case_analysis(rows, ids):
            rows = case_transaction(rows, ids)
            r = {**rows[-2][1], "operationId": "00000000-0000-4000-8000-000000000106", "requestId": "00000000-0000-4000-8000-000000000107"}
            return rows + [("case-operation", r, 0, "investigator")]
        protocol("signed case analysis UUID cannot be reused", reused_case_analysis)
        def brace_uuid(rows, ids):
            rows = case_transaction(rows, ids)
            rows[-2][1]["operationId"] = "{" + rows[-2][1]["operationId"] + "}"
            return rows
        protocol("case UUIDs must use canonical spelling", brace_uuid)

        def case_partition(rows, ids):
            for i in (0, 1, 2, 3, 4, 7):
                rows[i][1]["requesterRole"] = "investigator"
                if rows[i][2] == 0:
                    rows[i] = (*rows[i][:3], "investigator")
            for i in (5, 6):
                rows[i][1].update(kind="partition-analysis", requesterRole="investigator")
                if rows[i][2] == 0:
                    rows[i] = (*rows[i][:3], "investigator")
            start = rows[5][1]
            intent = {"schema": "nexvary-case-operation/v2", "operationId": start["operationId"], "requestId": start["requestId"], "caseId": start["caseRef"], "evidenceId": "00000000-0000-4000-8000-000000000103", "expectedRevision": 3, "sourceSha256": "b" * 64, "caseBeforeSha256": "d" * 64, "reportSha256": "", "analysisId": "00000000-0000-4000-8000-000000000104", "kind": "partition-analysis", "preparedUtc": start["startedUtc"], "requester": ids[0], "requesterRole": "investigator", "recoveryState": "prepared", "toolSha256": "c" * 64}
            return rows[:6] + [("case-operation", intent, 0, "investigator"), ("case-operation", {**intent, "recoveryState": "candidate", "reportSha256": "a" * 64}, None, "system-observation"), ("case-operation", {**intent, "recoveryState": "completed", "reportSha256": "a" * 64}, None, "system-observation")] + rows[6:]
        protocol("registered partition prepared/candidate/completed original approval", case_partition, expected=True)
        def partition_change(index, field, value):
            def change(rows, ids):
                rows = case_partition(rows, ids)
                rows[index][1][field] = value
                return rows
            return change
        for index, field, value in [(6,"caseId","00000000-0000-4000-8000-000000000099"), (6,"requestId","00000000-0000-4000-8000-000000000099"), (6,"preparedUtc","2026-10-25T00:00:00.000Z"), (6,"toolSha256",""), (6,"reportSha256","a"*64), (7,"toolSha256","e"*64), (7,"sourceSha256","e"*64), (7,"evidenceId","00000000-0000-4000-8000-000000000099"), (8,"reportSha256","e"*64), (8,"caseBeforeSha256","e"*64), (9,"outputSha256","e"*64), (9,"sourceSha256","e"*64)]:
            protocol("registered partition rejects altered " + str(index) + ":" + field, partition_change(index, field, value))
        def omit_partition_stage(index):
            def change(rows, ids):
                rows = case_partition(rows, ids)
                del rows[index]
                return rows
            return change
        protocol("registered partition cannot prepare without original operation start", omit_partition_stage(5))
        protocol("registered partition cannot complete without durable candidate", omit_partition_stage(7))
        def duplicate_candidate(rows, ids):
            rows = case_partition(rows, ids)
            return rows[:8] + [copy.deepcopy(rows[7])] + rows[8:]
        protocol("registered partition cannot append a second candidate", duplicate_candidate)
        def revoked_candidate(rows, ids):
            rows = case_partition(rows, ids)
            return rows[:7] + [rows[-1]] + rows[7:-1]
        protocol("registered partition candidate after revocation rejected", revoked_candidate)
        def unknown_external(rows, ids):
            rows = case_partition(rows, ids)
            intent = {**rows[6][1], "recoveryState": "unknown"}
            return rows[:7] + [rows[-1], ("case-operation", intent, None, "system-observation")]
        protocol("registered partition interrupted external outcome stays unknown", unknown_external, expected=True)

    report = {"schema": "nexvary-policy-audit-interop/v1", "checks": results, "passed": len(results), "scope": "Internally authored synthetic interoperability corpus, not independent institutional review"}
    Path("policy-audit-interop-results.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report))


if __name__ == "__main__":
    main()
