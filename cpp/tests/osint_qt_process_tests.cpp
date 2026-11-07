#include "../src/osint/OsintQtProcessRunner.hpp"
#include <QCoreApplication>
#include <atomic>
#include <cstdlib>
int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    std::atomic_bool cancelled{false};
    mr::osint::ProcessLaunchPlan plan{"subfinder", "/nonexistent/subfinder",
        std::string(64, 'a'), {"-d", "example.org"}, "TEST-CASE", false, 5, 4096};
    const auto denied = mr::osint::runPinnedTool(plan, cancelled);
    if (denied.completed || denied.error != "policy-denied") return EXIT_FAILURE;
    plan.approvedForExternalDisclosure = true;
    const auto missing = mr::osint::runPinnedTool(plan, cancelled);
    if (missing.completed || missing.error != "untrusted-executable") return EXIT_FAILURE;
    cancelled.store(true);
    const auto stopped = mr::osint::runPinnedTool(plan, cancelled);
    if (stopped.completed || stopped.error != "cancelled") return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
