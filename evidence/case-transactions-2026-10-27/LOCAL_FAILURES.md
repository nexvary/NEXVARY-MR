# Preserved local build/runner failures

- First new test build failed `-Werror=misleading-indentation` at the fixture source: `if(!f.open(...)||...)qFatal(...);f.close();`. Fixed the statement layout; warnings-as-errors remained enabled. The original diagnostic was returned by the tool; no runtime source workaround was used.
- One generated `ConnectionDiagnosis.cpp.o` was observed as exactly 0 bytes. The initial link failure is retained in `local-link-failure.txt`. Removed that generated object and rebuilt from unchanged source. This is a local build artifact observation, not a product root-cause finding.
- Final local linking also left three zero-byte generated executables (`mr_policy_audit_tests`, `mr_offline_tests`, `nexvary_mr`) and removed executable mode on two nonempty ELF test binaries. Initial full CTest reported policy/offline as Not Run / permission denied. The three generated targets were relinked and ELF executable mode restored, with no source/test-condition change. Preserve the initial CTest log/XML when it completes and run the complete suite again on repaired artifacts.
- A `/proc`-based attempt to interrupt the incomplete run could not signal its reported PID (ProcessLookupError). The original CTest run continued; it was not relabelled passed.

The package installation of standalone ClamAV CLI had service post-install configuration errors in this managed container; the existing scanner fixture uses a temporary synthetic database. No system scanner service or external database update is claimed.

## Initial complete regression

Initial CTest completed 43/49. Two tests were Not Run because of generated zero-byte files; full QML and both DPI groups failed because the new required operation-policy connection was missing in the native UI test fixture (main.cpp already connected it). The fixture now uses the same LawCaseBridge::setOperationPolicy wiring; no QML/layout assertions were removed or relaxed. Full original console/XML are retained.

The main synthetic GUI smoke failed at policy-audit-export, class audit-digest, in-memory sequence 22. Original private synthetic resources were copied before any rerun. The retained file has a valid synthetic wrapper and a valid signed prefix of **18 events**, not 22. The sanitized observation/digest is retained; raw secret-bearing journal/identity snapshots are not published. The mechanism causing the snapshot lag remains unresolved.

A new post-replacement readback equality gate refuses to acknowledge a divergent/unreadable committed snapshot without retry or overwrite. A deterministic native test injects the older valid snapshot after replacement and proves refusal and restart-only recovery. This tests the readback postcondition; it does not establish or fix the original rollback mechanism.

## Final-source fixture isolation

Final-source full CTest: 48/49. Actual retained policy rejection was "Close the existing request for this category before requesting again." An earlier failed synthetic run had left a nonterminal request in the reused smoke data. Its original resources were preserved privately and were not repaired or silently closed. Fresh isolated smoke exited0; a new explicitly synthetic CTest fixture then passed three sequential smokes (60.94s). Both the failed full run and the isolated passes are retained. Independently hosted first-attempt Push/PR full suites passed49/49. This does not erase or resolve the earlier audit snapshot-lag mechanism.

The first ad-hoc Windows archive comparison used ZIP '/' names against unnormalized Windows '\' manifest names and correctly stopped on a set mismatch. After explicitly normalizing only path separators, all1,419 exact byte hashes and the complete file set matched; no package bytes were changed.
