# Mobile Security Architecture Atlas — Module A

Purpose: provide a structured, sourced map of Android and iOS security architectures, hardware trust boundaries, boot chains, encryption states, interfaces, and diagnostic implications.

## Initial topics
- Android Verified Boot / AVB and dm-verity
- Android File-Based Encryption and Direct Boot
- KeyMint, Keystore, TEE / StrongBox
- Qualcomm SoC, boot chain and USB interfaces (architecture only)
- MediaTek boot chain and USB interfaces (architecture only)
- Samsung Knox and hardware-backed security
- Apple Secure Enclave and Data Protection
- USB enumeration, ADB authorization, Fastboot capability discovery
- Forensic integrity, evidence preservation and parser isolation

## Atlas record fields
`id, platform, subsystem, trust_boundary, supported_versions, description, implications, source_ids, evidence_status, last_reviewed, limitations`.

Do not conflate FRP, UI screen lock, bootloader lock, or encrypted user data. None of these records implies device access. Every entry must identify which claims are published and which have been validated in NEXVARY's laboratory.

## First integration milestone
A read-only Atlas search and detail panel under Device Intelligence, showing source provenance, applicability constraints and evidence confidence. No unsupported "unlock" actions.
