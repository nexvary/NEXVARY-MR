#include "RecoveryImage.hpp"
#include "RecoveryFile.hpp"
#include <QBuffer>
#include <QCryptographicHash>
#include <QFile>
#include <QImage>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>
#include <algorithm>

using namespace mr::recovery;
class RecoveryImageTests final : public QObject {
  Q_OBJECT
  QTemporaryDir directory;
  QByteArray raster(const char *format,Qt::GlobalColor color) {
    QImage image(12,9,QImage::Format_RGB32);image.fill(color);
    QByteArray bytes;QBuffer buffer(&bytes);
    if(!buffer.open(QIODevice::WriteOnly)||!image.save(&buffer,format)) return {};
    return bytes;
  }
  QString write(const QByteArray &bytes,const QString &name=QStringLiteral("fixture.img")) {
    QFile file(directory.filePath(name));
    if(!file.open(QIODevice::WriteOnly)||file.write(bytes)!=bytes.size()) return {};
    return file.fileName();
  }
private slots:
  void exactOffsetsHashesAndChunkBoundaries() {
    const auto png1=raster("PNG",Qt::red),png2=raster("PNG",Qt::green),jpeg=raster("JPEG",Qt::blue);
    QVERIFY(!png1.isEmpty());QVERIFY(!png2.isEmpty());QVERIFY(!jpeg.isEmpty());
    const auto raw=QByteArray(30,'x')+png1+QByteArray(19,'\0')+jpeg+QByteArray(17,'q')+png2+QByteArray(11,'z');
    const auto path=write(raw);QVERIFY(!path.isEmpty());
    const QList<qint64> offsets{30,30+png1.size()+19,30+png1.size()+19+jpeg.size()+17};
    for(const qint64 chunk: {qint64(1),qint64(31),qint64(65536)}) {
      Options options;options.chunkBytes=chunk;qint64 last=-1;
      const auto result=scanImage(QUrl::fromLocalFile(path).toString(),options,{},[&](qint64 done,qint64 total){QVERIFY(done>=last);QVERIFY(done<=total);QCOMPARE(total,qint64(raw.size())*3);last=done;});
      QVERIFY2(result.error.isEmpty(),qPrintable(result.error));QVERIFY(result.sourceUnchanged);QVERIFY(!result.limited);QCOMPARE(result.candidates.size(),3);
      QCOMPARE(result.scannedBytes,qint64(raw.size()));QCOMPARE(last,qint64(raw.size())*3);
      QCOMPARE(result.sourceSha256,QCryptographicHash::hash(raw,QCryptographicHash::Sha256));
      const QList<QByteArray> expected{png1,jpeg,png2};
      for(qsizetype i=0;i<3;++i) {QCOMPARE(result.candidates[i].offset,offsets[i]);QCOMPARE(result.candidates[i].bytes,expected[i]);QCOMPARE(result.candidates[i].sha256,QCryptographicHash::hash(expected[i],QCryptographicHash::Sha256));}
      QCOMPARE(result.candidates[1].format,QStringLiteral("jpeg"));
    }
    QFile source(path);QVERIFY(source.open(QIODevice::ReadOnly));QCOMPARE(source.readAll(),raw);
  }
  void invalidAndTruncatedCandidates() {
    auto png=raster("PNG",Qt::red);const auto jpeg=raster("JPEG",Qt::green);QVERIFY(!png.isEmpty());QVERIFY(!jpeg.isEmpty());
    png[40]=static_cast<char>(png.at(40)^1); // CRC-corrupted chunk must be rejected before decoding.
    const auto raw=png+QByteArray(10,'x')+jpeg.left(jpeg.size()-2)+QByteArray(10,'x');
    const auto result=scanImage(write(raw));QVERIFY(result.error.isEmpty());QVERIFY(result.sourceUnchanged);QVERIFY(result.candidates.isEmpty());
    const auto empty=scanImage(write({}));QVERIFY(!empty.error.isEmpty());QVERIFY(!empty.sourceUnchanged);
    const auto noSignatures=scanImage(write(QByteArray(100,'x')));QVERIFY(noSignatures.error.isEmpty());QVERIFY(noSignatures.candidates.isEmpty());QCOMPARE(noSignatures.signatureAttempts,0);
  }
  void jpegExifWithoutJfifIsDecodedByteExactly() {
    auto jpeg=raster("JPEG",Qt::blue);QVERIFY(jpeg.startsWith(QByteArray::fromHex("ffd8ffe0")));
    const auto app0Length=(static_cast<unsigned char>(jpeg.at(4))<<8)|static_cast<unsigned char>(jpeg.at(5));
    // Replace the optional JFIF APP0 marker with a minimal valid EXIF APP1 TIFF.
    const auto exif=QByteArray::fromHex("ffe1001645786966000049492a0008000000000000000000");
    jpeg=jpeg.left(2)+exif+jpeg.mid(4+app0Length);
    QVERIFY(!jpeg.contains("JFIF"));QVERIFY(jpeg.contains(QByteArray("Exif\0\0",6)));
    const auto raw=QByteArray(63,'x')+jpeg+QByteArray(11,'y');
    const auto result=scanImage(write(raw,"exif.img"));
    QVERIFY2(result.error.isEmpty(),qPrintable(result.error));QVERIFY(result.sourceUnchanged);
    QCOMPARE(result.candidates.size(),1);QCOMPARE(result.candidates.first().offset,qint64(63));
    QCOMPARE(result.candidates.first().bytes,jpeg);
    QCOMPARE(result.candidates.first().sha256,QCryptographicHash::hash(jpeg,QCryptographicHash::Sha256));
  }
  void rejectsContainersAndDevicePaths() {
    const auto png=raster("PNG",Qt::red);
    for(const auto &prefix:QList<QByteArray>{QByteArray::fromHex("504b0304"),QByteArray::fromHex("455646090d0aff00"),QByteArray::fromHex("ed26ff3a"),QByteArray("vhdxfile")}) {
      const auto result=scanImage(write(prefix+png));QVERIFY(!result.error.isEmpty());QVERIFY(result.candidates.isEmpty());
    }
    for(const auto &path:QStringList{QStringLiteral("https://example.invalid/test.img"),QStringLiteral("\\\\.\\PhysicalDrive0"),QStringLiteral("//server/share/test.img"),QStringLiteral("/dev/zero"),QStringLiteral("/proc/self/mem"),QStringLiteral("missing.img"),QString("bad")+QChar::Null+QString(".img")}) QVERIFY(!scanImage(path).error.isEmpty());
    QVERIFY(!scanImage(write(png,"wrong.png")).error.isEmpty());
#ifndef Q_OS_WIN
    const auto source=write(png,"real.img");const auto link=directory.filePath("symlink.img");QVERIFY(QFile::link(source,link));QVERIFY(!scanImage(link).error.isEmpty());
#endif
  }
  void resourceLimits() {
    const auto png=raster("PNG",Qt::red),second=raster("PNG",Qt::blue);const auto path=write(png+QByteArray(10,'x')+second);
    Options options;options.maximumCandidates=1;auto result=scanImage(path,options);QVERIFY(result.sourceUnchanged);QVERIFY(result.limited);QCOMPARE(result.candidates.size(),1);
    options={};options.maximumAggregateBytes=png.size();result=scanImage(path,options);QVERIFY(result.sourceUnchanged);QVERIFY(result.limited);QCOMPARE(result.candidates.size(),1);
    options={};options.maximumCandidateBytes=16;result=scanImage(path,options);QVERIFY(result.sourceUnchanged);QVERIFY(result.limited);QVERIFY(result.candidates.isEmpty());
    options={};options.maximumInputBytes=1;QVERIFY(!scanImage(path,options).error.isEmpty());
    options={};options.chunkBytes=0;QVERIFY(!scanImage(path,options).error.isEmpty());
    options={};options.maximumInputBytes=2LL*1024*1024*1024+1;QVERIFY(!scanImage(path,options).error.isEmpty());
    QByteArray flood;for(int i=0;i<50;++i) flood+=QByteArray::fromHex("89504e470d0a1a0a")+QByteArray(16,'x');
    options={};options.maximumSignatureAttempts=3;result=scanImage(write(flood+png,"flood.img"),options);QVERIFY(result.sourceUnchanged);QVERIFY(result.limited);QCOMPARE(result.signatureAttempts,3);QVERIFY(result.candidates.isEmpty());
  }
  void openedDescriptorMustBeRegular() {
    const auto path=write(QByteArray("synthetic descriptor test"),"regular.img");
    QFile regular(path);QVERIFY(openRegularInput(regular));QCOMPARE(regular.readAll(),QByteArray("synthetic descriptor test"));regular.close();
#ifndef Q_OS_WIN
    QFile device(QStringLiteral("/dev/zero"));QVERIFY(!openRegularInput(device));
    const auto fifo=directory.filePath("replacement-fifo.img");
    QVERIFY(::mkfifo(QFile::encodeName(fifo).constData(),0600)==0);
    QFile pipe(fifo);QVERIFY(!openRegularInput(pipe));
    const auto link=directory.filePath("descriptor-link.img");QVERIFY(QFile::link(path,link));
    QFile symlink(link);QVERIFY(!openRegularInput(symlink));
#else
    QFile device(QStringLiteral("NUL"));QVERIFY(!openRegularInput(device));
#endif
  }
  void cancellationAndSourceMutationDiscardEverything() {
    const auto png=raster("PNG",Qt::red);const auto bytes=png+QByteArray(100,'x');const auto path=write(bytes);
    auto cancel=std::make_shared<std::atomic_bool>(true);auto result=scanImage(path,{},cancel);QVERIFY(result.cancelled);QVERIFY(result.candidates.isEmpty());
    cancel->store(false);result=scanImage(path,{},cancel,[&](qint64 done,qint64){if(done>bytes.size())cancel->store(true);});QVERIFY(result.cancelled);QVERIFY(!result.sourceUnchanged);QVERIFY(result.candidates.isEmpty());
    bool changed=false;result=scanImage(path,{}, {},[&](qint64 done,qint64){if(!changed&&done>=2*qint64(bytes.size())){QFile edit(path);QVERIFY(edit.open(QIODevice::ReadWrite));QVERIFY(edit.seek(bytes.size()-1));QCOMPARE(edit.write("y",1),qint64(1));QVERIFY(edit.flush());changed=true;}});
    QVERIFY(changed);QVERIFY(!result.error.isEmpty());QVERIFY(!result.sourceUnchanged);QVERIFY(result.candidates.isEmpty());
  }
  void oversizedDimensionsAreNotDecoded() {
    QImage wide(4097,1,QImage::Format_RGB32);wide.fill(Qt::red);QByteArray bytes;QBuffer buffer(&bytes);QVERIFY(buffer.open(QIODevice::WriteOnly));QVERIFY(wide.save(&buffer,"PNG"));buffer.close();
    const auto result=scanImage(write(bytes));QVERIFY(result.sourceUnchanged);QVERIFY(result.error.isEmpty());QVERIFY(result.candidates.isEmpty());
  }
};
QTEST_GUILESS_MAIN(RecoveryImageTests)
#include "recovery_image_tests.moc"
