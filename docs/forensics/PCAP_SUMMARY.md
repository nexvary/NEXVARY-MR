# Native offline PCAP summary

Capability: `forensic.pcap.summary.v1`. This is an original bounded C++ parser integrated into RecoveryBridge, the existing encrypted case/evidence store, custody/audit chain and explicit plaintext export controls. No libpcap/Wireshark source or executable is bundled, launched or linked by this feature.

## Scope

- Classic PCAP 2.4, both byte orders, microsecond/nanosecond timestamps. Maximum 8 MiB, 200,000 records. PCAPNG and modified legacy variants are unsupported.
- Counts and captured/original lengths; timestamp extrema and backwards-order observations. Exact integers are represented as decimal JSON strings. Capture clocks and original lengths are untrusted declarations, not independently verified real-world times or traffic volumes.
- Ethernet II, at most two 802.1Q/802.1ad tags, basic IPv4/IPv6 header declarations. TCP/UDP/ICMP counters count protocol fields, including IP fragments, not verified transport packets or reassembled flows. IPv6 extension chains, checksums, jumbograms, applications and retransmissions are not interpreted. Other link encodings receive container statistics only.
- Strict policy rejects captured length exceeding snapshot/original length, invalid timestamp fractions, partial record headers/payloads, unsupported versions and bounds violations. The PCAP draft permits some legacy original-length anomalies; this stricter reader rejects them explicitly rather than guessing.
- No packet contents, MAC/IP addresses, ports, hostnames or credentials are emitted. No network interface is opened, no live capture occurs and no TLS decryption is attempted. Hashes and timing metadata can still identify a capture.

## Evidence handling

An active case, recorded operator and per-file authorization are required at the native entry. Regular bounded local files only; use a stable, owner-authorized forensic working copy. The selected source is read without writing and parsed from immutable in-memory bytes. A second full read must match the parsed bytes' SHA-256 before the summary is committed. A bounded memory copy is not an independently anchored disk image; identical observed reads do not rule out every concurrent-writer scenario.

Only the JSON summary is stored as `native-analysis` evidence. The original capture is not embedded. Internal case provenance records its selected source path and hash; whole-case exports can contain those details and require plaintext confirmation. The standalone summary omits the selected source path. Existing output checks reject overwriting sources/existing files. Rejection/cancellation commits no partial summary; existing failure-audit behavior remains in effect.

Production cases on Windows retain DPAPI requirements. Linux fixture tests use an explicitly synthetic case profile; they do not prove native Windows encryption or real-device acquisition.

## Primary references

- [IETF PCAP draft revision 09](https://www.ietf.org/archive/id/draft-ietf-opsawg-pcap-09.html), September 2026: work in progress, not a final RFC. Reviewed header/record format facts; no source code extracted.
- [RFC 791](https://www.rfc-editor.org/rfc/rfc791) and [RFC 8200](https://www.rfc-editor.org/rfc/rfc8200): basic IP header layout. This feature does not implement full protocol validation.

## Tests

`mr_pcap_tests`: four endian/resolution variants, hostile length/timestamp data, truncation, format rejection, privacy assertions, VLAN/IP scope, clocks, cancellation, record/input limits and native case authorization/persistence/export/source preservation.

`mr_pcap_mutation_tests`: 30,000 seeded bounded hostile-input cases; failures must return no partial report and successes must preserve false payload/address/decryption flags. This is mutation smoke coverage, not exhaustive fuzzing.

QML tests exercise consent/operator gates and the action at minimum size, Arabic/English and light/dark modes, evidence filtering and actual runtime screenshots. Physical Android, native Windows and third-party capture-corpus comparisons remain separate acceptance work.
