#include "GovernmentOperationPolicy.hpp"
#include "IdentityService.hpp"
#include "PolicyAudit.hpp"
#include "SyntheticPolicyExercise.hpp"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QTemporaryDir>
#include <QtTest>
#include <cstdio>
#include <cstdlib>
namespace {
const QString pass = "Synthetic policy audit fixture 2026!";
void setup(IdentityService &i) {
  if (!i.bootstrap("request-admin", "Synthetic requesting admin", pass) ||
      !i.login("request-admin", pass) ||
      i.createUser("review-admin", "Synthetic reviewing admin", "administrator",
                   pass)
          .isEmpty() ||
      i.createUser("reviewer", "Synthetic reviewer", "reviewer", pass)
          .isEmpty() ||
      i.createUser("investigator", "Synthetic investigator", "investigator",
                   pass)
          .isEmpty() ||
      i.createUser("auditor", "Synthetic auditor", "auditor", pass).isEmpty())
    qFatal("Policy fixture setup failed");
}
QVariantMap anchor(GovernmentOperationPolicy &p) {
  auto a = p.auditStatus();
  a.remove("healthy");
  a.remove("verification");
  return a;
}
QByteArray json(const QVariantMap &v) {
  return QJsonDocument(QJsonObject::fromVariantMap(v))
      .toJson(QJsonDocument::Compact);
}
QByteArray bytes(const QString &p) {
  QFile f(p);
  if (!f.open(QIODevice::ReadOnly))
    return {};
  return f.readAll();
}
bool write(const QString &p, const QByteArray &b) {
  QFile f(p);
  return f.open(QIODevice::WriteOnly) && f.write(b) == b.size();
}
QString approve(IdentityService &i, GovernmentOperationPolicy &p) {
  const auto id = p.requestException(
      "device-tools", QStringLiteral("تجربة اصطناعية"), {}, 5, true);
  if (id.isEmpty() || !i.login("reviewer", pass) ||
      !p.reviewException(id, "Synthetic separate-account review") ||
      !p.decideException(id, true, "Synthetic approve") ||
      !i.login("request-admin", pass))
    return {};
  return id;
}
int fixture(const QString &root, bool crash) {
  IdentityService i(root + "/identity", true);
  setup(i);
  GovernmentOperationPolicy p(&i);
  const auto id = approve(i, p);
  if (id.isEmpty() || !p.activateException(id) || !p.permit("device-tools"))
    return 9;
  if (!crash) {
    if (!performSyntheticReviewedException(i, p, "external-analysis",
                                           "Synthetic worker", pass))
      return 9;
    const auto op = p.beginOperation("external-analysis", "security-scan");
    if (op.isEmpty() ||
        p.finishOperation(op, "completed", QString(64, 'b'), QString(64, 'a'))
            .isEmpty())
      return 9;
  }
  if (!p.exportAudit(root + "/audit.json", true) ||
      !write(root + "/anchor.json", json(anchor(p))))
    return 9;
  if (crash)
    std::_Exit(0);
  p.revoke("device-tools");
  return 0;
}
} // namespace
class PolicyAuditTests final : public QObject {
  Q_OBJECT
private slots:
  void operationResultLifecycle() {
    QTemporaryDir t;
    IdentityService i(t.filePath("identity"), true);
    setup(i);
    GovernmentOperationPolicy p(&i);
    QVERIFY(p.beginOperation("external-analysis", "security-scan").isEmpty());
    QVERIFY(performSyntheticReviewedException(
        i, p, "external-analysis", "Synthetic operation receipts", pass));
    QVERIFY(p.beginOperation("external-analysis", "not-a-registered-kind")
                .isEmpty());
    const auto op = p.beginOperation("external-analysis", "security-scan");
    QVERIFY(!op.isEmpty());
    QVERIFY(p.finishOperation(op, "completed", {}, QString(64, 'a')).isEmpty());
    const auto r =
        p.finishOperation(op, "completed", QString(64, 'b'), QString(64, 'a'));
    QCOMPARE(r.value("status").toString(), QString("completed"));
    QVERIFY(p.finishOperation(op, "failed").isEmpty());
    const auto cancelled =
        p.beginOperation("external-analysis", "security-scan");
    QCOMPARE(p.finishOperation(cancelled, "cancelled")
                 .value("failureClass")
                 .toString(),
             QString("operator-cancelled"));
    const auto revoked = p.beginOperation("external-analysis", "security-scan");
    p.revoke("external-analysis");
    QCOMPARE(p.finishOperation(revoked, "completed", QString(64, 'b'),
                               QString(64, 'a'))
                 .value("status")
                 .toString(),
             QString("revoked"));
    QVERIFY(performSyntheticReviewedException(i, p, "external-analysis",
                                              "Synthetic logout", pass));
    const auto logout = p.beginOperation("external-analysis", "security-scan");
    i.logout();
    QCOMPARE(p.finishOperation(logout, "completed", QString(64, 'b'),
                               QString(64, 'a'))
                 .value("failureClass")
                 .toString(),
             QString("session-ended"));
    QVERIFY(p.operationReceipts().isEmpty());
    QVERIFY(i.login("request-admin", pass));
    const auto file = t.filePath("audit.json");
    QVERIFY(p.exportAudit(file, true));
    QString why;
    QVERIFY2(PolicyAudit::verifyExport(bytes(file),
                                       anchor(p).value("publicKey").toString(),
                                       anchor(p), why),
             qPrintable(why));
  }
  void operationExpiryAndRestart() {
    QTemporaryDir t;
    IdentityService i(t.filePath("identity"), true);
    setup(i);
    {
      GovernmentOperationPolicy p(&i, nullptr, 100);
      QVERIFY(performSyntheticReviewedException(i, p, "external-analysis",
                                                "Synthetic expiry", pass));
      const auto id = p.beginOperation("external-analysis", "security-scan");
      QVERIFY(!id.isEmpty());
      QTest::qWait(150);
      QCOMPARE(
          p.finishOperation(id, "completed", QString(64, 'b'), QString(64, 'a'))
              .value("status")
              .toString(),
          QString("expired"));
    }
    {
      GovernmentOperationPolicy p(&i);
      QVERIFY(performSyntheticReviewedException(
          i, p, "external-analysis", "Synthetic interrupted worker", pass));
      QVERIFY(
          !p.beginOperation("external-analysis", "security-scan").isEmpty());
    }
    GovernmentOperationPolicy reopened(&i);
    QCOMPARE(reopened.operationReceipts()
                 .first()
                 .toMap()
                 .value("failureClass")
                 .toString(),
             QString("process-interrupted"));
    const auto file = t.filePath("audit.json");
    QVERIFY(reopened.exportAudit(file, true));
    QString why;
    QVERIFY2(PolicyAudit::verifyExport(
                 bytes(file), anchor(reopened).value("publicKey").toString(),
                 anchor(reopened), why),
             qPrintable(why));
  }
  void requestInputsAndRoles() {
    QTemporaryDir t;
    IdentityService i(t.filePath("identity"), true);
    setup(i);
    GovernmentOperationPolicy p(&i);
    QVERIFY(!p.grant("device-tools", "direct", true));
    for (const auto &cap : {"unknown", ""})
      QVERIFY(p.requestException(cap, "valid", {}, 10, true).isEmpty());
    for (const auto &note : {QString(), QString(257, 'x'),
                             QStringLiteral("x\ny"), QString(QChar(0x202e))})
      QVERIFY(p.requestException("device-tools", note, {}, 10, true).isEmpty());
    for (int seconds : {0, -1, 601})
      QVERIFY(p.requestException("device-tools", "valid", {}, seconds, true)
                  .isEmpty());
    QVERIFY(
        p.requestException("device-tools", "valid", "not-case-uuid", 10, true)
            .isEmpty());
    QVERIFY(
        p.requestException("device-tools", "valid", {}, 10, false).isEmpty());
    QVERIFY(i.login("auditor", pass));
    QVERIFY(
        p.requestException("device-tools", "valid", {}, 10, true).isEmpty());
    QVERIFY(i.login("reviewer", pass));
    QVERIFY(
        p.requestException("device-tools", "valid", {}, 10, true).isEmpty());
  }
  void separatedReviewAndOneTimeActivation() {
    QTemporaryDir t;
    IdentityService i(t.filePath("identity"), true);
    setup(i);
    GovernmentOperationPolicy p(&i);
    const auto id = p.requestException(
        "device-tools", "Synthetic authority test", {}, 10, true);
    QVERIFY(!id.isEmpty());
    QVERIFY(p.offline());
    QVERIFY(!p.reviewException(id, "Self review"));
    QVERIFY(!p.decideException(id, true, "Unreviewed"));
    QVERIFY(!p.activateException(id));
    QVERIFY(p.requestException("device-tools", "Duplicate", {}, 10, true)
                .isEmpty());
    QVERIFY(i.login("reviewer", pass));
    QVERIFY(p.reviewException(id, "Separate account"));
    QVERIFY(i.login("review-admin", pass));
    QVERIFY(!p.decideException(id, true, "Not assigned"));
    QVERIFY(i.login("reviewer", pass));
    QVERIFY(p.decideException(id, true, "Approved bounded category"));
    QVERIFY(!p.activateException(id));
    QVERIFY(i.login("request-admin", pass));
    QVERIFY(p.activateException(id));
    QVERIFY(p.permit("device-tools"));
    QVERIFY(!p.permit("external-links"));
    QVERIFY(!p.activateException(id));
    p.revoke("device-tools");
    QVERIFY(p.offline());
    QVERIFY(!p.activateException(id));
  }
  void rejectionAndPendingRevocation() {
    QTemporaryDir t;
    IdentityService i(t.filePath("identity"), true);
    setup(i);
    GovernmentOperationPolicy p(&i);
    const auto id =
        p.requestException("device-tools", "Synthetic rejection", {}, 10, true);
    QVERIFY(i.login("reviewer", pass));
    QVERIFY(p.reviewException(id, "Review"));
    QVERIFY(p.decideException(id, false, "Reject"));
    QVERIFY(!p.decideException(id, true, "Cannot change final rejection"));
    QVERIFY(i.login("request-admin", pass));
    QVERIFY(!p.activateException(id));
    const auto next =
        p.requestException("device-tools", "Synthetic revoke", {}, 10, true);
    QVERIFY(!next.isEmpty());
    QVERIFY(i.login("investigator", pass));
    QVERIFY(!p.revokeException(next));
    QVERIFY(p.requests().isEmpty());
    QVERIFY(i.login("request-admin", pass));
    QVERIFY(p.revokeException(next));
    QVERIFY(!p.reviewException(next, "Already revoked"));
  }
  void monotonicExpiryAndLogout() {
    QTemporaryDir t;
    IdentityService i(t.filePath("identity"), true);
    setup(i);
    GovernmentOperationPolicy p(&i, nullptr, 100);
    QVERIFY(performSyntheticReviewedException(i, p, "device-tools",
                                              "Synthetic expiry", pass));
    QTRY_VERIFY_WITH_TIMEOUT(p.offline(), 3000);
    QVERIFY(!p.permit("device-tools"));
    QVERIFY(performSyntheticReviewedException(i, p, "device-tools",
                                              "Synthetic logout", pass));
    i.logout();
    QVERIFY(p.offline());
    QVERIFY(i.login("request-admin", pass));
    QVERIFY(p.offline());
  }
  void reviewerDisableInvalidatesApprovalAndActiveUse() {
    QTemporaryDir t;
    IdentityService i(t.filePath("identity"), true);
    setup(i);
    GovernmentOperationPolicy p(&i);
    const auto id = approve(i, p);
    QVERIFY(!id.isEmpty());
    QString reviewer;
    for (const auto &v : i.users())
      if (v.toMap().value("username") == "reviewer")
        reviewer = v.toMap().value("id").toString();
    QVERIFY(!reviewer.isEmpty());
    QVERIFY(i.setEnabled(reviewer, false));
    QVERIFY(!p.activateException(id));
    QVERIFY(i.setEnabled(reviewer, true));
    QVERIFY(p.activateException(id));
    IdentityService other(t.filePath("identity"), true);
    QVERIFY(other.login("review-admin", pass));
    QVERIFY(other.setEnabled(reviewer, false));
    QVERIFY(!p.permit("device-tools"));
    QVERIFY(p.offline());
  }
  void exportVerificationAndMutation() {
    QTemporaryDir t;
    IdentityService i(t.filePath("identity"), true);
    setup(i);
    GovernmentOperationPolicy p(&i);
    const auto id = approve(i, p);
    QVERIFY(p.activateException(id));
    QVERIFY(p.permit("device-tools"));
    p.revoke("device-tools");
    QVERIFY(!p.exportAudit(t.filePath("audit.json"), false));
    const auto a = anchor(p);
    QVERIFY(p.exportAudit(t.filePath("audit.json"), true));
    QVERIFY(!p.exportAudit(t.filePath("audit.json"), true));
    const auto original = bytes(t.filePath("audit.json"));
    QString error;
    QVERIFY2(PolicyAudit::verifyExport(
                 original, a.value("publicKey").toString(), a, error),
             qPrintable(error));
    auto doc = QJsonDocument::fromJson(original).object();
    auto events = doc.value("events").toArray();
    QVERIFY(events.size() >= 6);
    auto reordered = events;
    const QJsonValue first = reordered[0];
    reordered[0] = reordered[1];
    reordered[1] = first;
    doc.insert("events", reordered);
    QVERIFY(!PolicyAudit::verifyExport(QJsonDocument(doc).toJson(),
                                       a.value("publicKey").toString(), a,
                                       error));
    auto truncated = events;
    truncated.removeLast();
    doc.insert("events", truncated);
    QVERIFY(!PolicyAudit::verifyExport(QJsonDocument(doc).toJson(),
                                       a.value("publicKey").toString(), a,
                                       error));
    auto changed = events;
    auto e = changed[0].toObject();
    auto raw = QByteArray::fromBase64(e.value("payload").toString().toLatin1());
    raw[raw.size() / 2] ^= 1;
    e.insert("payload", QString::fromLatin1(raw.toBase64()));
    changed[0] = e;
    doc.insert("events", changed);
    QVERIFY(!PolicyAudit::verifyExport(QJsonDocument(doc).toJson(),
                                       a.value("publicKey").toString(), a,
                                       error));
    QVERIFY(
        p.verifyAudit(t.filePath("audit.json"), QString::fromUtf8(json(a))));
    QVERIFY(
        p.auditStatus().value("verification").toMap().value("valid").toBool());
    QVERIFY(i.login("investigator", pass));
    QVERIFY(!p.exportAudit(t.filePath("private.json"), true));
    QVERIFY(!QFileInfo::exists(t.filePath("private.json")));
  }
  void outsideWriterChangeFailsClosed() {
    QTemporaryDir t;
    IdentityService i(t.filePath("identity"), true);
    setup(i);
    GovernmentOperationPolicy p(&i);
    QVERIFY(performSyntheticReviewedException(i, p, "device-tools",
                                              "Synthetic tamper", pass));
    const auto path = i.policyJournalPath();
    auto raw = bytes(path);
    QVERIFY(!raw.isEmpty());
    raw[raw.size() / 2] ^= 1;
    QVERIFY(write(path, raw));
    QSignalSpy stop(&p, &GovernmentOperationPolicy::changed);
    QVERIFY(!p.permit("device-tools"));
    QVERIFY(p.offline());
    QVERIFY(!p.auditStatus().value("healthy").toBool());
    QVERIFY(stop.count() > 0);
    GovernmentOperationPolicy reopened(&i);
    QVERIFY(!reopened.auditStatus().value("healthy").toBool());
    QVERIFY(reopened
                .requestException("device-tools", "Never repair tamper", {}, 10,
                                  true)
                .isEmpty());
  }
  void competingWriterNeverOverwrites() {
    QTemporaryDir t;
    IdentityService i(t.filePath("identity"), true);
    setup(i);
    GovernmentOperationPolicy first(&i);
    GovernmentOperationPolicy stale(&i);
    QVERIFY(
        !first.requestException("device-tools", "First writer", {}, 10, true)
             .isEmpty());
    const auto before = bytes(i.policyJournalPath());
    QVERIFY(
        stale.requestException("external-links", "Stale writer", {}, 10, true)
            .isEmpty());
    QCOMPARE(bytes(i.policyJournalPath()), before);
    QVERIFY(!stale.auditStatus().value("healthy").toBool());
  }
  void crashRestartRetainsSignedClosure() {
    QTemporaryDir t;
    QProcess child;
    child.start(QCoreApplication::applicationFilePath(),
                {"--crash-fixture", t.path()});
    QVERIFY(child.waitForFinished(30000));
    QCOMPARE(child.exitCode(), 0);
    IdentityService i(t.filePath("identity"), true);
    QVERIFY(i.login("request-admin", pass));
    GovernmentOperationPolicy p(&i);
    QVERIFY(p.auditStatus().value("healthy").toBool());
    QVERIFY(p.offline());
    QCOMPARE(p.requests().size(), 1);
    QCOMPARE(p.requests()[0].toMap().value("status").toString(),
             QStringLiteral("revoked"));
    QVERIFY(p.exportAudit(t.filePath("after.json"), true));
    QString error;
    const auto retained =
        QJsonDocument::fromJson(bytes(t.filePath("anchor.json")))
            .object()
            .toVariantMap();
    QVERIFY2(PolicyAudit::verifyExport(bytes(t.filePath("after.json")),
                                       retained.value("publicKey").toString(),
                                       retained, error),
             qPrintable(error));
  }
};
int main(int argc, char **argv) {
  QCoreApplication app(argc, argv);
  const auto a = app.arguments();
  if (a.size() == 3 &&
      (a[1] == "--export-fixture" || a[1] == "--crash-fixture"))
    return fixture(a[2], a[1] == "--crash-fixture");
  if (a.size() == 3 && a[1] == "--verify-fixture") {
    const auto retained =
        PolicyAudit::parseAnchor(bytes(a[2] + "/anchor.json"));
    QString error;
    const bool ok =
        !retained.isEmpty() &&
        PolicyAudit::verifyExport(bytes(a[2] + "/audit.json"),
                                  retained.value("publicKey").toString(),
                                  retained, error);
    if (!ok)
      std::fprintf(stderr, "%s\n", qPrintable(error));
    return ok ? 0 : 8;
  }
  PolicyAuditTests tests;
  return QTest::qExec(&tests, argc, argv);
}
#include "policy_audit_tests.moc"
