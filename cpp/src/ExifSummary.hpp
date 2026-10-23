#pragma once
#include <QByteArray>
#include <QJsonObject>
#include <QString>
#include <atomic>
#include <memory>
namespace mr::forensic {
inline constexpr qsizetype maximumExifSourceBytes = 8 * 1024 * 1024;
struct ExifSummary { QJsonObject report; QString error; };
// Bounded JPEG APP1 / classic-TIFF allowlist. Does not decode image pixels,
// validate authenticity, follow thumbnail/MakerNote IFDs or extract GPS values.
ExifSummary summarizeExif(const QByteArray &bytes,
 const std::shared_ptr<std::atomic_bool> &cancelled = {});
}
