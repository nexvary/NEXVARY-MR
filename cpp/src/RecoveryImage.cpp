#include "RecoveryImage.hpp"
#include <QBuffer>
#include <QCryptographicHash>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QUrl>
#include <algorithm>
#include <array>

namespace mr::recovery {
namespace {
constexpr qint64 HardInput = 2LL * 1024 * 1024 * 1024;
constexpr qint64 HardCandidate = 8LL * 1024 * 1024;
constexpr qint64 HardAggregate = 64LL * 1024 * 1024;
constexpr int HardAttempts = 10000;
constexpr int MaximumDimension = 4096;
const QByteArray PngSignature = QByteArray::fromHex("89504e470d0a1a0a");
const QByteArray JpegSignature = QByteArray::fromHex("ffd8ff");
bool stopped(const std::shared_ptr<std::atomic_bool> &cancel) {
  return cancel && cancel->load(std::memory_order_relaxed);
}
QString imagePath(const QString &value) {
  if (value.isEmpty() || value.contains(QChar::Null)) return {};
  for (const auto character : value) if (character.unicode() < 32 || character.unicode() == 127) return {};
  const QUrl url(value);
  QString path;
  if (url.isLocalFile() && (url.host().isEmpty() || url.host() == QStringLiteral("localhost"))) path = url.toLocalFile();
  else if (url.scheme().isEmpty() || QFileInfo(value).isAbsolute()) path = value;
  if (path.isEmpty() || path.startsWith(QStringLiteral("\\\\")) || path.startsWith(QStringLiteral("//"))) return {};
#ifdef Q_OS_WIN
  if (path.mid(2).contains(QLatin1Char(':'))) return {}; // No alternate data streams/device aliases.
#endif
  const QFileInfo info(path);
  if (!info.exists() || !info.isFile() || info.isSymLink()) return {};
  const auto absolute = info.canonicalFilePath();
  for (const auto &prefix : {QStringLiteral("/dev/"), QStringLiteral("/proc/"), QStringLiteral("/sys/")})
    if (absolute.startsWith(prefix)) return {};
  const auto suffix = info.suffix().toLower();
  if (suffix != QStringLiteral("dd") && suffix != QStringLiteral("img") && suffix != QStringLiteral("raw") && suffix != QStringLiteral("bin")) return {};
  return absolute;
}
bool unsupportedContainer(const QByteArray &prefix) {
  return prefix.startsWith("PK\x03\x04") || prefix.startsWith("PK\x05\x06") || prefix.startsWith("PK\x07\x08") ||
      prefix.startsWith(QByteArray::fromHex("455646090d0aff00")) || // EWF/EVF
      prefix.startsWith(QByteArray::fromHex("4c5646090d0aff00")) || // Logical EWF
      prefix.startsWith(QByteArray::fromHex("ed26ff3a")) || // Android sparse image
      prefix.startsWith(QByteArray::fromHex("514649fb")) || // qcow
      prefix.startsWith("KDMV") || prefix.startsWith("vhdxfile") ||
      prefix.startsWith(QByteArray::fromHex("377abcaf271c")) || prefix.startsWith(QByteArray::fromHex("1f8b"));
}
quint32 big32(const QByteArray &data, qsizetype at) {
  quint32 value = 0;
  for (qsizetype i = 0; i < 4; ++i) value = (value << 8U) | static_cast<unsigned char>(data.at(at+i));
  return value;
}
quint16 big16(const QByteArray &data, qsizetype at) {
  return static_cast<quint16>((static_cast<quint16>(static_cast<unsigned char>(data.at(at))) << 8U) | static_cast<unsigned char>(data.at(at+1)));
}
quint32 crc32(const QByteArray &data, qsizetype at, qsizetype length) {
  quint32 crc = 0xffffffffU;
  for (qsizetype i = 0; i < length; ++i) {
    crc ^= static_cast<unsigned char>(data.at(at+i));
    for (int bit = 0; bit < 8; ++bit) crc = (crc >> 1U) ^ ((crc & 1U) ? 0xedb88320U : 0U);
  }
  return crc ^ 0xffffffffU;
}
struct Reader {
  QFile &file;
  qint64 maximum;
  qint64 &inspectionBytes;
  qint64 inspectionLimit;
  const std::shared_ptr<std::atomic_bool> &cancel;
  QByteArray data;
  bool limited = false;
  bool ioFailed = false;
  bool ensure(qint64 size) {
    if (size < 0 || size > maximum) { limited = true; return false; }
    while (data.size() < size) {
      if (stopped(cancel)) return false;
      const auto available = inspectionLimit - inspectionBytes;
      if (available <= 0) { limited = true; return false; }
      const auto amount = std::min({qint64(4096), maximum-qint64(data.size()), available});
      const auto chunk = file.read(amount);
      inspectionBytes += chunk.size();
      if (chunk.isEmpty()) { ioFailed = file.error() != QFileDevice::NoError; return false; }
      data.append(chunk);
    }
    return true;
  }
};
qint64 pngLength(Reader &reader) {
  if (!reader.ensure(8) || !reader.data.startsWith(PngSignature)) return 0;
  qsizetype cursor = 8;
  bool first = true, imageData = false, endedData = false, palette = false;
  unsigned char colorType = 0;
  while (reader.ensure(qint64(cursor)+8)) {
    const auto length = big32(reader.data,cursor);
    const auto type = reader.data.mid(cursor+4,4);
    for (const auto byte : type) if (!((byte >= 'A' && byte <= 'Z') || (byte >= 'a' && byte <= 'z'))) return 0;
    const qint64 end = qint64(cursor) + 12 + length;
    if (!reader.ensure(end)) return 0;
    if (crc32(reader.data,cursor+4,qsizetype(length)+4) != big32(reader.data,qsizetype(end)-4)) return 0;
    if (first) {
      if (type != "IHDR" || length != 13) return 0;
      const auto width = big32(reader.data,cursor+8), height = big32(reader.data,cursor+12);
      if (width == 0 || height == 0 || width > MaximumDimension || height > MaximumDimension) return 0;
      const auto depth = static_cast<unsigned char>(reader.data.at(cursor+16));
      colorType = static_cast<unsigned char>(reader.data.at(cursor+17));
      const bool validDepth = (colorType == 0 && (depth == 1 || depth == 2 || depth == 4 || depth == 8 || depth == 16)) ||
          (colorType == 2 && (depth == 8 || depth == 16)) || (colorType == 3 && (depth == 1 || depth == 2 || depth == 4 || depth == 8)) ||
          ((colorType == 4 || colorType == 6) && (depth == 8 || depth == 16));
      if (!validDepth || reader.data.at(cursor+18) != 0 || reader.data.at(cursor+19) != 0 || static_cast<unsigned char>(reader.data.at(cursor+20)) > 1) return 0;
      first = false;
    } else if (type == "IHDR") return 0;
    else if (type == "PLTE") {
      if (palette || imageData || length == 0 || length > 768 || length % 3 != 0 || colorType == 0 || colorType == 4) return 0;
      palette = true;
    } else if (type == "IDAT") {
      if (endedData || (colorType == 3 && !palette)) return 0;
      imageData = true;
    } else if (type == "IEND") return length == 0 && imageData ? end : 0;
    else {
      if (imageData) endedData = true;
      // Unknown critical chunks are invalid; ancillary metadata is retained byte-exactly.
      if (type.at(0) >= 'A' && type.at(0) <= 'Z') return 0;
    }
    cursor = qsizetype(end);
  }
  return 0;
}
qint64 jpegLength(Reader &reader) {
  if (!reader.ensure(2) || !reader.data.startsWith(QByteArray::fromHex("ffd8"))) return 0;
  qsizetype cursor = 2;
  bool frame = false, entropy = false, scan = false;
  while (reader.ensure(qint64(cursor)+1)) {
    auto marker = static_cast<unsigned char>(reader.data.at(cursor++));
    if (entropy) {
      if (marker != 0xffU) continue;
      do {
        if (!reader.ensure(qint64(cursor)+1)) return 0;
        marker = static_cast<unsigned char>(reader.data.at(cursor++));
      } while (marker == 0xffU);
      if (marker == 0 || (marker >= 0xd0U && marker <= 0xd7U)) continue;
      entropy = false;
    } else {
      if (marker != 0xffU) return 0;
      do {
        if (!reader.ensure(qint64(cursor)+1)) return 0;
        marker = static_cast<unsigned char>(reader.data.at(cursor++));
      } while (marker == 0xffU);
    }
    if (marker == 0xd9U) return frame && scan ? qint64(cursor) : 0;
    if (marker == 0xd8U || marker == 0 || (marker >= 0xd0U && marker <= 0xd7U)) return 0;
    if (marker == 1) continue;
    if (!reader.ensure(qint64(cursor)+2)) return 0;
    const auto length = big16(reader.data,cursor);
    if (length < 2 || !reader.ensure(qint64(cursor)+length)) return 0;
    const bool startOfFrame = marker >= 0xc0U && marker <= 0xcfU && marker != 0xc4U && marker != 0xc8U && marker != 0xccU;
    if (startOfFrame) {
      if (length < 8 || frame) return 0;
      const auto height = big16(reader.data,cursor+3), width = big16(reader.data,cursor+5);
      if (width == 0 || height == 0 || width > MaximumDimension || height > MaximumDimension) return 0;
      frame = true;
    }
    if (marker == 0xdaU) { if (!frame || length < 6) return 0;entropy = true;scan = true; }
    cursor += length;
  }
  return 0;
}
bool decodes(const QByteArray &bytes,const QString &format) {
  QBuffer buffer;buffer.setData(bytes);if (!buffer.open(QIODevice::ReadOnly)) return false;
  QImageReader reader(&buffer,format.toLatin1());
  const auto size = reader.size();
  if (!size.isValid() || size.width() > MaximumDimension || size.height() > MaximumDimension) return false;
  const auto image = reader.read();
  return !image.isNull() && image.width() <= MaximumDimension && image.height() <= MaximumDimension;
}
}

Result scanImage(const QString &value,const Options &options,const std::shared_ptr<std::atomic_bool> &cancel,const Progress &progress) {
  Result result;
  const auto abort = [&](const QString &message,bool cancelled = false) {
    result.candidates.clear();result.sourceUnchanged = false;result.error = message;result.cancelled = cancelled;return result;
  };
  if (options.maximumInputBytes < 1 || options.maximumInputBytes > HardInput || options.maximumCandidateBytes < 1 || options.maximumCandidateBytes > HardCandidate ||
      options.maximumCandidates < 1 || options.maximumCandidates > 100 || options.maximumAggregateBytes < 1 || options.maximumAggregateBytes > HardAggregate ||
      options.maximumSignatureAttempts < 1 || options.maximumSignatureAttempts > HardAttempts || options.chunkBytes < 1 || options.chunkBytes > 1024*1024)
    return abort(QStringLiteral("Invalid limits; hard safety bounds cannot be increased."));
  if (stopped(cancel)) return abort(QStringLiteral("Cancelled."),true);
  const auto path = imagePath(value);
  if (path.isEmpty()) return abort(QStringLiteral("Choose an existing regular local .dd/.img/.raw/.bin image; devices, symlinks and remote paths are not accepted."));
  const QFileInfo original(path);
  const auto size = original.size();
  if (size < 1 || size > options.maximumInputBytes) return abort(QStringLiteral("Image is empty or exceeds the input limit (maximum 2 GiB)."));
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) return abort(file.errorString());
  if (unsupportedContainer(file.peek(16))) return abort(QStringLiteral("This container/compressed/sparse image format is unsupported; provide an authorized raw image file."));
  const qint64 total = size*3;
  qint64 processed = 0;
  const auto report = [&]() { if (progress) progress(processed,total); };
  report();
  const auto hashPass = [&](QFile &input,QByteArray &digest) {
    if (!input.seek(0)) return false;
    QCryptographicHash hash(QCryptographicHash::Sha256);qint64 consumed = 0;
    while (consumed < size) {
      if (stopped(cancel)) return false;
      const auto chunk = input.read(std::min(options.chunkBytes,size-consumed));
      if (chunk.isEmpty()) return false;
      hash.addData(chunk);consumed += chunk.size();processed += chunk.size();report();
    }
    if (input.error() != QFileDevice::NoError || !input.atEnd()) return false;
    digest = hash.result();return true;
  };
  if (!hashPass(file,result.sourceSha256)) return abort(stopped(cancel)?QStringLiteral("Cancelled."):QStringLiteral("Source read failed or changed during initial hashing."),stopped(cancel));
  if (!file.seek(0)) return abort(file.errorString());
  QByteArray overlap;qint64 position = 0, nextEligible = 0, aggregate = 0, inspectionBytes = 0;
  const qint64 inspectionLimit = options.maximumAggregateBytes + 2*options.maximumCandidateBytes;
  bool accepting = true;
  while (position < size) {
    if (stopped(cancel)) return abort(QStringLiteral("Cancelled."),true);
    const auto chunk = file.read(std::min(options.chunkBytes,size-position));
    if (chunk.isEmpty()) return abort(QStringLiteral("Source read failed or changed during scanning."));
    const auto resume = position+chunk.size();
    const QByteArray window = overlap+chunk;
    const auto base = position-overlap.size();
    if (accepting) for (qsizetype index = 0;index < window.size();) {
      const auto pngIndex = window.indexOf(PngSignature,index);
      const auto jpegIndex = window.indexOf(JpegSignature,index);
      if (pngIndex < 0 && jpegIndex < 0) break;
      const bool png = pngIndex >= 0 && (jpegIndex < 0 || pngIndex < jpegIndex);
      index = png ? pngIndex : jpegIndex;
      const qint64 offset = base+index;
      ++index;
      if (offset < nextEligible) continue;
      if (result.signatureAttempts >= options.maximumSignatureAttempts || result.candidates.size() >= options.maximumCandidates || aggregate >= options.maximumAggregateBytes || inspectionBytes >= inspectionLimit) {
        result.limited = true;accepting = false;break;
      }
      ++result.signatureAttempts;
      if (!file.seek(offset)) return abort(file.errorString());
      Reader reader{file,options.maximumCandidateBytes,inspectionBytes,inspectionLimit,cancel,{},false,false};
      const auto length = png ? pngLength(reader) : jpegLength(reader);
      result.limited = result.limited || reader.limited;
      if (stopped(cancel)) return abort(QStringLiteral("Cancelled."),true);
      if (reader.ioFailed) return abort(QStringLiteral("Source read failed while inspecting a signature."));
      if (length > 0) {
        if (length > options.maximumAggregateBytes-aggregate) result.limited = true;
        else {
          const auto bytes = reader.data.left(qsizetype(length));
          const auto format = png ? QStringLiteral("png") : QStringLiteral("jpeg");
          if (decodes(bytes,format)) {
            if (stopped(cancel)) return abort(QStringLiteral("Cancelled."),true);
            result.candidates.append(Candidate{offset,format,bytes,QCryptographicHash::hash(bytes,QCryptographicHash::Sha256)});
            aggregate += length;nextEligible = offset+length;
          }
        }
      }
      if (!file.seek(resume)) return abort(file.errorString());
      // Retained overlap must never cause a signature to be attempted twice.
      nextEligible = std::max(nextEligible,offset+1);
      index = std::max(index,qsizetype(std::min<qint64>(window.size(),nextEligible-base)));
    }
    overlap = window.right(std::min<qsizetype>(7,window.size()));
    position = resume;result.scannedBytes = position;processed += chunk.size();report();
  }
  file.close();
  if (stopped(cancel)) return abort(QStringLiteral("Cancelled."),true);
  const QFileInfo finalInfo(path);
  if (imagePath(path) != path || finalInfo.size() != size || finalInfo.lastModified() != original.lastModified()) return abort(QStringLiteral("Source image changed; all candidates were discarded."));
  QFile finalFile(path);
  if (!finalFile.open(QIODevice::ReadOnly)) return abort(QStringLiteral("Cannot reopen source for final integrity verification."));
  QByteArray after;
  if (!hashPass(finalFile,after)) return abort(stopped(cancel)?QStringLiteral("Cancelled."):QStringLiteral("Source read failed or changed during final hashing."),stopped(cancel));
  if (stopped(cancel)) return abort(QStringLiteral("Cancelled."),true);
  const QFileInfo verified(path);
  if (after != result.sourceSha256 || verified.size() != size || verified.lastModified() != original.lastModified()) return abort(QStringLiteral("Source image changed; all candidates were discarded."));
  result.sourceUnchanged = true;return result;
}
}
