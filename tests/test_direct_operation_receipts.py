import unittest
import base64
import hashlib
import json
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey
from cryptography.hazmat.primitives import serialization
from cryptography.exceptions import InvalidSignature
from scripts.verify_policy_audit import operation_record, verify, DOMAIN


class DirectOperationReceiptTests(unittest.TestCase):
    def record(self):
        return dict(schema="nexvary-investigator-operation/v1", operationId="00000000-0000-4000-8000-000000000001",
                    requester="00000000-0000-4000-8000-000000000002", requesterRole="investigator",
                    requestId="", caseRef="", capability="external-analysis", kind="intelligence-mcp",
                    decisionSequence=0, decisionHash="", startedUtc="2026-11-13T12:00:00.000Z",
                    completedUtc="", status="started", failureClass="", sourceSha256="", outputSha256="")

    def check_start(self, record):
        operation_record(record, {}, {}, {}, "", record["requester"], record["requesterRole"],
                         record["startedUtc"], "started", True)

    def test_direct_operation_and_completed_result(self):
        start = self.record()
        self.check_start(start)
        end = dict(start, completedUtc="2026-11-13T12:00:01.000Z", status="completed", failureClass="none",
                   sourceSha256="a" * 64, outputSha256="b" * 64)
        operation_record(end, {}, {}, start, "", "", "system-observation", end["completedUtc"], "completed", False)

    def test_direct_scope_cannot_authorize_other_capabilities_or_roles(self):
        for changes in [dict(kind="partition-analysis"), dict(requesterRole="administrator"),
                        dict(capability="device-tools"), dict(decisionSequence=True), dict(decisionHash="a" * 64)]:
            with self.subTest(changes=changes), self.assertRaises(ValueError):
                self.check_start(dict(self.record(), **changes))

    def test_direct_receipt_cannot_claim_legacy_independent_approval(self):
        with self.assertRaises(ValueError):
            self.check_start(dict(self.record(), schema="nexvary-operation-result/v1"))

    def signed_gate(self, changes=None, action="operation-allowed", result="gate-allowed; operation completion not established"):
        # Independently signed historical producer bytes: no native writer or
        # live device/API is involved in this regression.
        key = Ed25519PrivateKey.generate()
        b64 = lambda data: base64.b64encode(data).decode("ascii")
        public = b64(key.public_key().public_bytes(serialization.Encoding.Raw, serialization.PublicFormat.Raw))
        journal = "00000000-0000-4000-8000-000000000001"
        record = dict(capability="device-tools", mode="windows-direct-work",
                      observedActor="00000000-0000-4000-8000-000000000002 / windows-workspace")
        record.update(changes or {})
        payload = dict(action=action, record=record, result=result, timeUtc="2026-11-14T09:00:00.000Z",
                       principalId="", principalRole="system-observation", actorProof={},
                       journalId=journal, sequence=1, previous="0" * 64)
        raw = json.dumps(payload, sort_keys=True, separators=(",", ":")).encode()
        signature = key.sign(DOMAIN + raw)
        digest = hashlib.sha256(raw + signature).hexdigest()
        export = dict(schema="nexvary-policy-audit/v1", journalId=journal, publicKey=public,
                      events=[dict(payload=b64(raw), signature=b64(signature), hash=digest)])
        anchor = dict(journalId=journal, publicKey=public, sequence=1, hash=digest)
        return json.dumps(export).encode(), anchor

    def test_historical_direct_device_observation_verifies(self):
        data, anchor = self.signed_gate()
        self.assertTrue(verify(data, anchor)["valid"])
        # Changing signed bytes still fails even when the record shape is valid.
        altered = json.loads(data)
        altered["events"][0]["signature"] = base64.b64encode(b"x" * 64).decode()
        with self.assertRaises((ValueError, InvalidSignature)):
            verify(json.dumps(altered).encode(), anchor)

    def test_direct_device_observation_does_not_accept_other_gates(self):
        for changes, action, result in [
            (dict(capability="external-analysis"), "operation-allowed", "gate-allowed; operation completion not established"),
            (dict(capability="paid-unknown-cost"), "operation-allowed", "gate-allowed; operation completion not established"),
            (dict(mode="anything"), "operation-allowed", "gate-allowed; operation completion not established"),
            (dict(observedActor="bad / windows-workspace"), "operation-allowed", "gate-allowed; operation completion not established"),
            (dict(observedActor="00000000-0000-0000-0000-000000000000 / windows-workspace"), "operation-allowed", "gate-allowed; operation completion not established"),
            (dict(approved=True), "operation-allowed", "gate-allowed; operation completion not established"),
            ({}, "browser-result", "gate-allowed; operation completion not established"),
            ({}, "operation-allowed", "completed")]:
            with self.subTest(changes=changes, action=action, result=result):
                data, anchor = self.signed_gate(changes, action, result)
                with self.assertRaises(ValueError):
                    verify(data, anchor)
