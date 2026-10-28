#pragma once
#include <QByteArray>
#include <QDeadlineTimer>
#include <QVariantMap>
#include <QStringList>
#include <atomic>
#include <memory>

namespace mr::forensic {
// Only protected, hash-verified case bytes cross this boundary. No path, command,
// disk handle, updater, second evidence database or caller-supplied authorization.
struct ForensicSourceDescriptor {
 QString adapter, repository, upstreamCommit, sourceModule, permissionBasis;
 int maxInputBytes=8*1024*1024, maxOutputBytes=64*1024, maxFindings=128;
 QVariantMap toMap() const;
};
struct ManifestEvidencePayload {
 QString evidenceId, sha256;
 QByteArray bytes;
};
struct ReadOnlyEvidenceRequest {
 QString caseId,evidenceId,sourceSha256,adapter;
 QByteArray verifiedBytes;
 std::shared_ptr<std::atomic_bool> cancelled;
 QDeadlineTimer deadline{5000};
 bool verifyManifestPayloads=false;
 QMap<QString,ManifestEvidencePayload> manifestPayloads{};
};
struct ForensicFinding {
 QString kind,reference,value,confidence="source-reported-unverified";
 QVariantMap details;
 QVariantMap toMap() const;
};
struct ForensicAdapterResult {
 QVariantMap report;
 QString errorCode;
 bool ok() const {return errorCode.isEmpty()&&!report.isEmpty();}
};
// This reference is filled by the existing signed case-operation transaction,
// never minted by an untrusted parser or treated as institutional accreditation.
struct ForensicOperationReceipt {
 QString operationId,analysisId,reportSha256;
 QVariantMap toMap() const {return {{"operationId",operationId},{"analysisId",analysisId},{"reportSha256",reportSha256},{"authority",operationId.isEmpty()?"synthetic-unsigned-observation":"existing-signed-case-operation"}};}
};
// Portable logical package names only; never resolved as filesystem paths.
bool safeManifestPath(const QString &path);
QStringList forensicAdapters();
ForensicSourceDescriptor forensicSource(const QString &adapter);
ForensicAdapterResult analyzeReadOnlyEvidence(const ReadOnlyEvidenceRequest &request);
// Validate a retained, case-bound report before projecting findings/timestamps.
ForensicAdapterResult retainedForensicReport(const QVariantMap &run,const QVariantMap &source,const QString &caseId);
}
