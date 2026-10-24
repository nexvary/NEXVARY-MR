# NPSE repository integration audit — 2026-10-24

Source and open branches/PRs were inspected with the NEXVARY GitHub connection
before writes. Private USB/NetScan access worked; no 404/publicity assumption was
made. No existing feature branch or main was merged/reset remotely.

| Repository/ref snapshot | Actual platform/functions inspected | Privilege relevance and decision |
|---|---|---|
| MR `feature/usb-doctor-intelligence` 9a4392a37b85186032ded8cdbe254b2064d8ed9b; main 7446b17d0d83d4e7d9da3c8d4bbdd7dae6d4de17 | Desktop repair/USB/ADB plus existing `recovery-agent` Android app, fixed Shizuku metadata service, consented local export | Host NPSE in existing Android build; reuse policy/runner and original report schema. Desktop cannot call Android Binder directly. No second companion or desktop operation engine. |
| USB `main` 079b49dd2e5717a07509472e6275114a26e22c11 | Python/Windows AT/PCSC/USIM tooling, bounded modem operations, secure bridge, local diagnostics | Shizuku cannot supply PCSC drivers, UICC credentials or arbitrary peripherals. Standard Android USB counts are available in NPSE; defer a new USB Android consumer until an actual host/OTG test proves benefit. GPL-3.0 USB code is not copied into reusable MIT modules. |
| NetScan `dev/foundation` 08f95946540b02fe731f50db3c97027161b612e2; main 32c9d58a9dcbd5043550c6e7bc5e9f1eeb80a7aa | Windows WPF/.NET network scanner, ARP/adapter classification, finding import/history/tests | No Android application here to link an AAR. No speculative privileged network scanning. Standard Android diagnostics can later be imported as a separate explicit schema; no unsupported desktop Shizuku adapter added. |
| Recorder/Guardian `feature/guardian-studio` bde1110629f0295b026848b8cd2651c69fa0f5e8; main d51e6339b7e30af6eec954a44dc6745cc5ea47e1 | Android Camera2/foreground recording, media/library/player/setup, optional call-companion readiness; existing Shizuku API 13.1.5 in CallCapturePreflight | Existing preflight must not imply call-audio access. First reusable NPSE consumer is MR. Guardian adoption is pending reviewed publication; camera/mic/MediaProjection consent and indicators remain mandatory. No recording-capture rights are inferred from Shizuku authorization. |

Open MR PR chain #1–13, Guardian #7/#10/#11/#12, and branch heads were reviewed.
NPSE PR #14 targets the existing MR USB/repair branch, preserving the other
workstreams and the paused forensic feature expansion. Existing Kotlin policy
adapter and helper were modified instead of creating a duplicate legacy engine.

## Further candidates, with evidence limits

The account repository inventory and `dev/foundation` trees were inspected for
WiFi-Call, Veil-Launcher and Traffic. WiFi-Call has an Android app/core-model and
telephony service; normal SIP/IMS/USIM boundaries are not removed by Shizuku.
Veil-Launcher has an Android app; scoped package/settings functionality may be
useful, but no adoption is claimed from a tree inspection. Traffic already has
an Android VpnService and consented capture test; NPSE must not replace VPN consent
or claim arbitrary HTTPS decryption. These are candidates, not completed audits
or implemented integrations. HifzAI needs no elevated service for its core goals.

## Reuse risk controls

No cross-product source copy of desktop engines; no dependency on an unreviewed
moving branch; no new repository; no extra server. NPSE Maven local publication
preserves its core/API dependencies. Publishing a stable shared artifact and
Guardian consumption are separate tracked gates, not implied by an AAR build.

## Completion decision

Guardian now consumes the reviewed NPSE binary as a pinned exclusive Maven
dependency (PR #13, source 7f32420), with checksum/license checks and successful
host build/lint/unit/API33–35 UI suites. This supersedes the earlier adoption
pending state. USB/NetScan remain Windows-only; no Android adapter is justified
in this alpha. Those study decisions are complete, not implemented integrations.
