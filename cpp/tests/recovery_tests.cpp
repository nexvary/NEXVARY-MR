#include "RecoveryBridge.hpp"
#include "CaseVault.hpp"
#include <QBuffer>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>

namespace {
constexpr qsizetype MiB = 1024 * 1024;
QByteArray readFile(const QString &path) {
  QFile file(path);
  return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}
bool writeFile(const QString &path, const QByteArray &bytes) {
  QFile file(path);
  return file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size();
}
QString sha256(const QByteArray &bytes) {
  return QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex());
}
bool createFixtureCase(RecoveryBridge &bridge) {
  return bridge.createCase(QStringLiteral("Synthetic recovery case"), QStringLiteral("LAB-OWNER-ONLY"),
                           QStringLiteral("NEXVARY Lab"), QStringLiteral("Synthetic device"),
                           QStringLiteral("unknown"), QStringLiteral("deleted"), QStringLiteral("unknown"), true);
}
QJsonObject decodeDocument(const QString &path, QString &error) {
  const auto bytes = mr::recovery::openCaseBytes(readFile(path), true, error);
  if (!error.isEmpty()) return {};
  return QJsonDocument::fromJson(bytes).object();
}
bool writeDocument(const QString &path, const QJsonObject &document, QString &error) {
  const auto bytes = mr::recovery::sealCase(QJsonDocument(document).toJson(QJsonDocument::Compact), true, error);
  return !bytes.isEmpty() && writeFile(path, bytes);
}
QJsonObject agentReport(const QString &state = QStringLiteral("shell"), int uid = 2000) {
  return {{"schema", "nexvary-recovery-agent/v1"}, {"generatedAtUtc", "2026-10-16T00:00:00Z"},
          {"consent", QJsonObject{{"confirmed", true}, {"scope", "device-metadata"}}},
          {"device", QJsonObject{{"manufacturer", "NEXVARY Lab"}, {"model", "Synthetic phone"},
                                  {"sdkInt", 35}, {"release", "15"}, {"securityPatch", "2026-10-01"}}},
          {"shizuku", QJsonObject{{"state", state}, {"uid", uid < 0 ? QJsonValue(QJsonValue::Null) : QJsonValue(uid)}}},
          {"storage", QJsonArray()}, {"probes", QJsonArray()}, {"selectedFile", QJsonValue(QJsonValue::Null)},
          {"audit", QJsonArray{QStringLiteral("Owner authorized synthetic metadata collection")}}};
}
QJsonObject evidenceRecord(const QByteArray &bytes, const QString &name) {
  return {{"name", name}, {"kind", "logical-copy"}, {"size", static_cast<double>(bytes.size())},
          {"sha256", sha256(bytes)}, {"source", "synthetic-fixture-only"}, {"sourceSha256", sha256(bytes)},
          {"offset", "0"}, {"collectedAtUtc", "2026-10-16T00:00:00Z"},
          {"payload", QString::fromLatin1(bytes.toBase64())}};
}
bool hasAudit(const RecoveryBridge &bridge, const QString &operation) {
  for (const auto &entry : bridge.audit())
    if (entry.toMap().value("operation").toString() == operation) return true;
  return false;
}
}

class RecoveryTests final : public QObject {
  Q_OBJECT
private slots:
  void vaultRoundtripAndTamperResistance() {
    const QByteArray plain("SYNTHETIC-LAB-MARKER: no customer data\n{\"payload\":\"round-trip\"}");
    QString error;
    const auto sealed = mr::recovery::sealCase(plain, true, error);
    QVERIFY2(error.isEmpty(), qPrintable(error)); QVERIFY(!sealed.isEmpty());
    QCOMPARE(mr::recovery::openCaseBytes(sealed, true, error), plain);
    QVERIFY2(error.isEmpty(), qPrintable(error));
#ifdef Q_OS_WIN
    QVERIFY(mr::recovery::vaultAvailable());
    QVERIFY(sealed.startsWith("NMR-DPAPI-1\n"));
    QVERIFY(!sealed.contains(plain)); QVERIFY(!sealed.contains("SYNTHETIC-LAB-MARKER"));
    QCOMPARE(mr::recovery::openCaseBytes(sealed, false, error), plain);
#else
    QVERIFY(!mr::recovery::vaultAvailable());
    QVERIFY(sealed.startsWith("NMR-SYNTHETIC-ONLY-1\n"));
    QVERIFY(sealed.contains(plain)); // Deliberate fixture mode is never described as encrypted.
    QVERIFY(mr::recovery::openCaseBytes(sealed, false, error).isEmpty()); QVERIFY(!error.isEmpty());
    QVERIFY(mr::recovery::sealCase(plain, false, error).isEmpty()); QVERIFY(!error.isEmpty());
#endif
    auto tampered = sealed; tampered[tampered.size() / 2] = static_cast<char>(tampered.at(tampered.size() / 2) ^ 1);
    QVERIFY(mr::recovery::openCaseBytes(tampered, true, error).isEmpty()); QVERIFY(!error.isEmpty());
    auto wrongHeader = sealed; wrongHeader[0] = 'X';
    QVERIFY(mr::recovery::openCaseBytes(wrongHeader, true, error).isEmpty()); QVERIFY(!error.isEmpty());
    QVERIFY(mr::recovery::openCaseBytes(sealed.left(12), true, error).isEmpty()); QVERIFY(!error.isEmpty());
    QVERIFY(mr::recovery::openCaseBytes({}, true, error).isEmpty()); QVERIFY(!error.isEmpty());
    QVERIFY(mr::recovery::sealCase({}, true, error).isEmpty()); QVERIFY(!error.isEmpty());
  }

  void consentValidationAndProductionPlatformBoundary() {
    QTemporaryDir temp; QVERIFY(temp.isValid());
    RecoveryBridge bridge(temp.filePath("synthetic cases"), true);
    QVERIFY(!bridge.active()); QVERIFY(bridge.vaultReady());
    QVERIFY(!bridge.importVisibleFile(temp.filePath("absent"), true));
    QVERIFY(!bridge.importAgent(temp.filePath("absent.json"), true));
    QVERIFY(!bridge.scanImage(temp.filePath("absent.img"), true));
    QVERIFY(!bridge.createCase("Case", "Owner", "Brand", "Model", "15", "deleted", "unknown", false));
    QVERIFY(!bridge.createCase(" ", "Owner", "Brand", "Model", "15", "deleted", "unknown", true));
    QVERIFY(!bridge.createCase("Case", "", "Brand", "Model", "15", "deleted", "unknown", true));
    QVERIFY(!bridge.createCase("Case", "Owner", "Brand", "Model", "15", "invented", "unknown", true));
    QVERIFY(!bridge.createCase("Case", "Owner", "Brand", "Model", "15", "deleted", "invented", true));
    QVERIFY(!bridge.createCase(QString(161, 'x'), "Owner", "Brand", "Model", "15", "deleted", "unknown", true));
    QVERIFY(!bridge.createCase(QString("Case") + QChar::Null, "Owner", "Brand", "Model", "15", "deleted", "unknown", true));
    QVERIFY(!bridge.active()); QVERIFY(bridge.casePath().isEmpty());
    QVERIFY(!QFileInfo::exists(temp.filePath("synthetic cases")));
#ifndef Q_OS_WIN
    RecoveryBridge production(temp.filePath("production cases"));
    QVERIFY(!production.vaultReady()); QVERIFY(!createFixtureCase(production));
    QVERIFY(!production.active()); QVERIFY(!QFileInfo::exists(temp.filePath("production cases")));
    QVERIFY(production.error().contains("Windows"));
#else
    RecoveryBridge production(temp.filePath("production cases"));
    QVERIFY(production.vaultReady()); QVERIFY2(createFixtureCase(production), qPrintable(production.error()));
    QVERIFY(readFile(production.casePath()).startsWith("NMR-DPAPI-1\n"));
#endif
    QVERIFY2(createFixtureCase(bridge), qPrintable(bridge.error()));
    QVERIFY(bridge.active()); QVERIFY(QFileInfo::exists(bridge.casePath()));
    QVERIFY(hasAudit(bridge, "case-created"));
    QVERIFY(bridge.assess("factory-reset", "fbe", true, true, false).contains("lost keys"));
  }

  void visibleFilePersistenceExportAndDeletion() {
    QTemporaryDir temp; QVERIFY(temp.isValid());
    const auto source = temp.filePath(QStringLiteral("ملف lab evidence.bin"));
    const QByteArray original = QByteArray("SYNTHETIC-VISIBLE-BYTES") + QByteArray(1, '\0') + "with-binary-tail";
    QVERIFY(writeFile(source, original));
    QString casePath;
    {
      RecoveryBridge bridge(temp.filePath("cases"), true); QVERIFY(createFixtureCase(bridge)); casePath = bridge.casePath();
      const auto before = readFile(casePath);
      QVERIFY(!bridge.importVisibleFile(source, false)); QVERIFY(!bridge.busy()); QCOMPARE(readFile(casePath), before);
      QVERIFY(!bridge.importVisibleFile(casePath, true));
      QVERIFY(!bridge.importVisibleFile("https://example.invalid/evidence", true));
      QVERIFY(!bridge.importVisibleFile(QString("bad") + QChar::Null + ".bin", true));
      QVERIFY(bridge.importVisibleFile(QUrl::fromLocalFile(source).toString(), true));
      QTRY_COMPARE_WITH_TIMEOUT(bridge.busy(), false, 30000);
      QVERIFY2(bridge.error().isEmpty(), qPrintable(bridge.error())); QCOMPARE(bridge.evidence().size(), 1);
      const auto item = bridge.evidence().first().toMap();
      QCOMPARE(item.value("sha256").toString(), sha256(original)); QCOMPARE(item.value("sourceSha256").toString(), sha256(original));
      QCOMPARE(item.value("kind").toString(), QStringLiteral("logical-copy"));
      QCOMPARE(item.value("size").toLongLong(), qint64(original.size())); QVERIFY(!item.contains("payload"));
      QCOMPARE(readFile(source), original); QVERIFY(hasAudit(bridge, "logical-file-copy"));
      QString error; const auto doc = decodeDocument(casePath, error); QVERIFY2(error.isEmpty(), qPrintable(error));
      QCOMPARE(QByteArray::fromBase64(doc.value("evidence").toArray().first().toObject().value("payload").toString().toLatin1()), original);
#ifdef Q_OS_WIN
      const auto disk = readFile(casePath); QVERIFY(!disk.contains(original)); QVERIFY(!disk.contains(original.toBase64()));
      QVERIFY(!disk.contains("SYNTHETIC-VISIBLE-BYTES"));
#endif
    }
    RecoveryBridge reopened(temp.filePath("cases"), true);
    QVERIFY(reopened.openCase(QUrl::fromLocalFile(casePath).toString()));
    QTRY_COMPARE_WITH_TIMEOUT(reopened.busy(), false, 30000);
    QVERIFY2(reopened.error().isEmpty(), qPrintable(reopened.error())); QCOMPARE(reopened.casePath(), casePath);
    QCOMPARE(reopened.evidence().size(), 1); QVERIFY(hasAudit(reopened, "case-opened"));
    const auto destination = temp.filePath("new evidence export.bin");
    QVERIFY(!reopened.exportEvidence(0, destination, false)); QVERIFY(!QFileInfo::exists(destination));
    QVERIFY(!reopened.exportEvidence(-1, destination, true)); QVERIFY(!reopened.exportEvidence(1, destination, true));
    QVERIFY(!reopened.exportEvidence(0, source, true)); QVERIFY(!reopened.exportEvidence(0, casePath, true));
    QVERIFY(!reopened.exportReport(source, true)); QVERIFY(!reopened.exportReport(casePath, true));
    const auto existing = temp.filePath("existing.bin"); QVERIFY(writeFile(existing, "keep this existing file"));
    QVERIFY(!reopened.exportEvidence(0, existing, true)); QCOMPARE(readFile(existing), QByteArray("keep this existing file"));
    QVERIFY2(reopened.exportEvidence(0, destination, true), qPrintable(reopened.error()));
    QCOMPARE(readFile(destination), original); QCOMPARE(sha256(readFile(destination)), sha256(original));
    QVERIFY(!reopened.exportEvidence(0, destination, true)); QCOMPARE(readFile(destination), original);
    const auto reportPath = temp.filePath("report.json");
    QVERIFY(!reopened.exportReport(reportPath, false)); QVERIFY(!QFileInfo::exists(reportPath));
    QVERIFY2(reopened.exportReport(reportPath, true), qPrintable(reopened.error()));
    const auto report = QJsonDocument::fromJson(readFile(reportPath)).object();
    QCOMPARE(report.value("evidence").toArray().size(), 1);
    QVERIFY(!report.value("evidence").toArray().first().toObject().contains("payload"));
    QVERIFY(report.value("protection").toString().contains("PLAINTEXT"));
    QVERIFY(hasAudit(reopened, "evidence-export")); QVERIFY(hasAudit(reopened, "report-export"));
    QVERIFY(!reopened.deleteCase(false)); QVERIFY(reopened.active()); QVERIFY(QFileInfo::exists(casePath));
    QVERIFY(reopened.deleteCase(true)); QVERIFY(!reopened.active()); QVERIFY(reopened.casePath().isEmpty());
    QVERIFY(!QFileInfo::exists(casePath)); QCOMPARE(readFile(source), original); QCOMPARE(readFile(destination), original);
    QVERIFY(QFileInfo::exists(reportPath)); QVERIFY(!reopened.deleteCase(true));
  }

  void failedCaseOpenCannotRetargetActiveVaultOrOverwriteInput() {
    QTemporaryDir temp; RecoveryBridge bridge(temp.filePath("cases"), true); QVERIFY(createFixtureCase(bridge));
    const auto activePath = bridge.casePath(); const auto title = bridge.caseTitle(); const auto activeBytes = readFile(activePath);
    const QByteArray original("This is a source image, never a writable case destination.");
    const auto badPath = temp.filePath("not a case.img"); QVERIFY(writeFile(badPath, original));
    QVERIFY(bridge.openCase(badPath)); QTRY_COMPARE_WITH_TIMEOUT(bridge.busy(), false, 30000);
    QVERIFY(!bridge.error().isEmpty()); QVERIFY(bridge.active()); QCOMPARE(bridge.casePath(), activePath);
    QCOMPARE(bridge.caseTitle(), title); QCOMPARE(readFile(activePath), activeBytes); QCOMPARE(readFile(badPath), original);
    for (const auto &invalidPlain : QList<QByteArray>{QByteArray("invalid JSON"), QByteArray("{\"schema\":\"not-a-case\"}")}) {
      QString error; const auto encryptedInvalid = mr::recovery::sealCase(invalidPlain, true, error);
      QVERIFY2(!encryptedInvalid.isEmpty(), qPrintable(error));
      const auto invalidVault = temp.filePath("invalid-content.nmrcase"); QVERIFY(writeFile(invalidVault, encryptedInvalid));
      QVERIFY(bridge.openCase(invalidVault)); QTRY_COMPARE_WITH_TIMEOUT(bridge.busy(), false, 30000);
      QVERIFY(!bridge.error().isEmpty()); QCOMPARE(bridge.casePath(), activePath); QCOMPARE(bridge.caseTitle(), title);
      QCOMPARE(readFile(invalidVault), encryptedInvalid); QCOMPARE(readFile(activePath), activeBytes);
    }
    const auto missing = temp.filePath("absent.nmrcase");
    QVERIFY(bridge.openCase(missing)); QTRY_COMPARE_WITH_TIMEOUT(bridge.busy(), false, 30000);
    QVERIFY(!bridge.error().isEmpty()); QCOMPARE(bridge.casePath(), activePath); QVERIFY(!QFileInfo::exists(missing));
    const auto source = temp.filePath("new original.bin"); QVERIFY(writeFile(source, "authorized fixture"));
    QVERIFY(bridge.importVisibleFile(source, true)); QTRY_COMPARE_WITH_TIMEOUT(bridge.busy(), false, 30000);
    QVERIFY2(bridge.error().isEmpty(), qPrintable(bridge.error())); QCOMPARE(bridge.casePath(), activePath);
    QCOMPARE(bridge.evidence().size(), 1); QCOMPARE(readFile(badPath), original);
  }

  void agentValidStates_data() {
    QTest::addColumn<QString>("state"); QTest::addColumn<int>("uid");
    QTest::newRow("unavailable") << QStringLiteral("unavailable") << -1;
    QTest::newRow("permission-required") << QStringLiteral("permission-required") << 2000;
    QTest::newRow("denied") << QStringLiteral("denied") << 2000;
    QTest::newRow("shell") << QStringLiteral("shell") << 2000;
    QTest::newRow("root-refused-with-no-exported-root-uid") << QStringLiteral("unsupported-root") << -1;
  }
  void agentValidStates() {
    QFETCH(QString, state); QFETCH(int, uid);
    QTemporaryDir temp; RecoveryBridge bridge(temp.filePath("cases"), true); QVERIFY(createFixtureCase(bridge));
    const auto raw = QJsonDocument(agentReport(state, uid)).toJson(); const auto path = temp.filePath("agent.json");
    QVERIFY(writeFile(path, raw)); QVERIFY(!bridge.importAgent(path, false)); QVERIFY(bridge.evidence().isEmpty());
    QVERIFY(bridge.importAgent(path, true)); QTRY_COMPARE_WITH_TIMEOUT(bridge.busy(), false, 30000);
    QVERIFY2(bridge.error().isEmpty(), qPrintable(bridge.error())); QCOMPARE(bridge.evidence().size(), 1);
    QCOMPARE(bridge.device().value("model").toString(), QStringLiteral("Synthetic phone"));
    QVERIFY(bridge.device().value("identitySource").toString().contains("unverified"));
    QCOMPARE(bridge.evidence().first().toMap().value("sha256").toString(), sha256(raw));
    QCOMPARE(bridge.evidence().first().toMap().value("kind").toString(), QStringLiteral("agent-metadata"));
    QCOMPARE(readFile(path), raw); QVERIFY(hasAudit(bridge, "agent-report-import"));
  }

  void agentInvalidProtocol_data() {
    QTest::addColumn<QByteArray>("report");
    auto row = [](const char *name, const QJsonObject &value) { QTest::newRow(name) << QJsonDocument(value).toJson(); };
    auto report = agentReport(); report.insert("schema", "invented"); row("wrong-schema", report);
    report = agentReport(); report.insert("consent", QJsonObject{{"confirmed", false}, {"scope", "device-metadata"}}); row("missing-consent", report);
    report = agentReport(); report.insert("consent", QJsonObject{{"confirmed", true}, {"scope", "protected-filesystem"}}); row("wrong-consent-scope", report);
    report = agentReport(); auto device = report.value("device").toObject(); device.insert("sdkInt", 25); report.insert("device", device); row("old-sdk", report);
    device.insert("sdkInt", 101); report.insert("device", device); row("invalid-sdk", report);
    device.insert("sdkInt", 35); device.insert("manufacturer", " "); report.insert("device", device); row("empty-identity", report);
    row("invented-state", agentReport("invented", 2000)); row("shell-is-not-root", agentReport("shell", 0));
    row("shell-needs-uid", agentReport("shell", -1)); row("untrusted-uid", agentReport("denied", 1234));
    row("root-refusal-cannot-be-shell", agentReport("unsupported-root", 2000));
    row("root-uid-not-exported", agentReport("unsupported-root", 0));
    row("unavailable-cannot-have-root-uid", agentReport("unavailable", 0));
    row("denied-needs-shell-identity", agentReport("denied", -1));
    QTest::newRow("invalid-json") << QByteArray("not JSON");
    QTest::newRow("over-report-bound") << QByteArray(128 * 1024 + 1, 'x');
  }
  void agentInvalidProtocol() {
    QFETCH(QByteArray, report);
    QTemporaryDir temp; RecoveryBridge bridge(temp.filePath("cases"), true); QVERIFY(createFixtureCase(bridge));
    const auto beforeDevice = bridge.device(); const auto path = temp.filePath("invalid-agent.json"); QVERIFY(writeFile(path, report));
    QVERIFY(bridge.importAgent(path, true)); QTRY_COMPARE_WITH_TIMEOUT(bridge.busy(), false, 30000);
    QVERIFY(!bridge.error().isEmpty()); QVERIFY(bridge.evidence().isEmpty()); QCOMPARE(bridge.device(), beforeDevice);
    QCOMPARE(readFile(path), report); QVERIFY(hasAudit(bridge, "operation-failed"));
  }

  void missingInputFailureIsPersistedInAudit() {
    QTemporaryDir temp; QString path;
    {
      RecoveryBridge bridge(temp.filePath("cases"), true); QVERIFY(createFixtureCase(bridge)); path = bridge.casePath();
      QVERIFY(bridge.importVisibleFile(temp.filePath("missing.bin"), true)); QTRY_COMPARE_WITH_TIMEOUT(bridge.busy(), false, 30000);
      QVERIFY(!bridge.error().isEmpty()); QVERIFY(bridge.evidence().isEmpty()); QVERIFY(hasAudit(bridge, "operation-failed"));
    }
    RecoveryBridge reopened(temp.filePath("cases"), true); QVERIFY(reopened.openCase(path));
    QTRY_COMPARE_WITH_TIMEOUT(reopened.busy(), false, 30000);
    QVERIFY2(reopened.error().isEmpty(), qPrintable(reopened.error())); QVERIFY(hasAudit(reopened, "operation-failed"));
  }

  void auditPersistenceFailureNeverClaimsCommittedImportOrExport() {
    QTemporaryDir temp; RecoveryBridge bridge(temp.filePath("cases"), true); QVERIFY(createFixtureCase(bridge));
    const auto source = temp.filePath("evidence.bin"); const QByteArray bytes("Synthetic audit failure fixture"); QVERIFY(writeFile(source, bytes));
    QVERIFY(bridge.importVisibleFile(source, true)); QTRY_COMPARE_WITH_TIMEOUT(bridge.busy(), false, 30000);
    QVERIFY2(bridge.error().isEmpty(), qPrintable(bridge.error())); const auto path = bridge.casePath();
    const auto savedVault = readFile(path); const auto auditCount = bridge.audit().size();
    QVERIFY(QFile::remove(path)); QVERIFY(QDir().mkdir(path)); // A directory deterministically blocks replacement on both platforms.
    QVERIFY(bridge.importVisibleFile(source, true)); QTRY_COMPARE_WITH_TIMEOUT(bridge.busy(), false, 30000);
    QVERIFY(bridge.error().contains("Audit update failed")); QCOMPARE(bridge.casePath(), path);
    QCOMPARE(bridge.evidence().size(), 1); QCOMPARE(bridge.audit().size(), auditCount); QCOMPARE(readFile(source), bytes);
    const auto exported = temp.filePath("export despite audit failure.bin");
    QVERIFY(!bridge.exportEvidence(0, exported, true));
    QVERIFY(bridge.error().contains("audit update failed")); QCOMPARE(readFile(exported), bytes);
    QCOMPARE(bridge.audit().size(), auditCount);
    QVERIFY(QDir().rmdir(path)); QVERIFY(writeFile(path, savedVault));
    QVERIFY(bridge.openCase(path)); QTRY_COMPARE_WITH_TIMEOUT(bridge.busy(), false, 30000);
    QVERIFY2(bridge.error().isEmpty(), qPrintable(bridge.error())); QCOMPARE(bridge.evidence().size(), 1);
  }

  void fileLimitAndCaseEvidenceCount() {
    QTemporaryDir temp; RecoveryBridge bridge(temp.filePath("cases"), true); QVERIFY(createFixtureCase(bridge));
    const auto source = temp.filePath("boundary.bin"); const QByteArray exact(8 * MiB, 'L'); QVERIFY(writeFile(source, exact));
    QVERIFY(bridge.importVisibleFile(source, true)); QTRY_COMPARE_WITH_TIMEOUT(bridge.busy(), false, 30000);
    QVERIFY2(bridge.error().isEmpty(), qPrintable(bridge.error())); QCOMPARE(bridge.evidence().size(), 1);
    QCOMPARE(bridge.evidence().first().toMap().value("size").toLongLong(), qint64(8 * MiB));
    const auto oversized = temp.filePath("too-large.bin"); QVERIFY(writeFile(oversized, exact + 'x'));
    QVERIFY(bridge.importVisibleFile(oversized, true)); QTRY_COMPARE_WITH_TIMEOUT(bridge.busy(), false, 30000);
    QVERIFY(!bridge.error().isEmpty()); QCOMPARE(bridge.evidence().size(), 1); QCOMPARE(readFile(source), exact);
    QString error; auto document = decodeDocument(bridge.casePath(), error); QVERIFY2(error.isEmpty(), qPrintable(error));
    QJsonArray entries; for (int i = 0; i < 100; ++i) entries.append(evidenceRecord({}, QStringLiteral("empty fixture %1").arg(i)));
    document.insert("evidence", entries); const auto hundred = temp.filePath("hundred.nmrcase"); QVERIFY(writeDocument(hundred, document, error));
    QVERIFY(bridge.openCase(hundred)); QTRY_COMPARE_WITH_TIMEOUT(bridge.busy(), false, 30000);
    QVERIFY2(bridge.error().isEmpty(), qPrintable(bridge.error())); QCOMPARE(bridge.evidence().size(), 100);
    const auto hundredBefore = readFile(hundred); entries.append(evidenceRecord({}, "excess")); document.insert("evidence", entries);
    const auto hundredOne = temp.filePath("hundred-one.nmrcase"); QVERIFY(writeDocument(hundredOne, document, error));
    const auto rejectedBefore = readFile(hundredOne); QVERIFY(bridge.openCase(hundredOne)); QTRY_COMPARE_WITH_TIMEOUT(bridge.busy(), false, 30000);
    QVERIFY(!bridge.error().isEmpty()); QCOMPARE(bridge.casePath(), hundred); QCOMPARE(bridge.evidence().size(), 100);
    QCOMPARE(readFile(hundred), hundredBefore); QCOMPARE(readFile(hundredOne), rejectedBefore);
  }

  void aggregateCaseSizeBoundaryAndEmbeddedHashValidation() {
    QTemporaryDir temp; RecoveryBridge bridge(temp.filePath("cases"), true); QVERIFY(createFixtureCase(bridge));
    QString error; auto document = decodeDocument(bridge.casePath(), error); QVERIFY2(error.isEmpty(), qPrintable(error));
    const auto record = evidenceRecord(QByteArray(8 * MiB, 'B'), "8MiB synthetic fixture");
    QJsonArray entries; for (int i = 0; i < 8; ++i) entries.append(record);
    document.insert("evidence", entries); const auto exact = temp.filePath("64MiB.nmrcase"); QVERIFY(writeDocument(exact, document, error));
    QVERIFY(bridge.openCase(exact)); QTRY_COMPARE_WITH_TIMEOUT(bridge.busy(), false, 60000);
    QVERIFY2(bridge.error().isEmpty(), qPrintable(bridge.error())); QCOMPARE(bridge.evidence().size(), 8);
    entries.append(evidenceRecord("x", "one byte over aggregate limit")); document.insert("evidence", entries);
    const auto over = temp.filePath("64MiB-plus-one.nmrcase"); QVERIFY(writeDocument(over, document, error));
    QVERIFY(bridge.openCase(over)); QTRY_COMPARE_WITH_TIMEOUT(bridge.busy(), false, 60000);
    QVERIFY(!bridge.error().isEmpty()); QCOMPARE(bridge.casePath(), exact); QCOMPARE(bridge.evidence().size(), 8);
    auto badHash = evidenceRecord("intact fixture", "incorrect digest"); badHash.insert("sha256", QString(64, '0'));
    document.insert("evidence", QJsonArray{badHash}); const auto mismatch = temp.filePath("hash-mismatch.nmrcase");
    QVERIFY(writeDocument(mismatch, document, error)); QVERIFY(bridge.openCase(mismatch)); QTRY_COMPARE_WITH_TIMEOUT(bridge.busy(), false, 30000);
    QVERIFY(!bridge.error().isEmpty()); QCOMPARE(bridge.casePath(), exact); QCOMPARE(bridge.evidence().size(), 8);
  }

  void nativeImageCandidatesReachVaultWithProvenance() {
    QTemporaryDir temp; RecoveryBridge bridge(temp.filePath("cases"), true); QVERIFY(createFixtureCase(bridge));
    QImage image(8, 8, QImage::Format_RGB32); image.fill(Qt::blue); QByteArray png; QBuffer buffer(&png);
    QVERIFY(buffer.open(QIODevice::WriteOnly)); QVERIFY(image.save(&buffer, "PNG")); buffer.close();
    const auto raw = QByteArray(37, 'x') + png + QByteArray(19, 'y'); const auto source = temp.filePath("original.img"); QVERIFY(writeFile(source, raw));
    QVERIFY(!bridge.scanImage(source, false)); QVERIFY(bridge.evidence().isEmpty());
    QVERIFY(bridge.scanImage(source, true)); QTRY_COMPARE_WITH_TIMEOUT(bridge.busy(), false, 30000);
    QVERIFY2(bridge.error().isEmpty(), qPrintable(bridge.error())); QCOMPARE(bridge.evidence().size(), 1);
    const auto candidate = bridge.evidence().first().toMap(); QCOMPARE(candidate.value("kind").toString(), QStringLiteral("carved-candidate"));
    QCOMPARE(candidate.value("offset").toString(), QStringLiteral("37")); QCOMPARE(candidate.value("sha256").toString(), sha256(png));
    QCOMPARE(candidate.value("sourceSha256").toString(), sha256(raw)); QCOMPARE(readFile(source), raw);
    const auto exported = temp.filePath("candidate.png"); QVERIFY(bridge.exportEvidence(0, exported, true)); QCOMPARE(readFile(exported), png);
    QString error; const auto doc = decodeDocument(bridge.casePath(), error); QVERIFY2(error.isEmpty(), qPrintable(error));
    QVERIFY(doc.value("sources").toArray().first().toObject().value("verifiedUnchanged").toBool());
    QVERIFY(hasAudit(bridge, "image-analysis"));
  }
};
QTEST_GUILESS_MAIN(RecoveryTests)
#include "recovery_tests.moc"
