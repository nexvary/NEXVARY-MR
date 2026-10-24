#pragma once
#include <QDateTime>
#include <QVariantMap>
#include <QVariantList>
namespace mr::diagnostics {
struct DeviceReading {
 QString selectedSerial,propertiesObservedAtUtc;
 QVariantList devices;
 QVariantMap properties,profile;
 QVariantList services;
 QVariantMap connection;
 QDateTime now=QDateTime::currentDateTimeUtc();
};
// Presentation projection only. This API neither approves nor executes a service.
QVariantMap deviceIntelligence(const DeviceReading &reading);
}
