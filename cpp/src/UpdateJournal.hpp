#pragma once
#include "UpdateVerifier.hpp"
#include <array>
#include <functional>

namespace mr::update {
// Supplied independently from the journal, never bootstrapped from its contents.
// A protected revision floor is necessary to detect replay of an older valid MAC.
struct JournalAnchor {
  QString installationId;
  InstalledState installed;
  quint64 revisionFloor = 0;
};
// Internal checkpoint ledger, not an installer. Trusted native observations and
// independently protected key/anchor provisioning remain caller obligations.
class Journal final {
public:
  Journal(QString approvedDirectory, const QByteArray &authenticationKey, JournalAnchor anchor);
  ~Journal();
  Journal(const Journal &) = delete;
  Journal &operator=(const Journal &) = delete;
  CheckResult read() const;
  CheckResult initialize(); // Missing only; never repairs/replaces existing state.
  CheckResult prepare(quint64 expectedRevision, StagedPackage &snapshot,
                      const TrustedKeys &currentKeys, const QDateTime &trustedNow);
  CheckResult startApplying(quint64 expectedRevision, StagedPackage &snapshot,
                            const TrustedKeys &currentKeys, const QDateTime &trustedNow);
  CheckResult startVerifying(quint64 expectedRevision);
  // This observation must come from a trusted native postcondition verifier.
  // Journal records the supplied observation; it cannot prove installation itself.
  CheckResult confirmInstalled(quint64 expectedRevision, const InstalledState &trustedObserved,
                               StagedPackage &snapshot, const TrustedKeys &currentKeys,
                               const QDateTime &trustedNow);
  CheckResult discardPrepared(quint64 expectedRevision);
  CheckResult recordRestoredBaseline(quint64 expectedRevision, const InstalledState &trustedObserved);
private:
  using Mutation = std::function<QString(QJsonObject &)>;
  CheckResult access(bool initialize, const quint64 *expectedRevision, const Mutation &mutation) const;
  QString directory_;
  JournalAnchor anchor_;
  mutable quint64 observedRevision_ = 0;
  mutable InstalledState observedInstalled_;
  std::array<unsigned char,32> key_{};
  bool keyValid_ = false;
};
}
