#include "CaseArtifacts.hpp"
#include "ImportAdapters.hpp"
#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QRegularExpression>
#include <QSet>
#include <algorithm>
#include <cmath>

namespace nexvary::law {
namespace {
QString hash(const QByteArray &bytes) {
    return QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex());
}
QString key(const QString &caseId, const QString &type, const QString &local) {
    return hash(QJsonDocument(QJsonArray{caseId, type, local}).toJson(QJsonDocument::Compact));
}
bool validHash(const QString &value) {
    static const QRegularExpression re(QStringLiteral("\\A[0-9a-f]{64}\\z"));
    return re.match(value).hasMatch();
}
bool identifier(const QString &value) { return !value.isEmpty() && value.size() <= 128; }
QVariantMap reject(const QString &code) { return {{"errorCode", code}}; }
// Reject ambiguous decoded JSON object keys before Qt silently chooses one.
bool uniqueKeys(const QByteArray &bytes) {
    QList<QSet<QString>> stack;
    for (qsizetype i = 0; i < bytes.size(); ++i) {
        const auto c = bytes[i];
        if (c == '{' || c == '[') {
            if (stack.size() >= 32) return false;
            stack.append(QSet<QString>());
        } else if (c == '}' || c == ']') {
            if (stack.isEmpty()) return false;
            stack.removeLast();
        } else if (c == '"') {
            const auto start = i++;
            bool ended = false;
            for (; i < bytes.size(); ++i) {
                if (bytes[i] == '\\') { ++i; continue; }
                if (bytes[i] == '"') { ended = true; break; }
            }
            if (!ended) return false;
            auto next = i + 1;
            while (next < bytes.size() && QByteArray(" \t\r\n").contains(bytes[next])) ++next;
            if (next < bytes.size() && bytes[next] == ':') {
                if (stack.isEmpty()) return false;
                const auto decoded = QJsonDocument::fromJson("[" + bytes.mid(start, i - start + 1) + "]");
                if (!decoded.isArray() || decoded.array().size() != 1 || !decoded.array()[0].isString()) return false;
                const auto name = decoded.array()[0].toString();
                if (stack.last().contains(name)) return false;
                stack.last().insert(name);
            }
        }
    }
    return stack.isEmpty();
}
struct Graph {
    QString caseId;
    QMap<QString, QVariantMap> nodes, edges;
    QString node(const QString &type, const QString &local, const QString &label) {
        const auto id = key(caseId, type, local);
        nodes.insert(id, {{"id", id}, {"type", type}, {"recordId", local}, {"label", label}, {"caseId", caseId}});
        return id;
    }
    void edge(const QString &from, const QString &to, const QString &type) {
        const auto id = key(caseId, type, from + ":" + to);
        edges.insert(id, {{"id", id}, {"from", from}, {"to", to}, {"type", type},
            {"basis", type == "records-sha256" ? "recorded-digest-only" : "recorded-provenance"}});
    }
    QVariantList nodeList() const { QVariantList out; for (const auto &v : nodes) out.append(v); return out; }
    QVariantList edgeList() const { QVariantList out; for (const auto &v : edges) out.append(v); return out; }
};
}

QVariantMap projectArtifacts(const QVariantMap &record, const QString &query,
    const std::shared_ptr<std::atomic_bool> &cancelled) {
    const auto stopped = [&] { return cancelled && cancelled->load(); };
    if (stopped()) return reject("cancelled");
    if (query.size() > 256) return reject("query-limit");
    const auto caseId = record.value("id").toString();
    const auto devices = record.value("devices").toList(), evidence = record.value("evidence").toList(), runs = record.value("analyses").toList();
    if (!identifier(caseId) || record.value("revision").toLongLong() < 1 || devices.size() > 100 || evidence.size() > 1000 || runs.size() > 100) return reject("snapshot-limit");
    for (const auto &field : {"devices", "evidence", "analyses"}) {
        if (record.contains(field) && record.value(field).metaType().id() != QMetaType::QVariantList) return reject("snapshot-schema");
    }
    QMap<QString, QVariantMap> sources, deviceMap;
    for (const auto &v : devices) {
        if (stopped()) return reject("cancelled");
        const auto d = v.toMap(); const auto id = d.value("id").toString();
        if (!identifier(id) || deviceMap.contains(id)) return reject("device-inventory");
        deviceMap.insert(id, d);
    }
    for (const auto &v : evidence) {
        if (stopped()) return reject("cancelled");
        const auto e = v.toMap(); const auto id = e.value("id").toString();
        if (!identifier(id) || sources.contains(id) || !deviceMap.contains(e.value("deviceId").toString())
            || !validHash(e.value("sha256").toString()) || e.value("name").toString().size() > 1024) return reject("evidence-inventory");
        sources.insert(id, e);
    }
    QVariantList rows;
    QSet<QString> seen;
    int observed = 0, matching = 0, omitted = 0, truncated = 0, rejected = 0, unsupported = 0;
    for (const auto &v : runs) {
        if (stopped()) return reject("cancelled");
        const auto run = v.toMap(); const auto runId = run.value("id").toString(), parser = run.value("parser").toString();
        if (!identifier(runId) || seen.contains(runId)) return reject("analysis-inventory");
        seen.insert(runId);
        if (!parser.startsWith("authorized-import/")) { ++unsupported; continue; }
        const auto adapter = parser.mid(18), eid = run.value("evidenceId").toString();
        const auto source = sources.value(eid); const auto bytes = run.value("reportJson").toString().toUtf8();
        if (source.isEmpty() || bytes.isEmpty() || bytes.size() > 64 * 1024
            || hash(bytes) != run.value("reportSha256") || run.value("sourceSha256") != source.value("sha256")
            || run.value("sourceSize") != source.value("size")) return reject("report-integrity");
        if (!uniqueKeys(bytes)) return reject("ambiguous-report-json");
        QJsonParseError error; const auto doc = QJsonDocument::fromJson(bytes, &error); const auto object = doc.object();
        const auto report = object.toVariantMap(), provenance = report.value("provenance").toMap();
        if (error.error != QJsonParseError::NoError || !doc.isObject() || report.value("schema") != "nexvary-authorized-import/v1"
            || !mr::forensic::importAdapterSupported(adapter) || report.value("adapter") != adapter || !object.value("parserVersion").isDouble() || object.value("parserVersion").toDouble() != 1
            || report.value("sourceSha256") != source.value("sha256") || report.value("sourceSize") != source.value("size")
            || provenance.value("caseId") != caseId || provenance.value("evidenceId") != eid
            || provenance.value("deviceId") != source.value("deviceId") || !object.value("findings").isArray()) return reject("report-schema-or-provenance");
        if (report.value("status") == "rejected") { ++rejected; continue; }
        if (report.value("status") != "imported" || !object.value("findingsTruncated").isBool()) return reject("report-status");
        const auto findings = object.value("findings").toArray();
        if (findings.size() > 128) return reject("findings-limit");
        const auto count = object.value("observedCount").toDouble(-1);
        if (!object.value("observedCount").isDouble() || !std::isfinite(count) || count < findings.size() || count > 4194304
            || count != static_cast<int>(count) || report.value("findingsTruncated").toBool() != (count > 128)) return reject("finding-count-schema");
        if (report.value("findingsTruncated").toBool()) ++truncated;
        for (qsizetype i = 0; i < findings.size(); ++i) {
            if (stopped()) return reject("cancelled");
            const auto f = findings[i].toObject();
            if (!findings[i].isObject() || !f.value("reference").isString() || !f.value("kind").isString()
                || !f.value("value").isString() || !f.value("valueTruncated").isBool()
                || f.value("reference").toString().size() > 3072 || f.value("value").toString().size() > 1536
                || f.value("kind").toString().isEmpty() || f.value("kind").toString().size() > 64) return reject("finding-schema");
            const auto kind = f.value("kind").toString(); QString memberHash;
            if (adapter == "adb-backup-v5" && kind == "adb-backup-member") {
                const auto type = f.value("entryType").toString();
                memberHash = f.value("memberSha256").toString();
                if (!f.value("memberSha256").isString() || (type != "file" && type != "directory") || (type == "file" && !validHash(memberHash))
                    || (type == "directory" && !memberHash.isEmpty())) return reject("member-digest-schema");
                const auto size = f.value("memberSize").toDouble(-1), offset = f.value("sourceByteOffset").toDouble(-1);
                if (f.value("valueTruncated").toBool() || f.value("value").toString() != (type == "file" ? memberHash : QString("directory"))
                    || !std::isfinite(size) || !std::isfinite(offset) || size < 0 || offset < 23
                    || size > 4194304 || offset > 4194304 || size != static_cast<int>(size) || offset != static_cast<int>(offset)
                    || size + offset > report.value("sourceSize").toDouble() || (type == "directory" && size != 0)) return reject("member-provenance-schema");
            } else if (adapter == "sqlite" && kind == "sqlite-blob-sha256" && !f.value("valueTruncated").toBool()) {
                memberHash = f.value("value").toString();
                if (!validHash(memberHash)) return reject("member-digest-schema");
            }
            ++observed;
            QVariantMap row{{"id", key(caseId, "finding", runId + ":" + QString::number(i))}, {"caseId", caseId},
                {"analysisId", runId}, {"ordinal", i}, {"evidenceId", eid}, {"deviceId", source.value("deviceId")},
                {"sourceName", source.value("name")}, {"sourceSha256", source.value("sha256")}, {"reportSha256", run.value("reportSha256")},
                {"adapter", adapter}, {"kind", kind}, {"reference", f.value("reference").toString()}, {"value", f.value("value").toString()},
                {"valueTruncated", f.value("valueTruncated").toBool()}, {"memberSha256", memberHash}};
            bool found = query.isEmpty();
            if (!found) for (const auto &field : {"id", "sourceName", "reference", "value", "kind", "adapter", "evidenceId", "analysisId", "deviceId", "sourceSha256", "reportSha256", "memberSha256"}) {
                if (row.value(field).toString().contains(query, Qt::CaseInsensitive)) { found = true; break; }
            }
            if (!found) continue;
            ++matching;
            if (rows.size() >= artifactRowLimit) { ++omitted; continue; }
            rows.append(row);
        }
    }
    std::sort(rows.begin(), rows.end(), [](const QVariant &a, const QVariant &b) { return a.toMap().value("id").toString() < b.toMap().value("id").toString(); });
    Graph graph{caseId, {}, {}};
    QMap<QString, QMap<QString, QVariantMap>> digestMembers;
    const auto caseNode = graph.node("case", caseId, caseId);
    for (qsizetype index = 0; index < rows.size(); ++index) {
        if (stopped()) return reject("cancelled");
        auto row = rows[index].toMap(); const auto eid = row.value("evidenceId").toString(), did = row.value("deviceId").toString();
        const auto d = graph.node("device", did, did), e = graph.node("evidence", eid, row.value("sourceName").toString());
        const auto a = graph.node("analysis", row.value("analysisId").toString(), row.value("analysisId").toString());
        const auto f = row.value("id").toString();
        graph.nodes.insert(f, {{"id", f}, {"type", "finding"}, {"recordId", row.value("analysisId").toString() + ":" + row.value("ordinal").toString()}, {"label", row.value("reference")}, {"caseId", caseId}});
        graph.edge(caseNode, d, "registers-device"); graph.edge(d, e, "registers-evidence");
        graph.edge(e, a, "input-to-analysis"); graph.edge(a, f, "reports-finding");
        row.insert("provenancePath", QVariantList{graph.nodes.value(caseNode), graph.nodes.value(d), graph.nodes.value(e), graph.nodes.value(a), graph.nodes.value(f)});
        rows[index] = row;
        for (const auto &pair : {qMakePair(e, row.value("sourceSha256").toString()), qMakePair(f, row.value("memberSha256").toString())}) {
            if (!pair.second.isEmpty()) {
                graph.edge(pair.first, graph.node("recorded-sha256", pair.second, pair.second), "records-sha256");
                digestMembers[pair.second].insert(pair.first, {{"nodeId", pair.first}, {"type", pair.first == e ? "evidence" : "finding"},
                    {"evidenceId", eid}, {"analysisId", pair.first == e ? QString() : row.value("analysisId").toString()},
                    {"reference", pair.first == e ? row.value("sourceName") : row.value("reference")}});
            }
        }
    }
    QVariantList groups;
    for (auto it = digestMembers.cbegin(); it != digestMembers.cend(); ++it) {
        if (stopped()) return reject("cancelled");
        if (it.value().size() < 2) continue;
        QVariantList members;
        for (const auto &member : it.value()) if (members.size() < 64) members.append(member);
        groups.append(QVariantMap{{"sha256", it.key()}, {"memberCount", it.value().size()}, {"members", members},
            {"omittedMembers", it.value().size() - members.size()}, {"basis", "equal-recorded-digest-only"}});
    }
    if (graph.nodes.size() > 8192 || graph.edges.size() > 12288) return reject("graph-limit");
    return {{"schema", "nexvary-case-artifacts/v1"}, {"caseId", caseId}, {"caseRevision", record.value("revision")},
        {"synthetic", record.value("synthetic")}, {"query", query}, {"rows", rows}, {"nodes", graph.nodeList()}, {"edges", graph.edgeList()}, {"digestGroups", groups},
        {"observedCount", observed}, {"matchingCount", matching}, {"omittedMatchingFindings", omitted}, {"truncatedImportReports", truncated},
        {"rejectedImportReports", rejected}, {"unsupportedAnalysisReports", unsupported}, {"rowLimit", artifactRowLimit},
        {"scope", "Retained import observations and recorded provenance only. Graph covers retained matches; equal recorded digests are claims, not current byte verification, people, events or causality. No semantic/Arabic normalization or full device/app indexing."}};
}
}
