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

Final-source full local observation: **49/50**, 455.46 seconds. All executed
groups passed, including the partition suite, TSK, Qt Quick, both DPI groups
and fresh GUI smoke. Compatibility was Not Run: another generated executable
was observed as zero bytes/mode0644. Its original metadata and complete failed
full run are retained. Only that generated zero-byte output was removed and
relinked from the existing object; targeted compatibility then passed1/1. This
is **not relabelled as a clean full local50/50 run**. Subsequent GUI smoke repeated
three times successfully (21.88/22.41/23.17 seconds, total67.47); original audit
transient was not reproduced in those observations and its cause remains open.

The `mr_ui*-final.txt` files are CTest passing-output excerpts extracted from
the retained JUnit XML, subject to CTest output truncation. They are not complete
per-case count evidence. Hosted original logs must establish full UI/DPI totals.

## First hosted Windows compile failure

Source7dc5fa9, Push40588628600, Windows Production job122486702812 failed
MSVC C4310/C2220 in the new fixture's constant signed-char casts. The complete
decoded original log is retained (CRLF normalized for the text artifact).
The correction uses explicit Hex byte ranges for the same MBR bytes, with no
warning suppression or assertion/gate change. New commit/CI results must be
assessed independently; other original jobs may finish or be superseded.

## First hosted layout failure retained

Source7dc Push Linux Production job122486702664: full49/50 with125% SIGSEGV
3.34 seconds after governmentAdbBackupControls. One original GDB diagnostic
reproduced SIGSEGV/exit255 in qmlAttachedPropertiesObject, stretchFactor and
Qt Quick Layout size-hint/rearrange calls. The original provider ZIP12773067393
was downloaded and independently matched SHA256
6a050bdd2f82fbf92f0b14436bd96424e6d2cb8fafa788b7fd20cd5b37067193.
Original full log/XML/stack/diagnostic are retained; no locals/memory/core dump.

The additional mitigation replaces only the remaining constant-three summary
Repeater with three stable slots and adds an identity/frame lifecycle regression.
The existing import/retained Columns and original crash/RTL/resize tests remain.
The hypothesis concerns layout-bearing delegate recreation during locale/summary
changes; the stack alone does not identify the exact affected QML object or prove
a common root with older crashes. Passing observations cannot prove all causes
removed, power-loss durability, hardware flicker repair or institutional readiness.

## Local corrective main-smoke / audit observations

The main smoke after the summary change failed with SIGSEGV2.65s; original
console/XML retained. Local GDB replay was unavailable (ptrace denied), not a
passing diagnostic. A single synthetic native-stack replay did not reproduce
the SIGSEGV; it instead failed closed at audit export with in-memory sequence139
versus signed disk prefix135. Independent wrapper and public signature/lifecycle
verification succeeded for that prefix. The snapshot-lag mechanism remains open;
no common cause with Qt or generated outputs is asserted. No success relabeling.

Both original private fixture snapshots and exact ELF were retained in a separate
private archive, SHA256
c3ad42631e2b66fdef36d41c9f015e58e9af5ef90b6551c4cd9db7d597465ae4
(3,203,220 bytes,72 original entries). No raw keys/databases/journals are public.
The existing CI helper now includes explicit synthetic main smoke: it replays
on a private fixture copy and distinct frame path, preserves original failure
status and excludes private resources from public artifact globs.
