# Retained local first observations

- First warnings-as-errors build rejected a misleadingly indented one-line test
  fixture `if` followed by `source.close()`. The statement was split; no warning
  or gate was disabled. Original `build-first-warning.txt` is retained.
- I overlapped two local builds using the same output directory. One link failed
  with `undefined reference to main` for the cloud test. The overlap was stopped,
  and subsequent builds were serialized. The original build log is retained.
  This is not evidence of a product or audit-store root cause. The final build
  and test receipts must be assessed separately; earlier failures are not green.
- First native partition suite: 25 passed / 1 failed / 0 skipped. The first test
  requested a non-UUID case reference and incorrectly expected approval; the
  existing policy correctly refused that request. The fixture now requests an
  otherwise valid UUID for a different case, then verifies the case adapter
  refuses it. Policy validation and fail-closed conditions were not weakened.
- The first full local suite uses that earlier native fixture. It is retained
  separately from the corrected final-source run. Additional pre-dispatch case
  snapshot and post-receipt session checks/tests were added after that build.
- Existing audit snapshot-lag cause and detached-descendant counterexamples stay
  open. A fresh smoke fixture, if needed, cannot erase earlier failed receipts.

First full CTest: **47/50**, 401.78 seconds. The earlier invalid-reference fixture
failed. The TSK test and GUI smoke were **Not Run**: both generated executables
were observed as mode 0644, size zero (metadata retained). Those are failed/not-run
observations, not passing product tests. Generated outputs are rebuilt before the
final source run; no source regression or gate is removed. Qt Quick53 and both
DPI groups passed in this first observation. Original full console/XML retained.

An additional interoperability launch failed with PermissionError: the generated
policy-test ELF had size3,623,496 bytes but mode0644. Only executable bits on three
generated ELF outputs were restored (metadata in generated-mode-repairs.json),
without changing their bytes or source. The corrected interoperability check69/69
passed. The failed launch is retained; no product/audit root cause is inferred.
The earlier synthetic smoke directory was preserved outside Git; final smoke uses
a fresh isolated synthetic fixture, without erasing any old failed observation.
