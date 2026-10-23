#include "AvbMetadata.hpp"
#include <QJsonArray>
#include <QRandomGenerator>
#include <QtEndian>
#include <cstdio>

// Deterministic hostile-input smoke; also built independently under ASan/UBSan.
// This is bounded mutation coverage, not exhaustive fuzzing or format certification.
int main() {
  QRandomGenerator random(0x4e455856U);
  QByteArray seed(512, '\0'); seed.replace(0, 4, "AVB0");
  qToBigEndian<quint32>(1, seed.data() + 4);
  qToBigEndian<quint64>(256, seed.data() + 20);
  qToBigEndian<quint64>(16, seed.data() + 104);
  if (AvbMetadata::inspect(seed)[QStringLiteral("status")].toString() != QStringLiteral("metadata-only")) return 1;
  for (unsigned iteration = 0; iteration < 30000; ++iteration) {
    auto bytes = seed;
    const unsigned edits = 1U + random.bounded(8U);
    for (unsigned i = 0; i < edits; ++i) {
      const auto at = static_cast<qsizetype>(random.bounded(512U));
      bytes[at] = static_cast<char>(random.bounded(256U));
    }
    if ((iteration % 3) == 0) bytes.truncate(static_cast<qsizetype>(random.bounded(513U)));
    const auto report = AvbMetadata::inspect(bytes);
    const auto status = report[QStringLiteral("status")].toString();
    if (status != QStringLiteral("metadata-only") && status != QStringLiteral("rejected")) return 2;
    for (const auto *key : {"signatureVerified", "oemTrustVerified", "deviceCompatibilityVerified", "rollbackSafetyVerified"}) {
      if (!report.value(QLatin1String(key)).isBool() || report.value(QLatin1String(key)).toBool(true)) return 3;
    }
    if (report[QStringLiteral("descriptorEnvelopes")].toArray().size() > 4096) return 4;
  }
  std::puts("30000 deterministic AVB mutations passed; no trust assertion permitted.");
  return 0;
}
