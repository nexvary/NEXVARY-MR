# NEXVARY Advanced Mobile Research Lab

Research is converted into bounded native capabilities with explicit evidence. This directory is a first provenance registry, not a completed device-support database or a claim of parity with commercial platforms.

The first implemented chain is **AOSP AVB format → raw vbmeta metadata → firmware.avb.metadata.v1 → C++ parser/async FirmwareBridge → synthetic Qt, mutation and UI tests**. No chipset, physical device or firmware build is approved by this record. The output's capability identifier links it to `knowledge.json`.

## Implemented AVB inspection

- Local raw `AVB0` images, 256 bytes–16 MiB; required header major 1. Minor version is reported, not treated as a supported feature set.
- Big-endian block/region bounds, alignment, non-overlap, reserved header and descriptor envelope checks. At most 4096 descriptors. Descriptor bodies and algorithms are not validated.
- Exact decimal strings preserve 64-bit rollback indexes and descriptor tags in JSON. SHA-256 covers the inspected bytes, including trailing data.
- All metadata is untrusted. Signature/OEM key, partition digests, stored device rollback indexes and device compatibility are **not verified**. A `metadata-only` result means readable envelopes, never an AVB verifier result.
- The asynchronous native API supports cancellation and rejects missing/oversized sources. Original files are opened read-only. Atomic report export refuses the source's canonical path. Reports omit the selected local path; declared release text and hashes can still be identifying. Same-size concurrent source changes cannot be ruled out: use a stable forensic working copy.
- ZIP, sparse images, AVB footer discovery, full partition verification, trust anchors and flashing are unsupported here. A syntactically readable descriptor can have an invalid body.

The parser is original NEXVARY C++, developed against the public format definitions. No `libavb` implementation or proprietary code is copied or linked. Upstream format headers carry an MIT-style license; this does not license firmware acquired from a vendor or user.

## Reproduction

Normal production/Laboratory CTest includes `mr_avb_tests`, `mr_avb_mutation_tests`, the FirmwareBridge regression, UI navigation and deployed GUI smoke. The mutation executable runs 30,000 deterministic hostile-input cases; it is bounded smoke coverage, not exhaustive fuzzing.

For isolated parser instrumentation:

```sh
cmake -S . -B build-avb-sanitized -G Ninja -DMR_BUILD_GUI=ON -DBUILD_TESTING=ON -DMR_SANITIZERS=ON -DMR_WARNINGS_AS_ERRORS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-avb-sanitized --target mr_avb_mutation_tests --parallel 2
ctest --test-dir build-avb-sanitized -R '^mr_avb_mutation_tests$' --output-on-failure
```

`knowledge.json` records traceability, not permission to run a phone operation. The compiled ServiceOperationPolicy remains authoritative and unchanged by this research increment. Unit checks validate graph references and prohibit model approval without scoped physical evidence. No external source is fetched by the application.

## Remaining research areas

AVB signature/trust validation, dm-verity, TEE/TrustZone, KeyMint/StrongBox, FBE/metadata encryption, published Qualcomm EDL and MediaTek DA protocols, Knox, Secure Enclave/Data Protection, USB/ADB/Fastboot/Recovery, chip-off/ISP/JTAG, damaged-device recovery and attestation need separate scoped source reviews and acceptance tests. They are **not implemented or endorsed by this AVB parser**. Raw memory access does not establish access to plaintext user data.

Official AOSP avbtool 1.3.0 reference fixtures are now generated and compared in native tests; see `samples/avb-reference/README.md`. Both a valid synthetic signature and its corrupted counterpart remain unverified by NEXVARY.

Next practical research: select stable OEM-provided vbmeta samples with redistribution rights and design a separate signature verifier with explicit OEM trust anchors. Do not authorize device writes from parser success.
