# Recovered process-ownership work — 2026-11-02

The native probe was compiled and run against unchanged published Government
cc23ccbcf9d32c55ac2ffade7cde50e4b2447ea1 before TskImage edits. Its recorded
stdout was: cancelled=1 error=Operation cancelled. detachedSurvived=1.
baseline_probe.cpp is recovered from its recorded creation command. Link it
against that exact source's cpp/src/TskImage.cpp and Qt6Core; running against the
new owned runtime is a different experiment.

local-observations.json records tool-observed predecessor results, including the
first 3-pass/8-fail component attempt, subsequent 11-pass component attempt and
9/9 product groups. It is a recovered observation summary, not the inaccessible
raw filesystem logs. Initial package-install/configuration/dependency attempts
and an interleaved-source build failed; later private-prefix setup/serialized
build and the recorded 9-group observation passed. A later build request named
a nonexistent interop target; its corrected build succeeded. No check was lowered.

The environment became unavailable during final CTest/Python/ASan collection.
Those final results and raw files/binaries were not recovered and are not claimed.
The published candidate was reconstructed from the exact original source plus
recorded edits using GitHub. No byte-equivalence to an unavailable local binary
is asserted. New exact-source Push/PR CI must test it. Do not relabel preceding
local results or the Windows packages from 5e0ac5b/b3113a4 as this candidate.

No real-case input, credentials, key or private archive is included. See
docs/OWNED_PROCESS_LIFECYCLE.md for the explicit Linux hostile-supervisor-kill
escape negative, unsigned/non-durable receipt boundary and external gates.
