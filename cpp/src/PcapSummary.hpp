#pragma once
#include <QByteArray>
#include <QJsonObject>
#include <QString>
#include <atomic>
#include <memory>
namespace mr::forensic {
inline constexpr qsizetype maximumCaptureBytes = 8 * 1024 * 1024;
struct PcapSummary {QJsonObject report; QString error;};
// Offline, bounded statistics from untrusted classic PCAP. No payload, address,
// process, live capture, credential extraction or decryption interface.
PcapSummary summarizeCapture(const QByteArray &bytes,
 const std::shared_ptr<std::atomic_bool> &cancelled = {});
}
