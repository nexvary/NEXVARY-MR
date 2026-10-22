#pragma once
#include <QByteArray>
#include <QJsonObject>

// Untrusted local metadata inspection only. Never authorizes a device operation.
namespace AvbMetadata {
inline constexpr qsizetype maxImageBytes = 16 * 1024 * 1024;
QJsonObject inspect(const QByteArray &image);
}
