# Structured diagnostics increment — 2026-10-19

## Live repository audit

Fetched origin before development. Heads: main `7446b17d0d83d4e7d9da3c8d4bbdd7dae6d4de17`, native foundation `0bfe9b113cfac6c9f50f0e278d6faa8ff6733ffc`, OEM catalog `ef302c9555dd4fc0b5189182be5bd109d932bbdf`, forensic foundation `0f979797f03cabf0dc0a504f23dc5c5f9c2cd116`, service engine `ac186f4ce569958401e1c4d63479da0553d26c1e`. This independent branch extends the service-engine head, preserving the neon controls and Arabic guidance fixes.

The GitHub API reconfirmed successful baseline native run [39219740626](https://github.com/nexvary/NEXVARY-MR/actions/runs/39219740626) and Python run [39219740650](https://github.com/nexvary/NEXVARY-MR/actions/runs/39219740650). They do not validate this change. README, native command adapters, process orchestration, service fixtures, Qt workflow, dependency notices and the existing competitive matrix were reviewed. Existing firmware preflight, discovery, consent, private package staging and recovery cases are retained, not rebuilt.

## Research refresh and implementation decision

The existing [comparative matrix](COMPETITIVE_ENGINEERING.md) covers functions, evidence, drivers, commercial dependencies, open implementation paths and priorities. Re-read primary [Chimera Device Info](https://chimeratool.com/en/features/device-info): Samsung Download information and fuller ADB information have distinct scope. Re-read [Octoplus features](https://octoplusbox.com/en/features/features/): advertised operations and separate product/module activations do not establish universal per-model support. The [UnlockTool .org site](https://www.unlocktool.org/) identifies its reseller status; direct retrieval of `unlocktool.net` failed again. No reseller model list is promoted into validated support. Prior video research remains descriptions/landing pages only; no complete video viewing is claimed.

This increment implements an independent structured observation layer over the already authorized ADB probe. It does not add a proprietary protocol or pretend to integrate commercial software. Existing adapter separation is retained. Driver correlation, general phone backup/restore, signed OEM loaders, and independently updatable executable modules remain unfinished.

## Implemented behavior

- Charge is calculated only from a valid `level` plus positive `scale`, with level within scale. Duplicate battery keys and integer overflow are rejected.
- Temperature uses the documented tenths-Celsius unit. Android status/health codes remain explicitly reported codes, not battery capacity/wear certification. Stopped battery updates are flagged as potentially simulated.
- `/data` total/used/available are reported in KiB from the existing `df -k /data` call. Missing, invalid or ambiguous rows stay unavailable; reserved blocks need not sum to total. Display converts KiB to GiB.
- Kernel block-device observations are bounded to 256 entries. They are not a validated flash partition layout, and absence does not prove storage health.
- Fastboot does not inherit ADB observations. USB driver remains explicitly uncorrelated.
- Arabic/English panel shows readings and capture time. JSON service exports include `profile.diagnostics` schema 1 as an additive field within report schema 2. Raw observations remain available in the profile. Existing profile invalidation/expiry semantics remain.
- No new device writes, shell interpolation, dependencies, permission changes, licenses or commercial services.

Format evidence: [AOSP BatteryService](https://android.googlesource.com/platform/frameworks/base/+/refs/heads/main/services/core/java/com/android/server/BatteryService.java), retrieved 2026-10-19 (blob `4cf17ae3984d8eab54fc165534fe175c965fede6`), documents scale, level and tenths-Celsius temperature. No upstream source was copied.

## Verification

Local `python -m unittest discover -s tests -v`: 4 passed. `git diff --check`: clean. Local CMake and Qt are unavailable; native build/tests are delegated to the repository's existing Windows/Linux GitHub Actions through this branch's PR. Added parser cases cover valid values, missing scale, duplicates, overflow, invalid/ambiguous disk rows, Fastboot separation, and an actual mocked child-process probe-to-JSON integration. Synthetic fixtures never count as phone acceptance. Hardware behavior: **Pending Hardware Validation**.

## Remaining order of work

1. Correlate a selected transport to Windows USB instance and signed driver, preserving ambiguity.
2. Scoped owner-authorized visible-file backup with hashes, bounded transfer and explicit restore review.
3. Exact-model firmware evidence and official-mode adapters only where independently documented/tested.
4. Signed module/update architecture and release gates; no claim of hot-loaded native modules today.
