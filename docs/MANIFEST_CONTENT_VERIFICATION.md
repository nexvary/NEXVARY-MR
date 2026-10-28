# Workbench manifest content verification

The existing `workbench-export-manifest` adapter retains its structural mode.
The Government Analysis page also offers content verification: import authorized
payload files through the existing case evidence workflow, select the manifest,
then bind its exact logical paths to evidence from that same case. Unbound entries
are reported missing. A valid report may contain mismatches or missing files;
`payloadVerified` is true only when every declared entry matches SHA-256.
A successful parser/retention operation is not equivalent to a matching package.

The adapter never opens a manifest path. It receives immutable, hash-verified
bytes from `LawCaseStore`, not a package directory. Consequently manifest paths
cannot follow symlinks or escape into the filesystem. Direct live-folder scanning
and acquisition are outside this increment. Existing evidence-import protections
remain authoritative; this feature does not expand acquisition permissions.
Only portable printable ASCII relative names are supported, with bounded length,
no empty/dot/dotdot components, backslashes, drive/stream syntax, Windows reserved
names, trailing dots/spaces or case-folded duplicate paths. Unsupported names are
rejected rather than silently normalized. Bindings must exactly match an entry;
extra bindings and case-aliased binding duplicates are rejected.

Limits: manifest 8 MiB, at most 128 entries/bindings, aggregate payload bytes 8 MiB,
report 64 KiB, existing cooperative 5-second worker deadline. Source and each
payload are checked before dispatch; payloads are re-read and hash-checked before
retention alongside current case revision, actor, grant and audit checks. No hard
OS thread/memory sandbox is claimed. Empty protected evidence is unsupported by
the existing store; a missing binding is distinct from an invalid evidence ID.

Reports use manifest entry order and contain no wall-clock/random data. They
retain manifest hash, case/evidence ID, original repository and pinned commit,
verifier version/parameters, exact raw JSON entry reference (`/N` or the selected entries/files/evidence member), declared/actual SHA-256, payload
evidence ID, size and verification status. Original/working hashes refer to the
imported protected bytes, not an independently authenticated acquisition. The
existing signed prepared operation and case SQLite transaction retain the report;
there is no additional evidence database or imported signing/update authority.
Retained content reports are checked for internally consistent paths, statuses,
hashes, counts, parameters and bounds before findings projection. Structural
confidence remains explicit; content matching does not prove acquisition truth.

Upstream inspected: `8b35940b1c879e0ce5f5c13e0d6c89aeab023757`, workflow payload
v12.22 tar.xz SHA-256 `9eff37d785b6e44df2e3a611f9d44387b5a78dccd9bf240a4b352edb8b14bb3e`.
`nexvary_lab/pro_tools_v1221.py:export_validate` checks structure/duplicates/hash
syntax only. Content checking is an MR extension to that selected adapter under
the recorded owner permission; broader Workbench review remains open.
