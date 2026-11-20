#include "FirmwareBridge.hpp"
#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>

class FirmwareTests : public QObject {
  Q_OBJECT
private:
  QJsonObject manifest(const QByteArray &payload = QByteArrayLiteral("training fixture\n")) const {
    return {{QStringLiteral("schemaVersion"), 1}, {QStringLiteral("brand"), QStringLiteral("TestBrand")}, {QStringLiteral("model"), QStringLiteral("MODEL-01")}, {QStringLiteral("build"), QStringLiteral("test-build")}, {QStringLiteral("sourceUrl"), QStringLiteral("https://vendor.example/firmware")}, {QStringLiteral("sha256"), QString::fromLatin1(QCryptographicHash::hash(payload, QCryptographicHash::Sha256).toHex())}};
  }
  QString save(const QString &name, const QByteArray &data) {
    const auto path = directory_.filePath(name);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(data) != data.size()) return {};
    return path;
  }
private slots:
  void exactIdentityAndSchema() {
    const auto json = QJsonDocument(manifest()).toJson();
    QVERIFY(FirmwareBridge::validateManifest(json, QStringLiteral("TestBrand"), QStringLiteral("MODEL-01")).valid);
    QVERIFY(!FirmwareBridge::validateManifest(json, QStringLiteral("testbrand"), QStringLiteral("MODEL-01")).valid);
    QVERIFY(!FirmwareBridge::validateManifest(json, QStringLiteral("TestBrand"), QStringLiteral("MODEL-01 Pro")).valid);
    QVERIFY(!FirmwareBridge::validateManifest(json, {}, QStringLiteral("MODEL-01")).valid);
    QVERIFY(!FirmwareBridge::validateManifest("[]", QStringLiteral("TestBrand"), QStringLiteral("MODEL-01")).valid);
    QVERIFY(!FirmwareBridge::validateManifest(QByteArray(65537, 'x'), {}, {}).valid);
  }
  void unsafeAndIncompleteMetadata() {
    const auto original = manifest();
    auto item = original;
    item.insert(QStringLiteral("sourceUrl"), QStringLiteral("file:///tmp/firmware"));
    QVERIFY(!FirmwareBridge::validateManifest(QJsonDocument(item).toJson(), "TestBrand", "MODEL-01").valid);
    item = original; item.insert(QStringLiteral("sourceUrl"), QStringLiteral("https://user:secret@vendor.example/fw"));
    QVERIFY(!FirmwareBridge::validateManifest(QJsonDocument(item).toJson(), "TestBrand", "MODEL-01").valid);
    item = original; item.insert(QStringLiteral("sha256"), QStringLiteral("deadbeef"));
    QVERIFY(!FirmwareBridge::validateManifest(QJsonDocument(item).toJson(), "TestBrand", "MODEL-01").valid);
    item = original; item.insert(QStringLiteral("schemaVersion"), 1.2);
    QVERIFY(!FirmwareBridge::validateManifest(QJsonDocument(item).toJson(), "TestBrand", "MODEL-01").valid);
    item = original; item.insert(QStringLiteral("brand"), QStringLiteral("TestBrand\n"));
    QVERIFY(!FirmwareBridge::validateManifest(QJsonDocument(item).toJson(), "TestBrand", "MODEL-01").valid);
    item = original; item.remove(QStringLiteral("build"));
    QVERIFY(!FirmwareBridge::validateManifest(QJsonDocument(item).toJson(), "TestBrand", "MODEL-01").valid);
  }
  void localHashAndTampering() {
    const auto manifestPath = save(QStringLiteral("manifest.json"), QJsonDocument(manifest()).toJson());
    const auto packagePath = save(QStringLiteral("fixture.bin"), QByteArrayLiteral("training fixture\n"));
    QVERIFY(!manifestPath.isEmpty()); QVERIFY(!packagePath.isEmpty());
    FirmwareBridge bridge;
    QSignalSpy finished(&bridge, &FirmwareBridge::inspectionFinished);
    bridge.inspect(QUrl::fromLocalFile(manifestPath).toString(), QUrl::fromLocalFile(packagePath).toString(), "TestBrand", "MODEL-01");
    QVERIFY(bridge.busy());
    QVERIFY(finished.wait(5000));
    QVERIFY(bridge.compatible());
    QVERIFY(!bridge.busy());
    QVERIFY(bridge.status().contains(QStringLiteral("لا يثبت")));
    save(QStringLiteral("fixture.bin"), QByteArrayLiteral("tampered bytes\n"));
    bridge.inspect(manifestPath, packagePath, "TestBrand", "MODEL-01");
    QVERIFY(!bridge.compatible());
    QVERIFY(finished.wait(5000));
    QVERIFY(!bridge.compatible());
    QVERIFY(bridge.status().contains(QStringLiteral("SHA256")));
    bridge.inspect(manifestPath, {}, "TestBrand", "MODEL-01");
    QVERIFY(!bridge.busy()); QVERIFY(!bridge.compatible());
  }
  void identityInvalidationClosesSuccessfulGate() {
    const auto manifestPath = save(QStringLiteral("invalidate.json"), QJsonDocument(manifest()).toJson());
    const auto packagePath = save(QStringLiteral("invalidate.bin"), QByteArrayLiteral("training fixture\n"));
    FirmwareBridge bridge;
    QSignalSpy finished(&bridge, &FirmwareBridge::inspectionFinished);
    bridge.inspect(manifestPath, packagePath, "TestBrand", "MODEL-01");
    QVERIFY(finished.wait(5000));
    QVERIFY(bridge.compatible());
    bridge.invalidate();
    QVERIFY(!bridge.compatible());
    QCOMPARE(bridge.progress(), 0);
    bridge.inspect(manifestPath, packagePath, "TestBrand", "MODEL-01");
    bridge.invalidate();
    QVERIFY(finished.wait(5000));
    QVERIFY(!bridge.compatible());
  }
  void cancelKeepsGateClosed() {
    const auto manifestPath = save(QStringLiteral("cancel.json"), QJsonDocument(manifest()).toJson());
    const auto packagePath = save(QStringLiteral("large.bin"), QByteArrayLiteral("start"));
    QFile package(packagePath);
    QVERIFY(package.open(QIODevice::WriteOnly));
    QVERIFY(package.resize(64 * 1024 * 1024));
    package.close();
    FirmwareBridge bridge;
    QSignalSpy finished(&bridge, &FirmwareBridge::inspectionFinished);
    bridge.inspect(manifestPath, packagePath, "TestBrand", "MODEL-01");
    bridge.cancel();
    QVERIFY(finished.wait(5000));
    QVERIFY(!bridge.compatible());
    QVERIFY(!bridge.busy());
    QVERIFY(bridge.status().contains(QStringLiteral("أُلغي")));
  }
private:
  QTemporaryDir directory_;
};
QTEST_GUILESS_MAIN(FirmwareTests)
#include "firmware_tests.moc"
