#include "BoardBridge.hpp"

#include <QCryptographicHash>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QStringList>
#include <QUrl>
#include <QVariantMap>
#include <QBuffer>
#include <QImageReader>
#include <QImage>
#include <QPainter>
#include <cmath>

namespace {
constexpr qint64 MaxFileBytes = 8 * 1024 * 1024;
constexpr qint64 MaxSessionBytes = 16 * 1024 * 1024;
constexpr qint64 MaxPhotoInputBytes = 4 * 1024 * 1024;
constexpr qint64 MaxPhotoBytes = 2 * 1024 * 1024;
constexpr qsizetype MaxPhotos = 4;
constexpr qsizetype MaxRecords = 10000;
bool finiteNumber(const QJsonValue &v) { return v.isDouble() && std::isfinite(v.toDouble()); }
bool textField(const QJsonObject &o, const QString &key, qsizetype maximum, bool required = true) {
  const auto v = o.value(key);
  return (!required && v.isUndefined()) || (v.isString() && v.toString().size() <= maximum && (!required || !v.toString().trimmed().isEmpty()));
}
bool identifier(const QString &s) {
  static const QRegularExpression pattern(QStringLiteral("^[A-Za-z][A-Za-z0-9_.:-]{0,63}$"));
  return pattern.match(s).hasMatch();
}
QString localPath(const QString &input, bool allowResource) {
  if (allowResource && input.startsWith(QLatin1String(":"))) return input;
  const QUrl url(input);
  if (url.isLocalFile()) return url.toLocalFile();
  if (allowResource && url.scheme() == QLatin1String("qrc")) return QLatin1Char(':') + url.path();
  // QUrl interprets C:\\... as a scheme. QFileInfo preserves native Windows paths.
  if (QFileInfo(input).isAbsolute() || url.scheme().isEmpty()) return input;
  return {};
}
bool readJson(const QString &path, QJsonObject &result, QByteArray &bytes, QString &error, qint64 maximum = MaxFileBytes) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) { error = QStringLiteral("Cannot read file: ") + file.errorString(); return false; }
  if (file.size() > maximum) { error = QStringLiteral("File exceeds the %1 MiB limit.").arg(maximum / (1024 * 1024)); return false; }
  bytes = file.read(maximum + 1);
  if (bytes.size() > maximum || file.error() != QFileDevice::NoError) { error = QStringLiteral("File read failed or exceeded its limit."); return false; }
  QJsonParseError parse;
  const auto doc = QJsonDocument::fromJson(bytes, &parse);
  if (parse.error != QJsonParseError::NoError || !doc.isObject()) { error = QStringLiteral("Expected a JSON object: ") + parse.errorString(); return false; }
  result = doc.object();
  return true;
}
bool validPoint(const QJsonObject &o, double width, double height) {
  return finiteNumber(o.value("x")) && finiteNumber(o.value("y")) && o.value("x").toDouble() >= 0 && o.value("y").toDouble() >= 0 && o.value("x").toDouble() <= width && o.value("y").toDouble() <= height;
}
bool normalizePhoto(const QByteArray &bytes, bool pngOnly, QByteArray &normalized, QSize &dimensions) {
  if(bytes.isEmpty() || bytes.size() > MaxPhotoInputBytes) return false;
  QBuffer input; input.setData(bytes); if(!input.open(QIODevice::ReadOnly)) return false;
  QImageReader reader(&input); const auto format=reader.format().toLower();
  if(format != "png" && (pngOnly || (format != "jpeg" && format != "jpg"))) return false;
  const auto size=reader.size();
  if(!size.isValid() || size.width()>4096 || size.height()>4096) return false;
  reader.setAutoTransform(true); const auto image=reader.read();
  if(image.isNull() || image.width()>4096 || image.height()>4096) return false;
  // Repaint into a fresh raster to discard EXIF, source text and other metadata.
  QImage clean(image.size(),QImage::Format_ARGB32); if(clean.isNull()) return false;
  clean.fill(Qt::transparent); {QPainter painter(&clean); if(!painter.isActive()) return false;painter.drawImage(0,0,image);}
  QBuffer output(&normalized);if(!output.open(QIODevice::WriteOnly)||!clean.save(&output,"PNG")) return false;
  dimensions=clean.size();return normalized.size()<=MaxPhotoBytes;
}
}

BoardBridge::BoardBridge(QObject *parent) : QObject(parent) {}
bool BoardBridge::fail(const QString &message) { m_error = message; emit errorChanged(); return false; }
void BoardBridge::clearError() { if (!m_error.isEmpty()) { m_error.clear(); emit errorChanged(); } }

bool BoardBridge::importBoard(const QString &path, bool rightsConfirmed) {
  if (!rightsConfirmed) return fail(QStringLiteral("Confirm that you have permission to use this board file."));
  const auto resolved = localPath(path, true);
  if (resolved.isEmpty()) return fail(QStringLiteral("Only local files and bundled resources are supported."));
  QJsonObject root; QByteArray bytes; QString problem;
  if (!readJson(resolved, root, bytes, problem)) return fail(problem);
  if (root.value("format").toString() != QLatin1String("nexvary.board.v1") ||
      !textField(root, "title", 160) || !textField(root, "author", 160) ||
      !textField(root, "license", 160) || !textField(root, "description", 4000, false))
    return fail(QStringLiteral("Unsupported format or missing title, author, or license metadata."));
  const auto width = root.value("width"), height = root.value("height");
  if (!finiteNumber(width) || !finiteNumber(height) || width.toDouble() <= 0 || height.toDouble() <= 0 || width.toDouble() > 100000 || height.toDouble() > 100000)
    return fail(QStringLiteral("Board dimensions must be finite and between 0 and 100000 units."));
  if (!root.value("components").isArray() || !root.value("nets").isArray()) return fail(QStringLiteral("Components and nets must be arrays."));
  const auto componentArray = root.value("components").toArray(), netArray = root.value("nets").toArray();
  if (componentArray.isEmpty() || componentArray.size() > MaxRecords || netArray.size() > MaxRecords)
    return fail(QStringLiteral("A board requires 1–10000 components and at most 10000 nets."));
  QSet<QString> netNames, references;
  QVariantList nets, components;
  for (const auto &entry : netArray) {
    if (!entry.isObject()) return fail(QStringLiteral("Each net must be an object."));
    const auto n = entry.toObject(); const auto name = n.value("name").toString();
    if (!identifier(name) || netNames.contains(name)) return fail(QStringLiteral("Net names must be unique ASCII identifiers."));
    const bool hasReference = n.contains("referenceVoltage"), hasTolerance = n.contains("tolerance");
    if (hasReference != hasTolerance || (hasReference && (!finiteNumber(n.value("referenceVoltage")) || !finiteNumber(n.value("tolerance")) || std::abs(n.value("referenceVoltage").toDouble()) > 1000 || n.value("tolerance").toDouble() < 0 || n.value("tolerance").toDouble() > 1000)))
      return fail(QStringLiteral("Reference voltage and nonnegative absolute tolerance must be supplied together, within ±1000 V."));
    QVariantMap map{{"name", name}};
    if (hasReference) { map.insert("referenceVoltage", n.value("referenceVoltage").toDouble()); map.insert("tolerance", n.value("tolerance").toDouble()); }
    nets.append(map); netNames.insert(name);
  }
  for (const auto &entry : componentArray) {
    if (!entry.isObject()) return fail(QStringLiteral("Each component must be an object."));
    const auto c = entry.toObject(); const auto reference = c.value("reference").toString();
    if (!identifier(reference) || references.contains(reference) || !textField(c, "name", 160)) return fail(QStringLiteral("Components need unique references and a name."));
    if (!validPoint(c, width.toDouble(), height.toDouble()) || !finiteNumber(c.value("width")) || !finiteNumber(c.value("height")) || c.value("width").toDouble() <= 0 || c.value("height").toDouble() <= 0 || c.value("x").toDouble() + c.value("width").toDouble() > width.toDouble() || c.value("y").toDouble() + c.value("height").toDouble() > height.toDouble())
      return fail(QStringLiteral("Component geometry must be finite, positive and inside board bounds."));
    if (!c.value("nets").isArray() || c.value("nets").toArray().size() > MaxRecords) return fail(QStringLiteral("Component nets must be an array."));
    QStringList attached; QSet<QString> seen;
    for (const auto &net : c.value("nets").toArray()) {
      if (!net.isString() || !netNames.contains(net.toString()) || seen.contains(net.toString())) return fail(QStringLiteral("A component references an unknown or duplicate net."));
      attached.append(net.toString()); seen.insert(net.toString());
    }
    components.append(QVariantMap{{"reference", reference}, {"name", c.value("name").toString()}, {"x", c.value("x").toDouble()}, {"y", c.value("y").toDouble()}, {"width", c.value("width").toDouble()}, {"height", c.value("height").toDouble()}, {"nets", attached}});
    references.insert(reference);
  }
  // Commit only after the complete document is validated; failed imports keep current work intact.
  m_title = root.value("title").toString(); m_author = root.value("author").toString(); m_license = root.value("license").toString(); m_description = root.value("description").toString();
  m_width = width.toDouble(); m_height = height.toDouble(); m_components = components; m_nets = nets;
  m_sourcePath = resolved.startsWith(QLatin1Char(':')) ? resolved : QFileInfo(resolved).absoluteFilePath();
  m_digest = QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex());
  m_measurements.clear(); m_annotations.clear(); m_photos.clear(); m_dirty = false; m_loaded = true; clearError(); emit boardChanged(); emit sessionChanged(); return true;
}

QVariantList BoardBridge::search(const QString &query) const {
  const auto needle = query.trimmed(); QVariantList matches;
  for (const auto &value : m_components) {
    const auto c = value.toMap(); bool match = c.value("reference").toString().contains(needle, Qt::CaseInsensitive) || c.value("name").toString().contains(needle, Qt::CaseInsensitive);
    for (const auto &net : c.value("nets").toStringList()) match = match || net.contains(needle, Qt::CaseInsensitive);
    if (match) matches.append(value);
  }
  return matches;
}
bool BoardBridge::knownNet(const QString &net) const { for (const auto &n : m_nets) if (n.toMap().value("name").toString() == net) return true; return false; }
bool BoardBridge::knownReference(const QString &ref) const { for (const auto &c : m_components) if (c.toMap().value("reference").toString() == ref) return true; return false; }
QString BoardBridge::comparison(const QString &net, double value) const {
  for (const auto &n : m_nets) {
    const auto map = n.toMap();
    if (map.value("name").toString() == net && map.contains("referenceVoltage"))
      return std::abs(value - map.value("referenceVoltage").toDouble()) <= map.value("tolerance").toDouble() + 1e-12 ? QStringLiteral("within_reference") : QStringLiteral("outside_reference");
  }
  return QStringLiteral("no_reference");
}
bool BoardBridge::addMeasurement(const QString &net, double value, const QString &note) {
  if (!m_loaded || !knownNet(net) || !std::isfinite(value) || std::abs(value) > 1000 || note.size() > 2000 || m_measurements.size() >= MaxRecords) return fail(QStringLiteral("Choose a known net, a finite voltage within ±1000 V and a note up to 2000 characters."));
  m_measurements.append(QVariantMap{{"net", net}, {"value", value}, {"note", note}, {"timestamp", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)}, {"comparison", comparison(net, value)}});
  m_dirty = true; clearError(); emit sessionChanged(); return true;
}
bool BoardBridge::addAnnotation(const QString &reference, const QString &text, double x, double y) {
  if (!m_loaded || !knownReference(reference) || text.trimmed().isEmpty() || text.size() > 4000 || !std::isfinite(x) || !std::isfinite(y) || x < 0 || y < 0 || x > m_width || y > m_height || m_annotations.size() >= MaxRecords) return fail(QStringLiteral("Annotation requires an existing component, text up to 4000 characters and an in-bounds position."));
  m_annotations.append(QVariantMap{{"reference", reference}, {"text", text}, {"x", x}, {"y", y}, {"timestamp", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)}});
  m_dirty = true; clearError(); emit sessionChanged(); return true;
}
bool BoardBridge::addPhoto(const QString &reference, const QString &path) {
  if(!m_loaded || !knownReference(reference) || m_photos.size()>=MaxPhotos) return fail(QStringLiteral("Select a component; each session allows up to four photographs."));
  const auto resolved=localPath(path,false);
  if(resolved.isEmpty() || resolved.startsWith(QLatin1Char(':'))) return fail(QStringLiteral("Choose a local PNG or JPEG photograph."));
  QFile file(resolved);
  if(!file.open(QIODevice::ReadOnly) || file.size()>MaxPhotoInputBytes) return fail(QStringLiteral("Cannot read photo, or its input exceeds 4 MiB."));
  const auto bytes=file.read(MaxPhotoInputBytes+1);QByteArray normalized;QSize dimensions;
  if(file.error()!=QFileDevice::NoError || !normalizePhoto(bytes,false,normalized,dimensions)) return fail(QStringLiteral("Photo must be a valid PNG/JPEG up to 4096×4096, producing a PNG no larger than 2 MiB."));
  m_photos.append(QVariantMap{{"reference",reference},{"name",QFileInfo(resolved).fileName().left(240)},{"image",QStringLiteral("data:image/png;base64,")+QString::fromLatin1(normalized.toBase64())},{"width",dimensions.width()},{"height",dimensions.height()},{"timestamp",QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)}});
  m_dirty=true;clearError();emit sessionChanged();return true;
}
bool BoardBridge::removePhoto(int index) {
  if(index<0 || index>=m_photos.size()) return fail(QStringLiteral("Photograph not found."));
  m_photos.removeAt(index);m_dirty=true;clearError();emit sessionChanged();return true;
}
bool BoardBridge::saveSession(const QString &path) {
  if (!m_loaded) return fail(QStringLiteral("Load a board first."));
  const auto resolved = localPath(path, false);
  if (resolved.isEmpty() || resolved.startsWith(QLatin1Char(':'))) return fail(QStringLiteral("Choose a writable local session file."));
  const QFileInfo destination(resolved), source(m_sourcePath);
#ifdef Q_OS_WIN
  constexpr auto pathCase = Qt::CaseInsensitive;
#else
  constexpr auto pathCase = Qt::CaseSensitive;
#endif
  const bool samePath = QString::compare(destination.absoluteFilePath(), source.absoluteFilePath(), pathCase) == 0;
  const bool sameCanonicalPath = !destination.canonicalFilePath().isEmpty() &&
      QString::compare(destination.canonicalFilePath(), source.canonicalFilePath(), pathCase) == 0;
  if (samePath || sameCanonicalPath) return fail(QStringLiteral("A session cannot overwrite its board source."));
  QJsonObject root{{"format", "nexvary.board-session.v1"}, {"boardSha256", m_digest}, {"measurements", QJsonArray::fromVariantList(m_measurements)}, {"annotations", QJsonArray::fromVariantList(m_annotations)}, {"photos",QJsonArray::fromVariantList(m_photos)}};
  const auto bytes = QJsonDocument(root).toJson();
  if (bytes.size() > MaxSessionBytes) return fail(QStringLiteral("Session exceeds the 16 MiB limit; export fewer records."));
  QSaveFile file(resolved);
  if (!file.open(QIODevice::WriteOnly)) return fail(file.errorString());
  if (file.write(bytes) != bytes.size() || !file.commit()) return fail(file.errorString());
  m_dirty = false; clearError(); emit sessionChanged(); return true;
}
bool BoardBridge::loadSession(const QString &path) {
  if (!m_loaded) return fail(QStringLiteral("Load the matching board first."));
  const auto resolved = localPath(path, false); QJsonObject root; QByteArray bytes; QString problem;
  if (resolved.isEmpty() || !readJson(resolved, root, bytes, problem, MaxSessionBytes)) return fail(problem.isEmpty() ? QStringLiteral("Only local session files are supported.") : problem);
  if (root.value("format").toString() != QLatin1String("nexvary.board-session.v1") || root.value("boardSha256").toString() != m_digest || !root.value("measurements").isArray() || !root.value("annotations").isArray()) return fail(QStringLiteral("Session format or board fingerprint does not match."));
  const auto measures = root.value("measurements").toArray(), notes = root.value("annotations").toArray();
  if (measures.size() > MaxRecords || notes.size() > MaxRecords) return fail(QStringLiteral("Session has too many records."));
  if(root.contains("photos") && !root.value("photos").isArray()) return fail(QStringLiteral("Photos must be an array."));
  const auto photos=root.value("photos").toArray();if(photos.size()>MaxPhotos) return fail(QStringLiteral("At most four photographs are allowed."));
  QVariantList validatedMeasurements, validatedAnnotations, validatedPhotos;
  for (const auto &v : measures) {
    if (!v.isObject()) return fail(QStringLiteral("Invalid measurement record."));
    const auto o = v.toObject(); const auto net = o.value("net").toString();
    if (!knownNet(net) || !finiteNumber(o.value("value")) || std::abs(o.value("value").toDouble()) > 1000 || !textField(o, "note", 2000, false) || !textField(o, "timestamp", 64) || !QDateTime::fromString(o.value("timestamp").toString(), Qt::ISODateWithMs).isValid()) return fail(QStringLiteral("Invalid measurement record."));
    validatedMeasurements.append(QVariantMap{{"net", net}, {"value", o.value("value").toDouble()}, {"note", o.value("note").toString()}, {"timestamp", o.value("timestamp").toString()}, {"comparison", comparison(net, o.value("value").toDouble())}});
  }
  for (const auto &v : notes) {
    if (!v.isObject()) return fail(QStringLiteral("Invalid annotation record."));
    const auto o = v.toObject();
    if (!knownReference(o.value("reference").toString()) || !textField(o, "text", 4000) || !validPoint(o, m_width, m_height) || !textField(o, "timestamp", 64) || !QDateTime::fromString(o.value("timestamp").toString(), Qt::ISODateWithMs).isValid()) return fail(QStringLiteral("Invalid annotation record."));
    validatedAnnotations.append(QVariantMap{{"reference", o.value("reference").toString()}, {"text", o.value("text").toString()}, {"x", o.value("x").toDouble()}, {"y", o.value("y").toDouble()}, {"timestamp", o.value("timestamp").toString()}});
  }
  const QString prefix=QStringLiteral("data:image/png;base64,");
  for(const auto &value:photos) {
    if(!value.isObject()) return fail(QStringLiteral("Invalid photograph record."));
    const auto photo=value.toObject();const auto encoded=photo.value("image").toString();
    if(!knownReference(photo.value("reference").toString()) || !textField(photo,"name",240) || !textField(photo,"timestamp",64) || !QDateTime::fromString(photo.value("timestamp").toString(),Qt::ISODateWithMs).isValid() || !encoded.startsWith(prefix) || encoded.size()>prefix.size()+4*((MaxPhotoBytes+2)/3)) return fail(QStringLiteral("Invalid photograph metadata or image encoding."));
    const auto decoded=QByteArray::fromBase64Encoding(encoded.mid(prefix.size()).toLatin1(),QByteArray::AbortOnBase64DecodingErrors);
    QByteArray normalized;QSize dimensions;
    if(!decoded || decoded.decoded.size()>MaxPhotoBytes || !normalizePhoto(decoded.decoded,true,normalized,dimensions)) return fail(QStringLiteral("Invalid embedded PNG photograph or image bounds."));
    validatedPhotos.append(QVariantMap{{"reference",photo.value("reference").toString()},{"name",photo.value("name").toString()},{"image",prefix+QString::fromLatin1(normalized.toBase64())},{"width",dimensions.width()},{"height",dimensions.height()},{"timestamp",photo.value("timestamp").toString()}});
  }
  m_measurements = validatedMeasurements; m_annotations = validatedAnnotations; m_photos=validatedPhotos; m_dirty = false; clearError(); emit sessionChanged(); return true;
}
