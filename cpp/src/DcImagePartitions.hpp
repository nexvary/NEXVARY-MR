#pragma once
#include "ForensicAdapters.hpp"
namespace mr::forensic {
// Byte-only derivative of Disk Care Partitions.cpp. No QFile/disk or writer API.
ForensicAdapterResult inspectDcPartitions(const ReadOnlyEvidenceRequest &,QVariantMap report);
}
