#pragma once
#include <QByteArray>
#include <QList>
#include <QString>
#include <atomic>
#include <functional>
#include <memory>

namespace mr::recovery {
struct Candidate {
  qint64 offset = 0;
  QString format;
  QByteArray bytes;
  QByteArray sha256; // Raw 32-byte SHA-256; use toHex() for presentation.
};
struct Options {
  qint64 maximumInputBytes = 2LL * 1024 * 1024 * 1024;
  qint64 maximumCandidateBytes = 8LL * 1024 * 1024;
  int maximumCandidates = 100;
  qint64 maximumAggregateBytes = 64LL * 1024 * 1024;
  int maximumSignatureAttempts = 10000;
  qint64 chunkBytes = 64 * 1024;
};
struct Result {
  QList<Candidate> candidates;
  QByteArray sourceSha256;
  qint64 scannedBytes = 0; // Signature-scan pass, independent of hashing passes.
  int signatureAttempts = 0;
  QString error;
  bool cancelled = false;
  bool sourceUnchanged = false;
  bool limited = false;
};
using Progress = std::function<void(qint64 processedBytes, qint64 totalBytes)>;
// Original contiguous signature carving, not a filesystem/fragment reconstruction parser.
// Existing regular local .dd/.img/.raw/.bin files only. No writes, device handles,
// partition access, decryption, EWF, Android sparse images or filesystem claims.
// Progress is monotonic actual bytes consumed by prehash + scan + posthash (3*size).
// Bounded candidate inspection lookahead is deliberately excluded from this counter.
// Cancellation/error/source changes release all candidates; resource limits set limited.
Result scanImage(const QString &localPathOrFileUrl, const Options &options = {},
                 const std::shared_ptr<std::atomic_bool> &cancel = {},
                 const Progress &progress = {});
}
