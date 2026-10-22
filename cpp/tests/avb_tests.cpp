#include "AvbMetadata.hpp"
#include "FirmwareBridge.hpp"
#include <QCryptographicHash>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>
#include <QtEndian>
#include <limits>

class AvbTests : public QObject {
  Q_OBJECT
  static void put32(QByteArray &b, qsizetype at, quint32 value) { qToBigEndian(value, b.data() + at); }
  static void put64(QByteArray &b, qsizetype at, quint64 value) { qToBigEndian(value, b.data() + at); }
  static QByteArray fixture() {
    QByteArray b(320, '\0');
    b.replace(0, 4, "AVB0"); put32(b, 4, 1); put32(b, 8, 2);
    put64(b, 20, 64); put64(b, 104, 16); put64(b, 112, 9007199254740993ULL);
    put32(b, 124, 7); b.replace(128, 11, "fixture-1.2");
    put64(b, 256, 2); // Framing only: deliberately no hash descriptor body.
    return b;
  }
private slots:
  void officialToolReferenceImages() {
    const QString directory = QString::fromUtf8(MR_AVB_SAMPLE_DIR);
    QFile manifest(directory + QStringLiteral("/expected.json"));
    QVERIFY(manifest.open(QIODevice::ReadOnly));
    const auto expected = QJsonDocument::fromJson(manifest.readAll()).object();
    QVERIFY(expected["fixtureOnly"].toBool());
    QCOMPARE(expected["toolVersion"].toString(), QStringLiteral("avbtool 1.3.0"));
    const auto images = expected["images"].toArray(); QCOMPARE(images.size(), 3);
    for (const auto &entry : images) {
      const auto row = entry.toObject();
      QFile file(directory + '/' + row["file"].toString()); QVERIFY(file.open(QIODevice::ReadOnly));
      const auto bytes = file.readAll(); const auto report = AvbMetadata::inspect(bytes);
      QCOMPARE(report["status"].toString(), QStringLiteral("metadata-only"));
      QCOMPARE(report["sha256"].toString(), row["sha256"].toString());
      QCOMPARE(report["declaredRollbackIndex"].toString(), row["rollbackIndex"].toString());
      QCOMPARE(report["declaredRollbackLocation"].toInt(), row["rollbackLocation"].toInt());
      QCOMPARE(report["declaredFlags"].toInt(), row["flags"].toInt());
      QCOMPARE(report["declaredAlgorithmId"].toInt(), row["algorithmId"].toInt());
      QCOMPARE(report["requiredVersion"].toString(), row["requiredVersion"].toString());
      QCOMPARE(report["descriptorEnvelopes"].toArray().size(), row["descriptorCount"].toInt());
      // Both valid and corrupted signatures are deliberately UNVERIFIED here.
      QVERIFY(!report["signatureVerified"].toBool(true));
      QVERIFY(!report["oemTrustVerified"].toBool(true));
      QVERIFY(!report["deviceCompatibilityVerified"].toBool(true));
    }
  }
  void declarationsNeverBecomeTrust() {
    const auto b = fixture(); const auto r = AvbMetadata::inspect(b);
    QCOMPARE(r["status"].toString(), QStringLiteral("metadata-only"));
    QCOMPARE(r["requiredVersion"].toString(), QStringLiteral("1.2"));
    QCOMPARE(r["declaredRollbackIndex"].toString(), QStringLiteral("9007199254740993"));
    QCOMPARE(r["sha256"].toString(), QString::fromLatin1(QCryptographicHash::hash(b, QCryptographicHash::Sha256).toHex()));
    QCOMPARE(r["descriptorEnvelopes"].toArray().size(), 1);
    for (const auto *field : {"signatureVerified", "oemTrustVerified", "deviceCompatibilityVerified", "rollbackSafetyVerified"}) QVERIFY(!r[field].toBool(true));
    auto fakeSigned = b; put32(fakeSigned, 28, 1);
    QVERIFY(!AvbMetadata::inspect(fakeSigned)["signatureVerified"].toBool(true));
  }
  void rejectsHostileLengths_data() {
    QTest::addColumn<int>("offset"); QTest::addColumn<quint64>("value"); QTest::addColumn<QString>("error");
    QTest::newRow("auth-overflow") << 12 << (std::numeric_limits<quint64>::max() - 63) << QStringLiteral("truncated-blocks");
    QTest::newRow("aux-overflow") << 20 << (std::numeric_limits<quint64>::max() - 63) << QStringLiteral("truncated-blocks");
    QTest::newRow("unaligned") << 20 << quint64(65) << QStringLiteral("unaligned-blocks");
    QTest::newRow("hash-offset") << 32 << std::numeric_limits<quint64>::max() << QStringLiteral("region-out-of-bounds");
    QTest::newRow("signature-size") << 56 << quint64(1) << QStringLiteral("region-out-of-bounds");
    QTest::newRow("key-size") << 72 << quint64(65) << QStringLiteral("region-out-of-bounds");
    QTest::newRow("keymeta-offset") << 80 << std::numeric_limits<quint64>::max() << QStringLiteral("region-out-of-bounds");
    QTest::newRow("descriptors-offset") << 96 << quint64(64) << QStringLiteral("region-out-of-bounds");
    QTest::newRow("partial-descriptor") << 104 << quint64(8) << QStringLiteral("truncated-descriptor");
    QTest::newRow("descriptor-overflow") << 264 << std::numeric_limits<quint64>::max() << QStringLiteral("invalid-descriptor-length");
    QTest::newRow("descriptor-unaligned") << 264 << quint64(1) << QStringLiteral("invalid-descriptor-length");
    QTest::newRow("descriptor-outside") << 264 << quint64(16) << QStringLiteral("invalid-descriptor-length");
  }
  void rejectsHostileLengths() {
    QFETCH(int, offset); QFETCH(quint64, value); QFETCH(QString, error);
    auto b = fixture(); put64(b, offset, value);
    const auto r = AvbMetadata::inspect(b); QCOMPARE(r["status"].toString(), QStringLiteral("rejected")); QCOMPARE(r["error"].toString(), error);
    QVERIFY(!r.contains("sha256"));
  }
  void truncationAndUnsupportedFormats() {
    const auto b = fixture();
    for (qsizetype n = 0; n < b.size(); ++n) QCOMPARE(AvbMetadata::inspect(b.left(n))["status"].toString(), QStringLiteral("rejected"));
    auto other = b; other.replace(0, 4, "PK\003\004"); QCOMPARE(AvbMetadata::inspect(other)["error"].toString(), QStringLiteral("raw-vbmeta-required"));
    other = b; put32(other, 4, 2); QCOMPARE(AvbMetadata::inspect(other)["error"].toString(), QStringLiteral("unsupported-header-major"));
    other = b; other[175] = 'x'; QCOMPARE(AvbMetadata::inspect(other)["error"].toString(), QStringLiteral("unterminated-release"));
    other = b; other[176] = 'x'; QCOMPARE(AvbMetadata::inspect(other)["error"].toString(), QStringLiteral("nonzero-reserved-header"));
    QCOMPARE(AvbMetadata::inspect(QByteArray(AvbMetadata::maxImageBytes + 1, '\0'))["error"].toString(), QStringLiteral("image-size-limit"));
  }
  void flagsAndUntrustedText() {
    auto b = fixture(); put32(b, 120, 7); b[128] = '\n';
    const auto r = AvbMetadata::inspect(b); const auto warnings = r["warnings"].toArray();
    QVERIFY(warnings.contains(QStringLiteral("hashtree-disabled-flag-declared")));
    QVERIFY(warnings.contains(QStringLiteral("verification-disabled-flag-declared")));
    QVERIFY(warnings.contains(QStringLiteral("unknown-flags-declared")));
    QVERIFY(!r["declaredRelease"].toString().contains('\n'));
  }
  void overlapsAndCountBound() {
    auto b = fixture(); put64(b, 72, 16);
    QCOMPARE(AvbMetadata::inspect(b)["error"].toString(), QStringLiteral("overlapping-regions"));
    b = QByteArray(256 + 65600, '\0'); b.replace(0, 4, "AVB0"); put32(b, 4, 1); put64(b, 20, 65600); put64(b, 104, 4097 * 16);
    QCOMPARE(AvbMetadata::inspect(b)["error"].toString(), QStringLiteral("descriptor-count-limit"));
  }
  void asyncBridgeAndSourcePreservation() {
    QTemporaryDir dir; QVERIFY(dir.isValid()); const auto source = dir.filePath("private-client.img");
    const auto bytes = fixture(); QFile file(source); QVERIFY(file.open(QIODevice::WriteOnly)); QCOMPARE(file.write(bytes), bytes.size()); file.close();
    FirmwareBridge bridge; QSignalSpy finished(&bridge, &FirmwareBridge::inspectionFinished);
    QVERIFY(!bridge.exportAvbReport(dir.filePath("empty.json")));
    bridge.inspectAvb(QUrl::fromLocalFile(source).toString()); QVERIFY(bridge.busy()); QVERIFY(finished.wait(5000));
    QVERIFY(!bridge.compatible()); QCOMPARE(bridge.avbReport().value("status").toString(), QStringLiteral("metadata-only"));
    QVERIFY(!bridge.avbJson().contains("private-client")); QVERIFY(!bridge.avbJson().contains(dir.path()));
    QVERIFY(!bridge.exportAvbReport(source)); QVERIFY(!bridge.exportAvbReport(QUrl::fromLocalFile(source).toString()));
    const auto output = dir.filePath("report.json"); QVERIFY(bridge.exportAvbReport(output));
    QFile report(output); QVERIFY(report.open(QIODevice::ReadOnly)); QCOMPARE(report.readAll(), bridge.avbJson().toUtf8());
    QVERIFY(file.open(QIODevice::ReadOnly)); QCOMPARE(file.readAll(), bytes); file.close();
    bridge.inspectAvb(dir.filePath("missing.img")); QVERIFY(!bridge.busy()); QVERIFY(bridge.avbJson().isEmpty());
    QVERIFY(!bridge.exportAvbReport(output));
    bridge.inspectAvb(source); bridge.cancel(); QVERIFY(finished.wait(5000)); QVERIFY(bridge.avbJson().isEmpty()); QVERIFY(!bridge.compatible());
    bridge.inspectAvb(source); bridge.clearAvbReport(); QVERIFY(finished.wait(5000)); QVERIFY(bridge.avbJson().isEmpty());
  }
};
QTEST_GUILESS_MAIN(AvbTests)
#include "avb_tests.moc"
