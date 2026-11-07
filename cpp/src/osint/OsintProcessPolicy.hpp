#pragma once
#include "OsintSourceRegistry.hpp"
#include "OsintOfflineParsers.hpp"
#include <string>
#include <string_view>
#include <vector>

namespace mr::osint {
struct ProcessLaunchPlan {
    std::string sourceId;
    std::string executablePath;
    std::string executableSha256;
    std::vector<std::string> arguments;
    std::string caseId;
    bool approvedForExternalDisclosure = false;
    unsigned timeoutSeconds = 30;
    std::size_t maximumOutputBytes = 1024 * 1024;
};
inline bool validSha256Hex(std::string_view value) {
    if (value.size() != 64) return false;
    for (char c : value) {
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
              (c >= 'A' && c <= 'F'))) return false;
    }
    return true;
}
// Pure policy check. Actual process launch must separately verify the binary
// digest, avoid a shell, enforce OS isolation, cancellation and output caps.
inline bool mayLaunchProcess(const ProcessLaunchPlan& plan) {
    const auto* source = findSource(plan.sourceId);
    if (!source || source->mode != IntegrationMode::LocalExecutable ||
        !source->requiresCaseAuthorization || plan.caseId.empty() || plan.caseId.find_first_not_of(" \t\r\n") == std::string::npos ||
        plan.executablePath.empty() || plan.executablePath.find_first_of("\r\n") != std::string::npos || !validSha256Hex(plan.executableSha256) ||
        plan.timeoutSeconds == 0 || plan.timeoutSeconds > 120 ||
        plan.maximumOutputBytes == 0 || plan.maximumOutputBytes > 1024 * 1024 ||
        plan.arguments.size() > 32 ||
        (source->externalDisclosure && !plan.approvedForExternalDisclosure))
        return false;
    for (const auto& arg : plan.arguments) {
        if (arg.empty() || arg.size() > 4096 || arg.find('\0') != std::string::npos)
            return false;
    }
    return true;
}
} // namespace mr::osint
