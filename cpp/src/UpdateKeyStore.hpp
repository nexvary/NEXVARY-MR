#pragma once
#include "UpdateJournal.hpp"
namespace mr::update {
struct JournalHandle {
  QString error;
  std::unique_ptr<Journal> journal;
  bool accepted() const { return error.isEmpty() && journal != nullptr; }
};
// Current-user Windows DPAPI through the shared project protection provider.
// This provisions only a local journal MAC key, never a publisher signing key.
class KeyStore final {
public:
  static bool available();
  static CheckResult create(const QString &approvedDirectory,const JournalAnchor &trustedBootstrap);
  // A loaded key handle is not a validated ledger: call read()/initialize() and
  // enforce the independently protected anchor before any transaction.
  static JournalHandle openJournal(const QString &approvedDirectory,const JournalAnchor &trustedAnchor);
};
}
