#pragma once
#include <QByteArray>
#include <QRegularExpression>
#include <QString>
#include <array>
#include <sodium.h>
namespace mr::update::detail {
inline bool decimal(const QString &value, quint64 &number) {
  static const QRegularExpression pattern(QStringLiteral("\\A(?:0|[1-9][0-9]{0,19})\\z"));
  if (!pattern.match(value).hasMatch()) return false;
  bool ok = false; number = value.toULongLong(&ok); return ok;
}
inline bool version(const QString &value, std::array<quint64,3> &parts) {
  const auto components = value.split(QLatin1Char('.'));
  if (components.size() != 3) return false;
  for (qsizetype i = 0; i < 3; ++i) {
    auto &part = parts[static_cast<std::size_t>(i)];
    if (!decimal(components[i], part) || part > 65535) return false;
  }
  return true;
}
inline bool cryptoInitialized() { static const int result = sodium_init(); return result >= 0; }
inline bool approvedCryptoVersion() {
  return QByteArray(SODIUM_VERSION_STRING) == "1.0.22" && QByteArray(sodium_version_string()) == "1.0.22";
}
}
