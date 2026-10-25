# Protected-backup increment validation

Synthetic data only. The source revision is the commit containing this folder;
GitHub source/CI pairing is recorded in PR #15. Previous source cb1f03a passed
all seven hosted native jobs in run 40146396789; its packages do not contain this
new protected-backup feature.

Final local checks: final-ctest.txt/XML, hash-font-build.txt, hash-font-native.txt,
final-ui-complete.txt, python.txt, gitleaks-dir.txt and gitleaks-regression.txt.
Native case/identity counts: 17/12; complete Qt UI: 43. backup-ar/en.png are real
native Qt captures. pdf-qa.json records native synthetic five-page PDF visual and
independent text checks, including a literal ASCII ff digest (no Unicode
normalization). The temporary PDF contained no real case data and is not a release.

Keep initial-ctest.xml and strict-build-initial-failed.txt: they document generated
ELF permission failures and a corrected compiler warning. strict-native-before-
rebuild.txt is stale output from the initial failed build and is NOT final proof.
retry-ctest.* records the successful affected-group retry before the final full run.

Production uses Windows current-user DPAPI; synthetic Linux tests deliberately do
not claim protected production storage or cross-account disaster recovery. Read
../../docs/PROTECTED_CASE_BACKUP.md and ../../docs/GOVERNMENT_READINESS.md.
