# Phone-number OSINT integration — evaluation candidate

Status: implementation candidate; **not a verified Windows evaluation release** until the exact commit's native suites, full Qt workflow, engine security review and packaging gates have passed.

## Architecture and authority

`mr::osint::Provider` is independent of QML and case persistence. PhoneInfoga runs in an `OwnedProcess` on a worker thread, with a fresh private directory, minimal environment, explicit binary SHA-256, a private copy of the verified executable, and no shell. Each request starts its own engine and always stops it. No service is left running between searches. Startup authentication has a five-second bound; each scanner has a ten-second bound; cancellation polls at 25 ms; cleanup uses the existing bounded ownership supervisor/Windows Job Object.

The application accepts a single explicit international number (E.164 syntax, not proof of allocation). Arabic and Persian digits, spaces, parentheses and hyphens normalize; national numbers without country codes, extensions, lists and control characters are rejected. It never guesses the country. Upstream numbering-plan validation remains separate.

Investigator authentication, AnalyzeEvidence/AddEvidence roles, an accessible active case, a registered case device and consent are mandatory in native code. A separately reviewed `external-analysis` exception must reference the actual case UUID and include the exact normalized number in its reason. One OSINT operation consumes that approval; no batch, file import, unattended retry or automatic rescan exists. A new independent review is needed for the next search. Session, approval, case identity/revision and case grants are checked at dispatch and before persistence; a 50 ms monitor cancels stale workers.

Reports retain UTC retrieval timestamps, scanner names, raw HTTP response bytes (base64), their SHA-256, HTTP/transport errors, upstream provenance, executable digest, operation link and limitations. Each response is capped at 256 KiB; five explicit scanners maximum. External keys are intentionally not inherited or configured in this first adapter. Missing-key scanners fail explicitly. No commercial OSINT Industries endpoint or test data is implemented; its adapter returns `provider-disabled-license-required`.

`local` means offline numbering-plan metadata; `googlesearch` produces suggested links, not observed search hits; other scanner content is unverified public-source content. Geography is not device location; carrier metadata is not a current subscriber record. No identity, tracking or telecom-record claim is made.

## Case evidence and audit

The report is registered as a protected JSON evidence payload using existing DPAPI storage, bounded SQLite transaction, case revision check, custody entry and case audit chain. Source responses remain inside that protected evidence, not ordinary temporary files. Existing evidence export/working-copy controls apply. The policy receipt records request/report hashes and original approval. Case persistence and policy receipt are separate commits: incomplete receipts withhold the displayed result and explicitly require review; this adapter does not claim atomic cross-store recovery for OSINT. A crash can leave an interrupted policy operation or a retained report needing reconciliation. Existing partition-analysis transaction recovery is unchanged.

## Why stock PhoneInfoga is refused

Reviewed upstream: `55807b05b753dab7bc6fe67403ca385668dda179`.
Upstream README explicitly says stable but unmaintained. `cmd/serve.go` binds `:port` without a localhost flag. `web/server.go` has no session authentication. The NEXVARY GPL patch forces `127.0.0.1`, requires a random per-launch token, rejects browser Origin headers, removes request logging, caps inbound request bodies and allows only scanner-list/single-scanner routes. The client proves engine possession of the token through an HMAC-SHA256 challenge **before** sending a number or bearer token. A port reservation race therefore cannot disclose the target to an unauthenticated competing listener. Stock binaries fail that handshake. External scanners may still contact their services: loopback binding and process ownership are not OS or outbound-network isolation.

## License and security packaging gate

PhoneInfoga is GPL-3.0. Its full license and the NEXVARY modification patch are under `third_party/phoneinfoga`. The adapter does not link Go modules or copy PhoneInfoga algorithms into C++. A process/REST boundary is an engineering separation, not an automatic exemption from GPL obligations; the GNU FAQ says both communication mechanism and semantics matter. When distributing the modified engine, retain upstream copyright/license notices, identify modifications and provide the complete corresponding source, dependency manifests, patch and build instructions with the engine. Do not apply restrictions that remove recipients' GPL rights. NEXVARY repository-wide licensing is not established by this integration.

Primary references:
- https://github.com/sundowndev/phoneinfoga/blob/55807b05b753dab7bc6fe67403ca385668dda179/README.md
- https://github.com/sundowndev/phoneinfoga/blob/55807b05b753dab7bc6fe67403ca385668dda179/web/docs/swagger.yaml
- https://www.gnu.org/licenses/gpl-faq.en.html#MereAggregation
- https://www.gnu.org/licenses/gpl-3.0.html#section6

Do not bundle an upstream release fetched as an opaque executable. Build from the pinned source plus patch, retain a source archive/module list/build receipt, run Go dependency vulnerability analysis and native lifecycle tests, then review any reachable vulnerabilities before distribution. Old Go module versions are not a security clearance. The engine module overlay pins updated dependencies and Go 1.26.10. `scripts/prepare_phoneinfoga.py` fails closed on scanner/session test failures, module verification failures and govulncheck findings; it retains a complete vendored corresponding-source archive, GPL license, build logs and binary/source digests. API-only packaging excludes the browser bundle. A clean vulnerability result is not a guarantee of security.

## Verification

`cmake -S . -B build-osint -DMR_BUILD_OSINT_TESTS=ON -DMR_WARNINGS_AS_ERRORS=ON`

`cmake --build build-osint --config Release --parallel 2`

`ctest --test-dir build-osint -C Release --output-on-failure`

The focused provider suite uses a **clearly synthetic native localhost fixture**, never fabricated investigative findings. The existing full Qt workflow must also pass on Windows/Linux in production/laboratory profiles. Real upstream-engine integration, bilingual rendering, authority revocation and protected case retention have dedicated tests; their exact-commit CI results and Windows packaging still control promotion from draft.
