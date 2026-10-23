#include "FirmwareBridge.hpp"
#include "RecoveryFile.hpp"
#include "FirmwareZipFixture.hpp"
#include "PackagePreflight.hpp"
#include <QBuffer>
#include <QElapsedTimer>
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
  void packageListingLimitsAndPayloadDisclaimer(){
    auto archive=[](int count){
      QByteArray data,directory;
      for(int i=0;i<count;++i){
        const auto name=QByteArray("entry-")+QByteArray::number(i)+".txt";QByteArray local(30,'\0'),central(46,'\0');
        local.replace(0,4,QByteArray("PK\003\004",4));qToLittleEndian<quint16>(20,local.data()+4);qToLittleEndian<quint32>(0x8cdc1683,local.data()+14);
        qToLittleEndian<quint32>(1,local.data()+18);qToLittleEndian<quint32>(1,local.data()+22);qToLittleEndian<quint16>(static_cast<quint16>(name.size()),local.data()+26);
        central.replace(0,4,QByteArray("PK\001\002",4));qToLittleEndian<quint16>(20,central.data()+6);qToLittleEndian<quint32>(0x8cdc1683,central.data()+16);
        qToLittleEndian<quint32>(1,central.data()+20);qToLittleEndian<quint32>(1,central.data()+24);qToLittleEndian<quint16>(static_cast<quint16>(name.size()),central.data()+28);qToLittleEndian<quint32>(static_cast<quint32>(data.size()),central.data()+42);
        data+=local+name+"x";directory+=central+name;
      }
      QByteArray end(22,'\0');end.replace(0,4,QByteArray("PK\005\006",4));qToLittleEndian<quint16>(static_cast<quint16>(count),end.data()+8);qToLittleEndian<quint16>(static_cast<quint16>(count),end.data()+10);
      qToLittleEndian<quint32>(static_cast<quint32>(directory.size()),end.data()+12);qToLittleEndian<quint32>(static_cast<quint32>(data.size()),end.data()+16);return data+directory+end;
    };
    FirmwareBridge bridge;QSignalSpy finished(&bridge,&FirmwareBridge::inspectionFinished);
    bridge.inspectPackage(save("many.zip",archive(300)));QVERIFY(finished.wait(5000));QCOMPARE(bridge.packageReport().value("entryCount").toInt(),300);
    QCOMPARE(bridge.packageReport().value("entries").toList().size(),256);QVERIFY(bridge.packageReport().value("listedEntriesTruncated").toBool());
    bridge.inspectPackage(save("too-many.zip",archive(4097)));QVERIFY(finished.wait(5000));QVERIFY(bridge.packageReport().isEmpty());
    auto changed=syntheticFirmwareZip();changed[38]='T';bridge.inspectPackage(save("unverified-payload.zip",changed));QVERIFY(finished.wait(5000));
    QCOMPARE(bridge.packageReport().value("status").toString(),QString("structure-only"));QVERIFY(!bridge.packageReport().value("payloadCrcVerified").toBool());QVERIFY(!bridge.compatible());
  }
  void packageSnapshotMetadataAndTrust(){
    const auto bytes=syntheticFirmwareZip();const auto path=save("synthetic-firmware.zip",bytes);FirmwareBridge bridge;
    QSignalSpy finished(&bridge,&FirmwareBridge::inspectionFinished);bridge.inspectPackage(path);QVERIFY(bridge.busy());QVERIFY(finished.wait(5000));
    const auto r=bridge.packageReport();QCOMPARE(r.value("status").toString(),QString("structure-only"));QCOMPARE(r.value("entryCount").toInt(),3);
    QCOMPARE(r.value("snapshotSha256").toString(),QStringLiteral("93f6b65816647c6790f39c88e2f44827af5d1429db471455657ecf7a4b944863"));
    for(const auto key:{"oemSignatureVerified","payloadCrcVerified","partitionLayoutVerified","deviceCompatibilityVerified","extracted"})QCOMPARE(r.value(key).toBool(),false);
    QVERIFY(!bridge.compatible());QCOMPARE(r.value("imageNameCandidates").toList().size(),1);QVERIFY(!bridge.packageJson().contains(path));
    bridge.invalidate();QVERIFY(bridge.packageReport().isEmpty());
  }
  void packageMalformedAndUnsupportedInputs(){
    const auto good=syntheticFirmwareZip();FirmwareBridge bridge;QSignalSpy finished(&bridge,&FirmwareBridge::inspectionFinished);
    auto encrypted=good;encrypted[6]=1;
    auto badName=good;badName.replace("boot.img","boot\nimg");
    auto bidi=good;bidi.replace("boot.img",QByteArray::fromHex("e280ae782e696d67"));bidi[7]=8;const auto central=bidi.indexOf(QByteArray("PK\001\002",4));bidi[central+9]=8;
    for(const auto &bytes:QList<QByteArray>{QByteArray(64,'x'),good.left(good.size()-2),encrypted,badName,bidi}){
      bridge.inspectPackage(save("bad-container.zip",bytes));QVERIFY(finished.wait(5000));QVERIFY(bridge.packageReport().isEmpty());QVERIFY(!bridge.compatible());
    }
    bridge.inspectPackage("https://example.invalid/fw.zip");QVERIFY(!bridge.busy());
    const auto oversized=save("oversize.zip",good);QFile file(oversized);QVERIFY(file.open(QIODevice::WriteOnly));QVERIFY(file.resize(128*1024*1024+1));file.close();
    bridge.inspectPackage(oversized);QVERIFY(!bridge.busy());QVERIFY(bridge.packageReport().isEmpty());
  }
  void packageListingPublishesOnlyAfterValidation(){
    auto bytes=syntheticFirmwareZip();QBuffer buffer(&bytes);QVERIFY(buffer.open(QIODevice::ReadOnly));QList<mr::service::PackageEntry> listing;
    QVERIFY(mr::service::validPackageStructure(buffer,false,&listing));QCOMPARE(listing.size(),3);QCOMPARE(listing.first().name,QByteArray("boot.img"));
    buffer.close();bytes.chop(1);QVERIFY(buffer.open(QIODevice::ReadOnly));QVERIFY(!mr::service::validPackageStructure(buffer,false,&listing));QVERIFY(listing.isEmpty());
  }
  void packageCancellationAndPrivateExport(){
    const auto bytes=syntheticFirmwareZip();const auto path=save("export-container.zip",bytes);FirmwareBridge bridge;QSignalSpy finished(&bridge,&FirmwareBridge::inspectionFinished);
    bridge.inspectPackage(path);bridge.cancel();QVERIFY(finished.wait(5000));QVERIFY(bridge.packageReport().isEmpty());QVERIFY(!bridge.compatible());
    bridge.inspectPackage(path);QVERIFY(finished.wait(5000));const auto report=directory_.filePath("package-report.json");
    QVERIFY(!bridge.exportPackageReport(report,false));QVERIFY(!QFile::exists(report));QVERIFY(!bridge.exportPackageReport(path,true));
    QVERIFY(bridge.exportPackageReport(report,true));QFile f(report);QVERIFY(f.open(QIODevice::ReadOnly));const auto original=f.readAll();f.close();
    QVERIFY(!bridge.exportPackageReport(report,true));QVERIFY(f.open(QIODevice::ReadOnly));QCOMPARE(f.readAll(),original);
    QVERIFY(!bridge.exportPackageReport("file://server/share/out.json",true));QFile source(path);QVERIFY(source.open(QIODevice::ReadOnly));QCOMPARE(source.readAll(),bytes);
  }
  void sharedLocalPathPolicy() {
    const auto ordinary=directory_.filePath(QStringLiteral("file with spaces.bin"));
    QCOMPARE(mr::recovery::localFilePath(ordinary),ordinary);
    QCOMPARE(mr::recovery::localFilePath(QUrl::fromLocalFile(ordinary).toString()),ordinary);
    const QStringList rejected={QString(),QStringLiteral("https://example.invalid/image.img"),
      QStringLiteral("file://server/share/image.img"),QStringLiteral("//server/share/image.img"),
      QStringLiteral("\\\\server\\share\\image.img"),QStringLiteral("file:///tmp/unsafe%00image"),
      QStringLiteral("file:///tmp/unsafe%0aimage"),QStringLiteral("bad\nname")};
    for(const auto &value:rejected)QVERIFY2(mr::recovery::localFilePath(value).isEmpty(),qPrintable(value));
#ifdef Q_OS_WIN
    QVERIFY(mr::recovery::localFilePath(QStringLiteral("C:/NUL")).isEmpty());
    QVERIFY(mr::recovery::localFilePath(QStringLiteral("C:/fixture.img:stream")).isEmpty());
#endif
  }
  void descriptorPolicyRejectsSpecialFiles() {
    QFile directory(directory_.path());QVERIFY(!mr::recovery::openRegularInput(directory));
#ifndef Q_OS_WIN
    const auto source=save(QStringLiteral("regular-input.bin"),QByteArrayLiteral("synthetic"));
    const auto link=directory_.filePath(QStringLiteral("input-link"));QVERIFY(QFile::link(source,link));
    QFile linked(link);QVERIFY(!mr::recovery::openRegularInput(linked));
    const auto fifo=directory_.filePath(QStringLiteral("input-fifo"));
    QVERIFY(::mkfifo(QFile::encodeName(fifo).constData(),0600)==0);
    QFile pipe(fifo);QElapsedTimer timer;timer.start();
    QVERIFY(!mr::recovery::openRegularInput(pipe));QVERIFY(timer.elapsed()<1000);
#endif
  }
  void firmwareEntriesRejectUnsafeInputPaths() {
    const auto metadata=save(QStringLiteral("policy.json"),QJsonDocument(manifest()).toJson());
    const auto package=save(QStringLiteral("policy.bin"),QByteArrayLiteral("training fixture\n"));
    FirmwareBridge bridge;
    for(const auto &input:QStringList{QStringLiteral("https://example.invalid/image"),QStringLiteral("file://server/share/image"),QStringLiteral("file:///tmp/unsafe%00image")}) {
      bridge.inspect(input,package,"TestBrand","MODEL-01");QVERIFY(!bridge.busy());QVERIFY(!bridge.compatible());
      bridge.inspect(metadata,input,"TestBrand","MODEL-01");QVERIFY(!bridge.busy());QVERIFY(!bridge.compatible());
      bridge.inspectAvb(input);QVERIFY(!bridge.busy());QVERIFY(bridge.avbJson().isEmpty());
    }
#ifndef Q_OS_WIN
    const auto manifestLink=directory_.filePath(QStringLiteral("manifest-link"));
    const auto packageLink=directory_.filePath(QStringLiteral("package-link"));
    QVERIFY(QFile::link(metadata,manifestLink));QVERIFY(QFile::link(package,packageLink));
    bridge.inspect(manifestLink,package,"TestBrand","MODEL-01");QVERIFY(!bridge.busy());QVERIFY(!bridge.compatible());
    bridge.inspect(metadata,packageLink,"TestBrand","MODEL-01");QVERIFY(!bridge.busy());QVERIFY(!bridge.compatible());
#endif
  }
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

