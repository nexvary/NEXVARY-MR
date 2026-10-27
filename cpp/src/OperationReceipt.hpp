#pragma once
#include <QDateTime>
#include <QRegularExpression>
#include <QUuid>
#include <QVariantMap>

namespace mr::policy {
inline bool digestOrEmpty(const QVariant &v) {
  return v.metaType().id() == QMetaType::QString &&
         (v.toString().isEmpty() ||
          QRegularExpression("^[0-9a-f]{64}$").match(v.toString()).hasMatch());
}
inline bool operationRecord(const QVariantMap &r, const QVariantMap &request,
                            const QVariantMap &decision, const QVariantMap &old,
                            const QString &closure, const QString &actor,
                            const QString &role, const QString &time,
                            const QString &result, bool start) {
  const QStringList keys{
      "capability",       "caseRef",      "completedUtc", "decisionHash",
      "decisionSequence", "failureClass", "kind",         "operationId",
      "outputSha256",     "requestId",    "requester",    "requesterRole",
      "schema",           "sourceSha256", "startedUtc",   "status"};
  if (r.keys() != keys || r.value("schema") != "nexvary-operation-result/v1" ||
      QUuid(r.value("operationId").toString()).isNull() || request.isEmpty() ||
      r.value("requestId") != request.value("id") ||
      r.value("requester") != request.value("requester") ||
      r.value("requesterRole") != request.value("requesterRole") ||
      r.value("caseRef") != request.value("caseRef") ||
      r.value("capability") != request.value("capability") ||
      decision.isEmpty() || r.value("decisionHash") != decision.value("hash") ||
      r.value("decisionSequence").metaType().id() == QMetaType::Bool ||
      r.value("decisionSequence").metaType().id() == QMetaType::QString ||
      r.value("decisionSequence").toDouble() !=
          decision.value("sequence").toDouble() ||
      !digestOrEmpty(r.value("sourceSha256")) ||
      !digestOrEmpty(r.value("outputSha256")))
    return false;
  const auto kind = r.value("kind").toString(),
             cap = r.value("capability").toString();
  if (!(((kind == "security-scan" || kind == "partition-analysis") &&
         cap == "external-analysis") ||
        (kind == "signature-update" && cap == "signature-update")))
    return false;
  const auto status = r.value("status").toString();
  if (result != status || !r.value("startedUtc").toString().endsWith('Z') ||
      !QDateTime::fromString(r.value("startedUtc").toString(),
                             Qt::ISODateWithMs)
           .isValid())
    return false;
  if (start)
    return old.isEmpty() && request.value("status") == "active" &&
           actor == r.value("requester") && role == r.value("requesterRole") &&
           status == "started" && r.value("startedUtc") == time &&
           r.value("completedUtc").toString().isEmpty() &&
           r.value("failureClass").toString().isEmpty() &&
           r.value("outputSha256").toString().isEmpty();
  if (old.value("status") != "started" || role != "system-observation" ||
      !actor.isEmpty() || r.value("completedUtc") != time ||
      !time.endsWith('Z') ||
      !QDateTime::fromString(time, Qt::ISODateWithMs).isValid())
    return false;
  auto a = old, b = r;
  for (const auto &key : {"status", "completedUtc", "failureClass",
                          "outputSha256", "sourceSha256"}) {
    a.remove(key);
    b.remove(key);
  }
  if (a != b || (!old.value("sourceSha256").toString().isEmpty() &&
                 old.value("sourceSha256") != r.value("sourceSha256")))
    return false;
  const auto failure = r.value("failureClass").toString();
  if (failure == "process-interrupted")
    return status == "failed";
  if (closure == "expire")
    return status == "expired" && failure == "permission-expired";
  if (closure == "session-close")
    return status == "cancelled" && failure == "session-ended";
  if (closure == "restart-close" || failure == "process-interrupted")
    return status == "failed" && failure == "process-interrupted";
  if (!closure.isEmpty())
    return status == "revoked" && failure == "permission-revoked";
  return request.value("status") == "active" &&
         ((status == "completed" && failure == "none" &&
           !r.value("outputSha256").toString().isEmpty() &&
           (kind == "signature-update" ||
            !r.value("sourceSha256").toString().isEmpty())) ||
          (status == "failed" && failure == "engine-failure") ||
          (status == "cancelled" && failure == "operator-cancelled"));
}
} // namespace mr::policy
