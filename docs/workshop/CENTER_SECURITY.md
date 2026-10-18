# Workshop profiles, customer status and security checkpoint

Starting head: `b84342b8c5aaadcfa0ef94bcca6b92ac140a9810`. Native C++/Qt remains the sole desktop operation engine. PR #3 stays a development draft.

## Implemented

- SQLite v3 migration preserves v1/v2 orders, diagnoses, inventory and existing customer/device IDs. A returning phone can have multiple orders without creating another owner or device. A returning customer can register another phone. Profiles expose owner/contact, serial/model, repair history, symptoms, status and historical order costs. Editing a customer's current contact does not create a new identity. Existing duplicate profiles are not automatically merged.
- Workshop tabs: repair orders, customer/phone profiles and customer tracking. Data stays in the existing workshop database and its validated backup/restore flow.
- Opt-in customer portal implemented with Qt Network. Bind only to a selected local/private IPv4 address; default is loopback. Explicit network and sharing consent; no automatic firewall change. Links use 256 random bits, are stored only as SHA256 digests, expire within 1–90 days and can be rotated/revoked. Restoring a backup invalidates all restored tracking links to avoid resurrecting a revoked bearer token.
- Actual GET status pages, with Arabic/English status labels and refresh, show model, current status, status history and a deliberately entered customer-visible message. They omit customer name/contact, serial, fault, cost, technician and internal notes. Responses use no-store, no-referrer, noindex and restrictive CSP; no request URL/token logging. Unknown/revoked/expired links return the same unavailable response. Bounded connections, header size and timeout; no write or admin endpoint.
- ClamAV adapter: selected separately installed native `clamscan`, explicitly selected signature folder, one authorized local file/APK copy (128 MiB maximum). Creates a private verified working copy; records source, executable and database file SHA256, version, time, scope, output and return code; rechecks inputs after scan. Fixed arguments only, no shell, delete, move, quarantine or upload. Background worker with cancellation, process/output/time/archive bounds, encrypted/limit alerts, bytecode disabled and engine-side 7-day CVD-age gate. Custom-only signatures are clearly marked as limited coverage. No-signature-match is never a device health verdict. Report export requires consent and a new filename.
- Diagnostic tree engine binds a yes/no path to a particular order's exact brand/model/symptom, stores technician observations/path through existing diagnoses, and supports an operator-reviewed model-guide library. Import validates bounded text/nodes, exact identity, source/license metadata, references, reachability and absence of cycles. No fuzzy model substitution. Without an imported exact match, only clearly labeled general triage is offered; no invented board voltages.
- OEM catalog links official Samsung, Huawei, OPPO, vivo, LG and ZTE sources; retains read-only Windows PnP driver inspection. These are official landing pages, not bundled or silently installed driver packages. Availability depends on model, connection mode, region and Windows version.

## Test gates

`mr_center_tests` verifies repeat-owner/device histories, invalid IDs/rollback, persistence, v2 migration, public-data omission/HTML escaping, random-token rotation/revocation/expiry/restore, and actual localhost HTTP requests reflecting changed statuses. `mr_security_tests` tests the process adapter's alert/no-match/error/tamper/cancellation/time/output behavior and asynchronous bridge consent/report boundaries. `MR_CLAMSCAN_PATH` enables a real separately installed engine integration with a synthetic custom signature and benign control. It does not use real malware or pretend that custom fixtures validate official malware coverage. Linux CI requires `/usr/bin/clamscan`; Windows CI additionally downloads the separately packaged official ClamAV 1.5.4 Windows x64 ZIP, verifies pinned SHA256 `0d9e0228b2674137ea1a2853566c98a0278ad52ab2582c3d6dbd75373848c395` against the reviewed GitHub release asset metadata, and runs the same real-engine synthetic signature/control. These test files are not bundled with the product. Actual success must be read from the tested head's run.

`mr_catalog_tests` checks exact model matching, cyclic/malformed trees, license/source requirements, persistence and generic fallback validity. QML covers new profile/tracking/security/tree tabs and saving findings, alongside existing maintenance/navigation/RTL/theme tests. Packaged smoke makes an actual localhost status request and captures every new tab in AR/EN and dark/light, in addition to existing forensic/recovery/board smoke. Qt6Network.dll is a required runtime dependency.

Read the tested head's CI results and artifacts in PR #3; source compilation alone is not release acceptance.

## Official dependency/source review — 2026-10-18

ClamAV reviewed upstream head: `72cd48c9faed4fa4afc22bc4ed0b9b19f8d3f8f7`, [COPYING.txt](https://github.com/Cisco-Talos/clamav/blob/72cd48c9faed4fa4afc22bc4ed0b9b19f8d3f8f7/COPYING.txt), GPL version 2. The adapter invokes a separately installed process, does not link libclamav, copy upstream code, redistribute binaries or bundle signature databases. Bundling or commercial redistribution needs its own dependency/license review.

- [ClamAV scanning and scope](https://docs.clamav.net/manual/Usage/Scanning.html)
- [ClamAV signatures, custom rules and FreshClam](https://docs.clamav.net/manual/Signatures.html)
- [clamscan options](https://github.com/Cisco-Talos/clamav/blob/72cd48c9faed4fa4afc22bc4ed0b9b19f8d3f8f7/docs/man/clamscan.1.in)
- [Android OEM-driver guidance](https://developer.android.com/studio/run/oem-usb)
- [Samsung](https://developer.samsung.com/android-usb-driver), [Huawei HiSuite](https://consumer.huawei.com/en/support/hisuite/), [OPPO](https://support.oppo.com/en/), [vivo](https://www.vivo.com/en/support), [LG](https://www.lg.com/us/support?popup=software), [ZTE](https://support.ztedevices.com/en-gl/).

## Unfinished acceptance requirements

1. Public remote tracking is **not deployed on an operator server**. A separate single-workshop HTTPS service, credential enrollment, authenticated snapshot sync, bounded requests, backups and Linux systemd installer are now implemented in [server/tracking-portal](../../server/tracking-portal/README_AR.md). The retained HTTP portal remains trusted-LAN only and must not be port-forwarded. Public DNS/ACME and operator-server acceptance remain pending. No external customer data has been published. QR receipts are not generated; actual private links are provided.
2. Device-wide Android scanning/acquisition, installed-app inventory/collection, runtime behavior and inaccessible/encrypted private data are not covered by a file scanner. No FRP, lock or encryption bypass. FreshClam update orchestration and real Windows official-database/device acceptance remain pending.
3. A complete verified model-specific hardware fault library is **not supplied**. Licensed exact-model service guides, revision compatibility and qualified physical measurements are required. Imported operator guides are not automatically OEM-certified.
4. Driver binaries, installation/elevation and model/VID/PID verification are not integrated. No universal Qualcomm/MediaTek/Unisoc installer is claimed. Physical device/USB and clean Windows 10/11 tests remain necessary.
5. Phone ownership transfer/duplicate-profile reconciliation, estimate approvals/deposits, tax/accounting, role-based staff access and encrypted multi-user storage are future work. Existing local workshop database is not a multi-tenant server.

Next engineering increment after latest-head Windows/server gates: accept the HTTPS portal on the operator's actual DNS/server; add lawful Android app inventory/base+split APK collection through the existing ToolBridge; then verify fresh official Windows signatures. Never replace those steps with a "whole phone clean" label.

### Follow-up acceptance gate

The real Windows test runtime is pinned to the official [ClamAV 1.5.4 release](https://github.com/Cisco-Talos/clamav/releases/tag/clamav-1.5.4) (published 2026-08-07), [source](https://github.com/Cisco-Talos/clamav/tree/clamav-1.5.4). Release ZIP is used only in CI with synthetic data, not redistributed in the installer. New workspace panels respect their implicit wrapped content height; QML checks bottom actions at 1180×760 in AR/EN and both themes.

### Windows feedback on first workshop increment

Run [39132569833](https://github.com/nexvary/NEXVARY-MR/actions/runs/39132569833) at `8eadc37` is **failed**: native compilation, portable and installer checks passed, but the center test suite failed. The token-at-rest test retained its own read-only QFile handle while exercising backup restore; Windows file replacement disallows that open inspection handle. The follow-up closes this test-only inspection handle before restore, preserves the restore assertions, and makes QtTest write explicit text logs for every suite on all platforms. The next run must confirm the diagnosis; passing packaging does not make the failed test run green.

### Server installation increment — 2026-10-18

Accepted starting head `c9dd78275ba1232b26c42d5b67436ebdc17356a6` passed all four jobs in [39133227949](https://github.com/nexvary/NEXVARY-MR/actions/runs/39133227949), including 14 Qt/CTest suites on Windows/Linux, 20 QML checks, real separately installed ClamAV fixtures, portable and install/uninstall smoke. Those artifacts predate the server increment.

The next source increment adds `CloudTracking`: explicit public-data upload consent, a Windows current-user DPAPI-protected credential file, HTTPS-only transport, no redirect/TLS bypass, publisher identity and revision checks, automatic 30-second/debounced sync, and copy only after server acknowledgment. Server state expires after six hours without sync; revocation while offline is explicitly pending. No customer/contact/serial/fault/cost/technician/internal notes are included. The service stores only SHA256 token digests. Backup restoration empties published links until fresh desktop sync, avoiding revival of revoked tokens.

Local validation: six Python unit tests; real Gunicorn/Caddy trusted TLS publication/customer page/revocation/log redaction; native Qt client through the same real HTTPS service. CI adds a disposable Ubuntu 24.04 systemd VM install/upgrade/backup/restore/key-rotation test of the actual single-file bundle. Read the new head's job conclusions before accepting this increment. Debian, Ubuntu 22.04, ARM64, public ACME and the operator's actual server are target environments, not already-tested deployments.

At `9bfe75ba7737b2bca7bacfcce7c64ad1bacd5329`, [run 39139227874](https://github.com/nexvary/NEXVARY-MR/actions/runs/39139227874) passed the actual Ubuntu 24.04 systemd bundle install, trusted local TLS, backup, upgrade with retained credentials, restore revocation, key rotation and journal privacy. The tested server artifact is [12140504886](https://github.com/nexvary/NEXVARY-MR/actions/runs/39139227874/artifacts/12140504886), installer SHA256 `897ce84483e358159df12f92a2bd0c0ed126e401855a8b56f1080f3fbd39e8f8`. Windows compilation failed because Qt 6.8 deprecates `setTimeSpec`; the follow-up parses the stored SQLite UTC time with an explicit ISO UTC suffix, retaining Qt 6.4 compatibility and warnings-as-errors. The full run is not accepted as green; the follow-up must pass Windows.

Windows packaging explicitly requires Qt's native Schannel TLS backend. Isolated portable and installed-runtime smoke also checks `QSslSocket::supportsSsl()` with the SDK removed from PATH, so a package cannot pass merely because HTTP worked in the developer environment. Native Qt -> real Gunicorn/Caddy TLS was retested locally with `TZ=Pacific/Auckland` after the UTC fix; certificate validation, publication and revocation passed without a TLS bypass.
