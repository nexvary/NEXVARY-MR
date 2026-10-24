#pragma once
#include <QDateTime>
#include <QVariantMap>
#include <QVariantList>

namespace mr::diagnostics {
// Observation confidence describes a reported fact, never a probability that a
// cable, driver or handset is healthy. This engine has no I/O or execution API.
struct ConnectionSnapshot {
 QString selectedSerial;
 QVariantList devices;
 QVariantMap scans; // adb / fastboot: state and observedAtUtc
 QVariantMap usb;   // state, observedAtUtc and validated inventory
 QVariantMap profile;
 QDateTime now=QDateTime::currentDateTimeUtc();
};
inline constexpr qint64 connectionFreshnessMs=120000;
QVariantMap connectionDiagnosis(const ConnectionSnapshot &snapshot);
}
