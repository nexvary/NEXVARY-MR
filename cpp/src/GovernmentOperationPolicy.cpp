#include "GovernmentOperationPolicy.hpp"
#include "IdentityService.hpp"
#include "OperationReceipt.hpp"
#include "CaseOperationIntent.hpp"
#include "PolicyAudit.hpp"
#include "RecoveryFile.hpp"
#include <QDateTime>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <QUrl>
#include <QUuid>
#include <algorithm>
namespace {
QString utc() {
  return QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
}
bool note(const QString &s) {
  if (s.trimmed().isEmpty() || s.size() > 256)
    return false;
  for (const auto c : s)
    if (c.unicode() < 32 || c.unicode() == 127 ||
        (c.unicode() >= 0x202a && c.unicode() <= 0x202e) ||
        (c.unicode() >= 0x2066 && c.unicode() <= 0x2069))
      return false;
  return true;
}
bool terminal(const QString &s) {
  return s == "rejected" || s == "revoked" || s == "expired";
}
} // namespace
QStringList GovernmentOperationPolicy::capabilities() {
  return {"cloud-publish", "tracking-portal",   "signature-update",
          "device-tools",  "external-analysis", "external-links"};
}
GovernmentOperationPolicy::GovernmentOperationPolicy(IdentityService *identity,
                                                     QObject *parent,
                                                     int lifetimeMs)
    : QObject(parent), identity_(identity),
      lifetimeMs_(qBound(10, lifetimeMs, 600000)) {
  clock_.start();
  monitor_.setInterval(qMin(1000, lifetimeMs_));

  audit_ = std::make_unique<PolicyAudit>(
      identity ? identity->policyJournalPath() : QString(),
      identity && identity->syntheticProfile());

  if (audit_->healthy()) {
    for (const auto &v : audit_->events()) {
      const auto e = v.toMap();
      const auto bytes =
          QByteArray::fromBase64(e.value("payload").toString().toLatin1());
      const auto p = QJsonDocument::fromJson(bytes).object();
      const auto r = p.value("record").toObject().toVariantMap();
      const auto action = p.value("action").toString();
      if (action == "operation-start")
        operations_.insert(r.value("operationId").toString(), r);
      if (action == "operation-result")
        operations_.remove(r.value("operationId").toString());
      if (QStringList{"expire", "session-close", "restart-close",
                      "emergency-stop", "revoke"}
              .contains(action))
        operationClosures_.insert(r.value("id").toString(), action);

      if (r.contains("id") &&
          !p.value("action").toString().startsWith("operation-") &&
          p.value("action").toString() != "browser-result")
        requests_.insert(r.value("id").toString(), r);
    }
    const auto snapshot = requests_;

    for (auto it = snapshot.cbegin(); it != snapshot.cend(); ++it)
      if (it->value("status") == "active")
        transition(it.key(), "restart-close", "revoked");
    for (auto it = snapshot.cbegin(); it != snapshot.cend(); ++it)
      if (it->value("status") == "active")
        operationClosures_.insert(it.key(), "restart-close");
    const auto unfinished = operations_.keys();
    for (const auto &id : unfinished)
      finishOperation(id, "interrupted");

  } else
    error_ = audit_->error();

  connect(&monitor_, &QTimer::timeout, this,
          &GovernmentOperationPolicy::refresh);

  if (identity_)
    connect(identity_, &IdentityService::changed, this, [this] {
      // Account saves also emit changed. Only an actual session mismatch closes
      // permissions, so signing/session checks cannot recursively revoke them.
      const auto active = grants_;
      bool changedState = false;

      for (auto it = active.cbegin(); it != active.cend(); ++it)
        if (!identity_ || !identity_->authenticated() ||
            it->actor != identity_->actor()) {
          closeGrant(it.key(), "session-close");
          changedState = true;
        }
      if (changedState || grants_.isEmpty())
        emit changed();
      emit auditChanged();
    });

  monitor_.start();
}
GovernmentOperationPolicy::~GovernmentOperationPolicy() { revokeAll(); }
bool GovernmentOperationPolicy::fail(const QString &error) {
  error_ = error;
  // A rejected API decision is a system observation, never a forged user
  // signature. No source contents, passwords, URLs or credentials are recorded.
  if (audit_ && audit_->healthy()) {
    if (!audit_->append(
            "policy-rejection",
            {{"reason", error},
             {"observedActor", identity_ ? identity_->actor() : QString()}},
            "denied")) {
      const bool active = !grants_.isEmpty();
      grants_.clear();
      error_ += QStringLiteral(" / ") + audit_->error();
      if (active)
        emit changed();
    }
    emit auditChanged();
  }
  emit errorChanged();
  return false;
}
bool GovernmentOperationPolicy::available() {
  if (audit_ && audit_->check())
    return true;

  const bool active = !grants_.isEmpty();
  grants_.clear();
  if (active)
    emit changed();
  return fail(audit_ ? audit_->error()
                     : QStringLiteral("Policy audit unavailable."));
}
bool GovernmentOperationPolicy::audit(const QString &action,
                                      const QVariantMap &record,
                                      const QString &result, bool human) {
  if (!available())
    return false;

  if (!audit_->append(action, record, result, human ? identity_ : nullptr)) {
    available();
    return false;
  }
  emit auditChanged();
  return true;
}
bool GovernmentOperationPolicy::approvalCurrent(const QString &id) {
  if (!identity_)
    return false;
  const auto events = audit_->events();
  for (auto it = events.crbegin(); it != events.crend(); ++it) {
    const auto p = QJsonDocument::fromJson(
                       QByteArray::fromBase64(
                           it->toMap().value("payload").toString().toLatin1()))
                       .object()
                       .toVariantMap();
    if (p.value("action") == "approve" &&
        p.value("record").toMap().value("id") == id) {
      const auto proof = p.value("actorProof").toMap();
      auto envelope = QJsonDocument::fromJson(
                          QByteArray::fromBase64(
                              proof.value("message").toString().toLatin1())
                              .mid(32))
                          .object()
                          .toVariantMap();
      envelope.insert("signature", proof.value("signature"));
      return identity_->verify(envelope, "government-policy-event",
                               envelope.value("payload").toMap());
    }
  }
  return false;
}
bool GovernmentOperationPolicy::granted(const QString &capability) const {
  const auto it = grants_.constFind(capability);
  return audit_ && audit_->healthy() && it != grants_.cend() &&
         clock_.elapsed() < it->deadline && identity_ &&
         identity_->authenticated() && it->actor == identity_->actor();
}
bool GovernmentOperationPolicy::offline() const {
  for (const auto &name : capabilities())
    if (granted(name))
      return false;
  return true;
}
QVariantList GovernmentOperationPolicy::permissions() const {
  QVariantList rows;
  for (const auto &name : capabilities()) {
    const auto g = grants_.value(name);
    rows.append(QVariantMap{{"capability", name},
                            {"allowed", granted(name)},
                            {"reason", g.reason},
                            {"actor", g.actor},
                            {"expiresUtc", g.expiresUtc},
                            {"requestId", g.id}});
  }
  return rows;
}
QVariantList GovernmentOperationPolicy::requests() const {
  QVariantList rows;
  if (!identity_ || !identity_->authenticated())
    return rows;
  const auto role = identity_->session().value("role").toString();

  for (auto it = requests_.cbegin(); it != requests_.cend(); ++it)
    if (role == "administrator" || role == "reviewer" || role == "auditor" ||
        it->value("requester") == identity_->userId())
      rows.prepend(it.value());

  std::sort(rows.begin(), rows.end(), [](const QVariant &a, const QVariant &b) {
    const auto x = a.toMap(), y = b.toMap();
    return x.value("requestedUtc").toString() ==
                   y.value("requestedUtc").toString()
               ? x.value("id").toString() > y.value("id").toString()
               : x.value("requestedUtc").toString() >
                     y.value("requestedUtc").toString();
  });
  return rows;
}
QVariantMap GovernmentOperationPolicy::auditStatus() const {
  auto r = audit_->anchor();
  r.insert("healthy", audit_->healthy());
  r.insert("verification", verification_);
  return r;
}
bool GovernmentOperationPolicy::transition(const QString &id,
                                           const QString &action,
                                           const QString &status,
                                           const QString &reason) {
  auto r = requests_.value(id);
  if (r.isEmpty())
    return fail("Unknown exception request.");
  r.insert("status", status);

  const bool system = action == "expire" || action == "session-close" ||
                      action == "restart-close" || action == "emergency-stop";

  if (action == "review") {
    r.insert("reviewer", identity_->userId());
    r.insert("reviewReason", reason.trimmed());
  }
  if (action == "approve" || action == "reject")
    r.insert("decisionReason", reason.trimmed());

  if (action == "activate") {
    r.insert("activatedUtc", utc());
    r.insert("expiresUtc",
             QDateTime::currentDateTimeUtc()
                 .addMSecs(qMin(lifetimeMs_, r.value("durationMs").toInt()))
                 .toString(Qt::ISODateWithMs));
  }
  if (terminal(status))
    r.insert("closedUtc", utc());

  if (!audit(action, r, status,
             !system && identity_ && identity_->authenticated()))
    return false;

  requests_.insert(id, r);
  error_.clear();
  emit errorChanged();
  emit changed();
  return true;
}
void GovernmentOperationPolicy::refresh() {
  if (refreshing_)
    return;
  refreshing_ = true;

  if (!available()) {
    refreshing_ = false;
    return;
  }
  if (!grants_.isEmpty() && (!identity_ || !identity_->checkSession())) {
    revokeAll();
    refreshing_ = false;
    return;
  }
  const auto active = grants_;
  for (auto it = active.cbegin(); it != active.cend(); ++it)
    if (clock_.elapsed() >= it->deadline)
      closeGrant(it.key(), "expire");
    else if (!approvalCurrent(it->id))
      closeGrant(it.key(), "session-close");

  const auto now = QDateTime::currentDateTimeUtc();
  const auto snapshot = requests_;

  for (auto it = snapshot.cbegin(); it != snapshot.cend(); ++it) {
    const auto s = it->value("status").toString();
    if (s != "active" && !terminal(s) &&
        now >= QDateTime::fromString(it->value("requestDeadlineUtc").toString(),
                                     Qt::ISODateWithMs))
      transition(it.key(), "expire", "expired");
  }
  refreshing_ = false;
}
void GovernmentOperationPolicy::refreshAudit() {
  refresh();
  emit auditChanged();
  emit changed();
}
bool GovernmentOperationPolicy::permit(const QString &capability) {
  refresh();
  const bool allowed = granted(capability);

  QVariantMap r{
      {"capability", capability},
      {"scope", "category-and-requester; not destination containment"}};

  if (allowed)
    r = requests_.value(grants_.value(capability).id);

  if (!audit(allowed ? "operation-allowed" : "operation-denied", r,
             allowed ? "gate-allowed; operation completion not established"
                     : "gate-denied",
             false))
    return false;

  if (allowed)
    return true;

  return fail(
      QStringLiteral(
          "Government offline policy blocked %1. Request an exception, obtain "
          "a different authorized reviewer's decision, then activate it in the "
          "requesting account. Saved settings do not grant permission.")
          .arg(capability));
}
bool GovernmentOperationPolicy::grant(const QString &, const QString &, bool) {
  return fail("Direct grants are disabled. Request, separate-account "
              "review/decision and requester activation are required.");
}
QString GovernmentOperationPolicy::requestException(const QString &capability,
                                                    const QString &reason,
                                                    const QString &caseRef,
                                                    int seconds, bool consent) {
  refresh();
  if (!available() || !identity_ || !identity_->checkSession())
    return {};

  const auto role = identity_->session().value("role").toString();

  if (!consent || !capabilities().contains(capability) || !note(reason) ||
      seconds < 1 || seconds > 600 ||
      (!caseRef.isEmpty() && QUuid(caseRef).isNull()) ||
      (role != "administrator" && role != "investigator" &&
       role != "repair-technician")) {
    fail("Authenticated requesting role, known category, reason, optional case "
         "UUID, 1–600 seconds and explicit consent are required.");
    return {};
  }
  int open = 0;
  for (auto it = requests_.cbegin(); it != requests_.cend(); ++it)
    if (!terminal(it->value("status").toString())) {
      ++open;
      if (it->value("requester") == identity_->userId() &&
          it->value("capability") == capability) {
        fail("Close the existing request for this category before requesting "
             "again.");
        return {};
      }
    }
  if (open >= 64) {
    fail("Open exception request limit reached (64).");
    return {};
  }
  const auto id = QUuid::createUuid().toString(QUuid::WithoutBraces);

  const QVariantMap r{
      {"id", id},
      {"requester", identity_->userId()},
      {"requesterActor", identity_->actor()},
      {"requesterRole", role},
      {"capability", capability},
      {"scope", "category-and-requester; not destination containment"},
      {"caseRef", caseRef},
      {"reason", reason.trimmed()},
      {"durationMs", seconds * 1000},
      {"requestedUtc", utc()},
      {"requestDeadlineUtc",
       QDateTime::currentDateTimeUtc().addSecs(600).toString(
           Qt::ISODateWithMs)},
      {"status", "requested"}};

  if (!audit("request", r, "requested", true))
    return {};

  requests_.insert(id, r);
  error_.clear();
  emit errorChanged();
  emit changed();
  return id;
}
bool GovernmentOperationPolicy::reviewException(const QString &id,
                                                const QString &reason) {
  refresh();
  if (!identity_ || !identity_->checkSession())
    return fail("Sign in to review.");
  const auto r = requests_.value(id);
  const auto role = identity_->session().value("role").toString();

  if (r.value("status") != "requested" ||
      r.value("requester") == identity_->userId() ||
      (role != "administrator" && role != "reviewer") || !note(reason))
    return fail("A different local administrator/reviewer must review a "
                "requested exception with a reason.");

  return transition(id, "review", "in-review", reason);
}
bool GovernmentOperationPolicy::decideException(const QString &id, bool approve,
                                                const QString &reason) {
  refresh();
  if (!identity_ || !identity_->checkSession())
    return fail("Sign in to decide.");
  const auto r = requests_.value(id);
  const auto role = identity_->session().value("role").toString();

  if (r.value("status") != "in-review" ||
      r.value("reviewer") != identity_->userId() ||
      r.value("requester") == identity_->userId() ||
      (role != "administrator" && role != "reviewer") || !note(reason))
    return fail("Only the assigned separate reviewer may approve/reject an "
                "in-review request with a reason.");

  return transition(id, approve ? "approve" : "reject",
                    approve ? "approved" : "rejected", reason);
}
bool GovernmentOperationPolicy::activateException(const QString &id) {
  refresh();
  if (!available() || !identity_ || !identity_->checkSession())
    return false;
  const auto r = requests_.value(id);

  if (r.value("status") != "approved" ||
      r.value("requester") != identity_->userId() ||
      grants_.contains(r.value("capability").toString()))
    return fail("Only the original requester can activate an unused approved "
                "exception.");

  if (r.value("requesterRole") != identity_->session().value("role") ||
      !approvalCurrent(id))
    return fail("Requester/reviewer authority changed; obtain a new decision.");
  if (!transition(id, "activate", "active"))
    return false;

  grants_.insert(
      r.value("capability").toString(),
      {r.value("reason").toString(), identity_->actor(),
       requests_.value(id).value("expiresUtc").toString(), id,
       clock_.elapsed() + qMin(lifetimeMs_, r.value("durationMs").toInt())});
  emit changed();
  return true;
}
void GovernmentOperationPolicy::closeGrant(const QString &capability,
                                           const QString &action) {
  const auto g = grants_.take(capability);
  if (g.id.isEmpty())
    return;
  operationClosures_.insert(g.id, action);
  transition(g.id, action, action == "expire" ? "expired" : "revoked");
  emit changed();
}
bool GovernmentOperationPolicy::revokeException(const QString &id) {
  if (!identity_ || !identity_->checkSession())
    return fail("Sign in to revoke a pending/approved request; active "
                "permissions can always be stopped.");
  const auto r = requests_.value(id);

  if (r.isEmpty() || terminal(r.value("status").toString()) ||
      (identity_->session().value("role") != "administrator" &&
       r.value("requester") != identity_->userId() &&
       r.value("reviewer") != identity_->userId()))
    return fail("Request revocation authority/state rejected.");

  const auto cap = r.value("capability").toString();
  if (grants_.value(cap).id == id) {
    operationClosures_.insert(id, "revoke");
    grants_.remove(cap);
    emit changed();
  }
  return transition(id, "revoke", "revoked");
}
void GovernmentOperationPolicy::revoke(const QString &capability) {
  closeGrant(capability, "emergency-stop");
}
void GovernmentOperationPolicy::revokeAll() {
  const auto active = grants_;
  for (auto it = active.cbegin(); it != active.cend(); ++it)
    closeGrant(it.key(), "session-close");
}
bool GovernmentOperationPolicy::exportAudit(const QString &input,
                                            bool consent) {
  if (!available() || !identity_ || !identity_->checkSession())
    return false;
  const auto role = identity_->session().value("role").toString();

  if (!consent ||
      (role != "administrator" && role != "reviewer" && role != "auditor"))
    return fail("Audit export needs explicit unencrypted-export consent and an "
                "administrator/reviewer/auditor.");

  const auto path = mr::recovery::localFilePath(input);
  if (path.isEmpty() || QFileInfo::exists(path) || QFileInfo(path).isSymLink())
    return fail("Choose a new regular local audit export path.");

  QFile f(path);
  const auto bytes = audit_->exportBytes();
  if (!f.open(QIODevice::WriteOnly | QIODevice::NewOnly) ||
      !f.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner) ||
      f.write(bytes) != bytes.size() || !f.flush()) {
    if (f.isOpen()) {
      f.close();
      QFile::remove(path);
    }
    return fail("Cannot create audit export.");
  }
  error_.clear();
  emit errorChanged();
  return true;
}
bool GovernmentOperationPolicy::verifyAudit(const QString &input,
                                            const QString &anchorJson) {
  if (!available() || !identity_ || !identity_->checkSession())
    return false;
  const auto role = identity_->session().value("role").toString();
  if (role != "administrator" && role != "reviewer" && role != "auditor")
    return fail("An audit review role is required.");

  const auto path = mr::recovery::localFilePath(input);
  QFile f(path);
  if (path.isEmpty() || QFileInfo(path).isSymLink() ||
      !mr::recovery::openRegularInput(f) || f.size() > 16 * 1024 * 1024)
    return fail("Invalid bounded audit input.");

  const auto anchor = PolicyAudit::parseAnchor(anchorJson.toUtf8());
  QString why;
  const bool ok =
      !anchor.isEmpty() &&
      PolicyAudit::verifyExport(
          f.readAll(), anchor.value("publicKey").toString(), anchor, why);

  verification_ = {{"valid", ok},
                   {"timeUtc", utc()},
                   {"reason", ok ? "Signatures, order and retained prefix "
                                   "verified; local identity/time only"
                                 : why},
                   {"anchor", anchor}};

  if (!audit("audit-verification", verification_, ok ? "verified" : "rejected",
             true))
    return false;

  emit changed();
  if (!ok)
    return fail("Independent-history verification rejected: " + why);
  error_.clear();
  emit errorChanged();
  return true;
}
bool GovernmentOperationPolicy::openExternal(const QString &text) {
  if (!permit("external-links"))
    return false;
  const QUrl url(text, QUrl::StrictMode);

  const bool valid = text.size() <= 2048 && url.isValid() &&
                     !url.host().isEmpty() && url.userInfo().isEmpty() &&
                     (url.scheme() == "https" || url.scheme() == "http");
  const bool opened = valid && QDesktopServices::openUrl(url);

  if (!audit(
          "browser-result", requests_.value(grants_.value("external-links").id),
          opened  ? "handoff-accepted; subsequent browser activity uncontained"
          : valid ? "handoff-failed"
                  : "invalid-url",
          false))
    return false;

  if (!valid)
    return fail("Only an explicit HTTP/HTTPS link without embedded credentials "
                "may be opened.");

  if (!opened)
    return fail("The system could not open the selected link.");
  error_.clear();
  emit errorChanged();
  return true;
}

QString GovernmentOperationPolicy::beginOperation(const QString &capability,
                                                  const QString &kind) {
  if (!permit(capability) || operations_.size() >= 64)
    return {};
  const auto request = requests_.value(grants_.value(capability).id);
  QVariantMap decision;
  for (const auto &v : audit_->events()) {
    const auto e = v.toMap();
    const auto p =
        QJsonDocument::fromJson(
            QByteArray::fromBase64(e.value("payload").toString().toLatin1()))
            .object();
    if (p.value("action") == "approve" &&
        p.value("record").toObject().value("id").toString() ==
            request.value("id"))
      decision = {{"sequence", p.value("sequence").toVariant()},
                  {"hash", e.value("hash")}};
  }
  const auto now = utc(),
             id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  const QVariantMap r{{"schema", "nexvary-operation-result/v1"},
                      {"operationId", id},
                      {"requestId", request.value("id")},
                      {"requester", request.value("requester")},
                      {"requesterRole", request.value("requesterRole")},
                      {"caseRef", request.value("caseRef")},
                      {"capability", capability},
                      {"kind", kind},
                      {"decisionSequence", decision.value("sequence")},
                      {"decisionHash", decision.value("hash")},
                      {"startedUtc", now},
                      {"completedUtc", QString()},
                      {"status", "started"},
                      {"failureClass", QString()},
                      {"sourceSha256", QString()},
                      {"outputSha256", QString()}};
  if (!mr::policy::operationRecord(
          r, request, decision, {}, {}, identity_->userId(),
          identity_->session().value("role").toString(), now, "started", true))
    return {};
  if (!audit("operation-start", r, "started", true))
    return {};
  operations_.insert(id, r);
  emit auditChanged();
  return id;
}
QVariantMap GovernmentOperationPolicy::finishOperation(
    const QString &id, const QString &status, const QString &sourceDigest,
    const QString &outputDigest) {
  refresh();
  auto r = operations_.value(id);
  if (r.isEmpty() || !available())
    return {};
  QString terminalStatus = status, failure;
  const auto closure =
      operationClosures_.value(r.value("requestId").toString());
  if (status == "interrupted") {
    terminalStatus = "failed";
    failure = "process-interrupted";
  } else if (closure == "expire") {
    terminalStatus = "expired";
    failure = "permission-expired";
  } else if (closure == "session-close") {
    terminalStatus = "cancelled";
    failure = "session-ended";
  } else if (closure == "restart-close" || status == "interrupted") {
    terminalStatus = "failed";
    failure = "process-interrupted";
  } else if (!closure.isEmpty()) {
    terminalStatus = "revoked";
    failure = "permission-revoked";
  } else if (status == "completed")
    failure = "none";
  else if (status == "failed")
    failure = "engine-failure";
  else if (status == "cancelled")
    failure = "operator-cancelled";
  else
    return {};
  const auto old = r;
  const auto now = utc();
  r.insert("status", terminalStatus);
  r.insert("failureClass", failure);
  r.insert("completedUtc", now);
  r.insert("sourceSha256", sourceDigest);
  r.insert("outputSha256", outputDigest);
  const QVariantMap decision{{"sequence", r.value("decisionSequence")},
                             {"hash", r.value("decisionHash")}};
  if (!mr::policy::operationRecord(
          r, requests_.value(r.value("requestId").toString()), decision, old,
          closure, {}, "system-observation", now, terminalStatus, false) ||
      !audit("operation-result", r, terminalStatus, false))
    return {};
  operations_.remove(id);
  emit auditChanged();
  return r;
}
QVariantList GovernmentOperationPolicy::operationReceipts() const {
  QVariantList result;
  if (!identity_ || !identity_->authenticated())
    return result;
  const auto role = identity_->session().value("role").toString();
  const bool review =
      role == "administrator" || role == "reviewer" || role == "auditor";
  QSet<QString> seen;
  const auto events = audit_->events();
  for (auto it = events.crbegin(); it != events.crend() && result.size() < 32;
       ++it) {
    const auto e = it->toMap();
    const auto p =
        QJsonDocument::fromJson(
            QByteArray::fromBase64(e.value("payload").toString().toLatin1()))
            .object();
    if (p.value("action") != "operation-start" &&
        p.value("action") != "operation-result")
      continue;
    auto r = p.value("record").toObject().toVariantMap();
    const auto id = r.value("operationId").toString();
    if (seen.contains(id) ||
        (!review && r.value("requester") != identity_->userId()))
      continue;
    seen.insert(id);
    r.insert("eventSequence", p.value("sequence").toVariant());
    r.insert("eventHash", e.value("hash"));
    result.append(r);
  }
  return result;
}

QVariantList GovernmentOperationPolicy::caseOperationStates() {
 caseHistoryHealthy_=false;
 if(!available())return {};
 QString why;
 if(!PolicyAudit::verifyExport(audit_->exportBytes(),audit_->anchor().value("publicKey").toString(),audit_->anchor(),why)){
  fail("Case operation history rejected: "+why);return {};
 }
 QMap<QString,QVariantMap> states;
 for(const auto &value:audit_->events()){
  const auto event=value.toMap();
  const auto p=QJsonDocument::fromJson(QByteArray::fromBase64(event.value("payload").toString().toLatin1())).object();
  if(p.value("action")=="case-operation"){
   const auto r=p.value("record").toObject().toVariantMap();states.insert(r.value("operationId").toString(),r);
  }
 }
 caseHistoryHealthy_=true;
 QVariantList result;for(const auto &r:states)result.append(r);return result;
}
bool GovernmentOperationPolicy::recordCaseOperation(const QVariantMap &r) {
 const auto states=caseOperationStates();
 if(!caseHistoryHealthy_ || !available())return false;
 QVariantMap old;
 for(const auto &value:states)if(value.toMap().value("operationId")==r.value("operationId"))old=value.toMap();
 // Terminal observations are immutable. Identical retries do not append events.
 if(!old.isEmpty() && old==r && old.value("recoveryState")!="prepared")return true;
 const bool prepared=r.value("recoveryState")=="prepared";
 if(prepared){
  int unfinished=0;
  for(const auto &value:states){
   const auto previous=value.toMap();
   if(previous.value("recoveryState")=="prepared")++unfinished;
   if(previous.value("requestId")==r.value("requestId") || previous.value("analysisId")==r.value("analysisId"))return false;
  }
  if(unfinished>=64)return false;
 }
 if(prepared && (!identity_ || !identity_->checkSession() || !nexvary::law::allows(identity_->lawRole(),nexvary::law::Action::AnalyzeEvidence)))return false;
 if(!mr::law::caseIntentEvent(r,old,prepared?identity_->userId():QString(),prepared?identity_->session().value("role").toString():QString("system-observation"),r.value("recoveryState").toString()))return false;
 return audit("case-operation",r,r.value("recoveryState").toString(),prepared);
}

void GovernmentOperationPolicy::setSyntheticAuditWriteCheckpoint(std::function<void()> callback) {
 if(audit_)audit_->setSyntheticWriteCheckpoint(std::move(callback));
}
