import unittest
from scripts.verify_policy_audit import operation_record


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
