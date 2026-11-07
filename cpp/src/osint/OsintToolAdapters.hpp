#pragma once
#include "OsintQtProcessRunner.hpp"
#include "OsintOfflineParsers.hpp"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QVariantList>
#include <QVariantMap>

namespace mr::osint {
struct AdapterResult {
    bool completed = false;
    QString error;
    QVariantList findings;
};
inline AdapterResult parseExifToolJson(const QByteArray& data) {
    AdapterResult out;
    if (data.size() > 1024 * 1024) { out.error = "output-limit"; return out; }
    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError || !doc.isArray()) {
        out.error = "invalid-exiftool-json"; return out;
    }
    const auto items = doc.array();
    if (items.size() > 100) { out.error = "too-many-files"; return out; }
    for (const auto& item : items) {
        if (!item.isObject()) { out.error = "invalid-exiftool-record"; out.findings.clear(); return out; }
        const auto obj = item.toObject();
        if (!obj.value("SourceFile").isString() || obj.value("SourceFile").toString().isEmpty()) {
            out.error = "missing-source-file"; out.findings.clear(); return out;
        }
        QVariantMap record;
        record.insert("sourceId", "exiftool");
        record.insert("sourceFile", obj.value("SourceFile").toString());
        record.insert("classification", "unverified-local-metadata");
        record.insert("metadata", obj.toVariantMap());
        out.findings.push_back(record);
    }
    out.completed = true;
    return out;
}
inline AdapterResult parseSubfinderOutput(const QByteArray& data, const QString& domain) {
    AdapterResult out;
    const auto parsed = parseSubfinderLines(
        std::string_view(data.constData(), static_cast<std::size_t>(data.size())),
        domain.toStdString());
    if (!parsed.error.empty()) { out.error = QString::fromStdString(parsed.error); return out; }
    for (const auto& lead : parsed.leads)
        out.findings.push_back(QVariantMap{{"sourceId", "subfinder"},
                                           {"value", QString::fromStdString(lead.value)},
                                           {"classification", "unverified-lead"}});
    out.completed = true;
    return out;
}
inline AdapterResult runExifTool(const ProcessLaunchPlan& plan, const std::atomic_bool& cancelled) {
    AdapterResult out;
    if (plan.sourceId != "exiftool") { out.error = "provider-mismatch"; return out; }
    if (plan.arguments.size() != 3 || plan.arguments[0] != "-json" ||
        plan.arguments[1] != "--" || plan.arguments[2].empty() ||
        plan.arguments[2][0] == '-' ||
        !QFileInfo(QString::fromStdString(plan.arguments[2])).isAbsolute()) {
        out.error = "unexpected-arguments"; return out;
    }
    const auto run = runPinnedTool(plan, cancelled);
    if (!run.completed) { out.error = run.error; return out; }
    return parseExifToolJson(run.standardOutput);
}
inline AdapterResult runSubfinder(const ProcessLaunchPlan& plan, const QString& domain,
                                  const std::atomic_bool& cancelled) {
    AdapterResult out;
    if (plan.sourceId != "subfinder" || !safeDomain(domain.toStdString())) {
        out.error = "invalid-provider-or-scope"; return out;
    }
    // Prevent caller-supplied arguments from changing the authorized search scope.
    if (plan.arguments.size() != 3 || plan.arguments[0] != "-silent" ||
        plan.arguments[1] != "-d" || plan.arguments[2] != domain.toStdString()) {
        out.error = "unexpected-arguments"; return out;
    }
    const auto run = runPinnedTool(plan, cancelled);
    if (!run.completed) { out.error = run.error; return out; }
    return parseSubfinderOutput(run.standardOutput, domain);
}
} // namespace mr::osint
