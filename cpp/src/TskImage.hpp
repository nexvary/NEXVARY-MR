#pragma once
#include <QByteArray>
#include <QString>
#include <atomic>
#include <functional>
#include <memory>
namespace mr::forensic {
struct TskLimits {qint64 maximumImage=2LL*1024*1024*1024;int processTimeoutMs=60000;qsizetype maximumOutput=512*1024;};
struct TskResult {QString error,version,output;QByteArray sourceSha256,toolSha256;bool sourceUnchanged=false,cancelled=false;};
// User-selected, separately installed mmls only. A separate process and temporary
// working copy are not an OS sandbox; no privileges or raw device access are used.
TskResult analyzePartitions(const QString &image,const QString &executable,const std::shared_ptr<std::atomic_bool> &cancelled,
 const std::function<void(qint64,qint64)> &progress={},const TskLimits &limits={});
}
