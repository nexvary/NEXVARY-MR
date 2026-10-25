#include "AdbBackupImport.hpp"
#include "ImportAdapters.hpp"
#include <QCryptographicHash>
#include <QMap>
#include <QRegularExpression>
#include <QStringDecoder>
#include <limits>

namespace mr::forensic {
namespace {
constexpr int memberLimit = 4096;
bool zeroes(const QByteArray &bytes) {
    for (const auto c : bytes) if (c != '\0') return false;
    return true;
}
bool octal(const QByteArray &field, quint64 &value) {
    value = 0;
    bool digit = false, ended = false;
    for (const auto c : field) {
        if (c >= '0' && c <= '7' && !ended) {
            if (value > (std::numeric_limits<quint64>::max() - 7) / 8) return false;
            value = value * 8 + static_cast<quint64>(c - '0');
            digit = true;
        } else if (c == '\0' || c == ' ') {
            if (digit || c == '\0') ended = true;
        } else return false;
    }
    return digit;
}
bool stringField(const QByteArray &field, QString &value) {
    const auto end = field.indexOf('\0');
    if (end >= 0 && !zeroes(field.mid(end))) return false;
    const auto raw = end < 0 ? field : field.left(end);
    QStringDecoder decoder(QStringDecoder::Utf8);
    value = decoder(raw);
    if (decoder.hasError() || value.toUtf8() != raw) return false;
    for (const auto c : value) {
        if (c.unicode() < 32 || c.unicode() == 127 || c.category() == QChar::Other_Format) return false;
    }
    return true;
}
bool appPath(QString &path, bool directory, QString &package, QString &domain) {
    if (directory && path.endsWith('/')) path.chop(1);
    if (path.isEmpty() || path.toUtf8().size() > 255 || path.contains('\\') || path.contains(':')) return false;
    const auto parts = path.split('/');
    for (const auto &part : parts) if (part.isEmpty() || part == "." || part == "..") return false;
    static const QRegularExpression packageName("^[A-Za-z][A-Za-z0-9_]*(\\.[A-Za-z][A-Za-z0-9_]*)+$");
    if (parts.size() < 3 || parts[0] != "apps" || !packageName.match(parts[1]).hasMatch()) return false;
    package = parts[1]; domain = parts[2];
    // Metadata files have no domain. Other names are inventoried literally,
    // without claiming knowledge of application or vendor semantics.
    if (!directory && parts.size() == 3 && domain != "_manifest" && domain != "_meta") return false;
    return true;
}
}
bool inventoryAdbBackup(const QByteArray &bytes, QVariantList &findings,
                        int &observedCount, QVariantMap &summary) {
    findings.clear(); observedCount = 0; summary.clear();
    const QByteArray header("ANDROID BACKUP\n5\n0\nnone\n");
    if (!bytes.startsWith(header) || bytes.size() > importInputLimit) return false;
    const auto tar = bytes.mid(header.size());
    if (tar.size() < 1024 || tar.size() % 512 != 0) return false;
    QMap<QString, bool> paths;
    qsizetype cursor = 0;
    int files = 0, directories = 0;
    quint64 totalBytes = 0;
    bool terminated = false;
    while (cursor + 512 <= tar.size()) {
        const auto block = tar.mid(cursor, 512);
        if (zeroes(block)) {
            if (tar.size() - cursor < 1024 || !zeroes(tar.mid(cursor))) return false;
            terminated = true; break;
        }
        if (++observedCount > memberLimit || block.mid(257, 8) != QByteArray("ustar\0" "00", 8)) return false;
        quint64 expected = 0, size = 0, mtime = 0, ignored = 0, checksum = 0;
        if (!octal(block.mid(148, 8), expected) || !octal(block.mid(124, 12), size)
            || !octal(block.mid(136, 12), mtime) || !octal(block.mid(100, 8), ignored)
            || !octal(block.mid(108, 8), ignored) || !octal(block.mid(116, 8), ignored)) return false;
        for (int i = 0; i < 512; ++i) checksum += i >= 148 && i < 156 ? 32 : static_cast<unsigned char>(block[i]);
        if (checksum != expected) return false;
        const bool directory = block[156] == '5';
        if ((!directory && block[156] != '0') || (directory && size != 0) || !zeroes(block.mid(157, 100))) return false;
        QString path, prefix, package, domain;
        if (!stringField(block.left(100), path) || !stringField(block.mid(345, 155), prefix)) return false;
        if (!prefix.isEmpty()) path = prefix + '/' + path;
        if (!appPath(path, directory, package, domain) || paths.contains(path)) return false;
        paths.insert(path, directory);
        const auto dataOffset = cursor + 512;
        // Check before conversion/addition: archive size is hostile input.
        if (size > static_cast<quint64>(tar.size() - dataOffset)) return false;
        const auto length = static_cast<qsizetype>(size);
        const auto padding = (512 - length % 512) % 512;
        if (length + padding > tar.size() - dataOffset || !zeroes(tar.mid(dataOffset + length, padding))) return false;
        const auto digest = directory ? QString() : QString::fromLatin1(QCryptographicHash::hash(tar.mid(dataOffset, length), QCryptographicHash::Sha256).toHex());
        if (findings.size() < importFindingLimit) {
            findings.append(QVariantMap{{"reference", path}, {"kind", "adb-backup-member"},
                {"value", directory ? QString("directory") : digest}, {"valueTruncated", false},
                {"package", package}, {"domainToken", domain}, {"entryType", directory ? "directory" : "file"},
                {"memberSize", size}, {"memberSha256", digest},
                {"sourceByteOffset", header.size() + dataOffset},
                {"rawMtimeEpochSeconds", QString::number(mtime)}});
        }
        if (directory) ++directories; else ++files;
        totalBytes += size;
        cursor = dataOffset + length + padding;
    }
    if (!terminated || observedCount == 0) return false;
    // A regular member cannot also be an ancestor of another member.
    for (auto it = paths.cbegin(); it != paths.cend(); ++it) {
        auto parent = it.key();
        while (parent.contains('/')) {
            parent = parent.left(parent.lastIndexOf('/'));
            if (paths.contains(parent) && !paths.value(parent)) return false;
        }
    }
    summary = {{"format", "Android ADB backup"}, {"version", 5}, {"compressed", false},
        {"encrypted", false}, {"inventoryOnly", true}, {"files", files}, {"directories", directories},
        {"membersScanned", observedCount}, {"memberLimit", memberLimit}, {"memberBytes", totalBytes},
        {"manifestAuthenticity", "not established"}, {"mtimeInterpretation", "raw untrusted epoch seconds; not event time"}};
    return true;
}
}
