#include "AvbMetadata.hpp"
#include <QCryptographicHash>
#include <QJsonArray>
#include <QString>
#include <QtEndian>
#include <array>

namespace AvbMetadata {
QJsonObject inspect(const QByteArray &image) {
  QJsonObject report{{QStringLiteral("schemaVersion"), 1},
    {QStringLiteral("capability"), QStringLiteral("firmware.avb.metadata.v1")},
    {QStringLiteral("status"), QStringLiteral("rejected")},
    {QStringLiteral("signatureVerified"), false},
    {QStringLiteral("oemTrustVerified"), false},
    {QStringLiteral("deviceCompatibilityVerified"), false},
    {QStringLiteral("rollbackSafetyVerified"), false}};
  auto fail = [&report](const char *code) {
    report.insert(QStringLiteral("error"), QString::fromLatin1(code));
    return report;
  };
  if (image.size() < 256 || image.size() > maxImageBytes) return fail("image-size-limit");
  if (!image.startsWith("AVB0")) return fail("raw-vbmeta-required");
  const auto u32 = [&image](qsizetype offset) { return qFromBigEndian<quint32>(image.constData() + offset); };
  const auto u64 = [&image](qsizetype offset) { return qFromBigEndian<quint64>(image.constData() + offset); };
  const auto within = [](quint64 offset, quint64 length, quint64 limit) {
    return offset <= limit && length <= limit - offset;
  };
  if (u32(4) != 1) return fail("unsupported-header-major");
  const quint64 auth = u64(12), aux = u64(20);
  const quint64 available = static_cast<quint64>(image.size()) - 256;
  if ((auth % 64) != 0 || (aux % 64) != 0) return fail("unaligned-blocks");
  if (!within(0, auth, available) || !within(auth, aux, available)) return fail("truncated-blocks");
  struct Region { quint64 offset; quint64 size; quint64 limit; };
  const std::array<Region, 5> regions{{{u64(32), u64(40), auth}, {u64(48), u64(56), auth},
    {u64(64), u64(72), aux}, {u64(80), u64(88), aux}, {u64(96), u64(104), aux}}};
  for (const auto &region : regions) if (!within(region.offset, region.size, region.limit)) return fail("region-out-of-bounds");
  const auto overlaps = [](const Region &a, const Region &b) {
    // Endpoints cannot overflow: every region was checked against a bounded block.
    return a.size && b.size && a.offset < b.offset + b.size && b.offset < a.offset + a.size;
  };
  if (overlaps(regions[0], regions[1]) || overlaps(regions[2], regions[3]) ||
      overlaps(regions[2], regions[4]) || overlaps(regions[3], regions[4])) return fail("overlapping-regions");
  if (image.at(175) != '\0') return fail("unterminated-release");
  for (qsizetype i = 176; i < 256; ++i) if (image.at(i) != '\0') return fail("nonzero-reserved-header");
  QJsonArray descriptors;
  quint64 cursor = 256 + auth + regions[4].offset;
  const quint64 end = cursor + regions[4].size;
  while (cursor < end) {
    if (descriptors.size() >= 4096) return fail("descriptor-count-limit");
    if (end - cursor < 16) return fail("truncated-descriptor");
    const quint64 tag = u64(static_cast<qsizetype>(cursor));
    const quint64 following = u64(static_cast<qsizetype>(cursor + 8));
    if ((following % 8) != 0 || following > end - cursor - 16) return fail("invalid-descriptor-length");
    // Descriptor bodies remain uninterpreted; do not infer partition coverage.
    descriptors.append(QJsonObject{{QStringLiteral("tag"), QString::number(tag)},
      {QStringLiteral("bodyBytes"), QString::number(following)}});
    cursor += 16 + following;
  }
  const quint32 flags = u32(120);
  QJsonArray warnings{QStringLiteral("metadata-is-untrusted"), QStringLiteral("signature-and-oem-key-not-verified"),
    QStringLiteral("partition-digests-not-verified"), QStringLiteral("device-rollback-state-not-read")};
  if (flags & 1U) warnings.append(QStringLiteral("hashtree-disabled-flag-declared"));
  if (flags & 2U) warnings.append(QStringLiteral("verification-disabled-flag-declared"));
  if (flags & ~3U) warnings.append(QStringLiteral("unknown-flags-declared"));
  if (u32(28) == 0) warnings.append(QStringLiteral("unsigned-algorithm-declared"));
  QString release;
  for (qsizetype i = 128; i < 175 && image.at(i) != '\0'; ++i) {
    const auto byte = static_cast<unsigned char>(image.at(i));
    release.append(byte >= 32 && byte <= 126 ? QChar(byte) : QChar('?'));
  }
  report.insert(QStringLiteral("status"), QStringLiteral("metadata-only"));
  report.insert(QStringLiteral("sha256"), QString::fromLatin1(QCryptographicHash::hash(image, QCryptographicHash::Sha256).toHex()));
  report.insert(QStringLiteral("imageBytes"), QString::number(image.size()));
  report.insert(QStringLiteral("vbmetaBytes"), QString::number(256 + auth + aux));
  report.insert(QStringLiteral("requiredVersion"), QStringLiteral("%1.%2").arg(u32(4)).arg(u32(8)));
  report.insert(QStringLiteral("declaredAlgorithmId"), static_cast<double>(u32(28)));
  report.insert(QStringLiteral("declaredRollbackIndex"), QString::number(u64(112)));
  report.insert(QStringLiteral("declaredRollbackLocation"), static_cast<double>(u32(124)));
  report.insert(QStringLiteral("declaredFlags"), static_cast<double>(flags));
  report.insert(QStringLiteral("declaredRelease"), release);
  report.insert(QStringLiteral("descriptorEnvelopes"), descriptors);
  report.insert(QStringLiteral("warnings"), warnings);
  return report;
}
}
