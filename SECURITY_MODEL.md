# NPSE security model

## Authority

Standard Android first; optional consented Shizuku shell; authorized desktop ADB
remains a separate existing MR subsystem. NPSE refuses UID 0 and any backend UID
other than 2000. A live manager or permission alone is never proof a probe works.
Owner consent is a product interaction; it is not ownership verification or a
legal authorization system.

The UserService obtains the owning app UID from its trusted package Context,
not an IPC argument. Collection and cancellation require exact Binder caller UID
match (app UID >= 10000). Lifecycle destroy also accepts shell UID 2000 because
Shizuku's server invokes it. Both original and NPSE helpers validate callers.
These are source/JVM checks; live Binder adversarial tests remain pending.

## Execution

Only enum-selected executable/argument lists are accepted. No `sh -c`, user
commands, arbitrary paths, variable package arguments or public terminal exists.
Executable paths are absolute. Per-process output is <= 4096 bytes; processes
have a 1500 ms deadline. Cancellation destroys the active child, including races
with startup; finally closes streams and destroys children. A service request
has a 12-second UI deadline and one collection at a time. Failures never retain
raw stderr or exception text. Root and system security changes are unsupported.

## Lifecycle and data

The optional control is off on each new screen/session. Permission and UID are
checked before binding and before result delivery, plus foreground polling and
Binder received/death listeners. Service loss/revocation clears privileged
observations; an epoch drops stale worker results. onStop cancels work, unbinds
the non-daemon helper and clears shell observations. No boot receiver/persistence
claim is introduced. Standard mode remains available without Shizuku.

Reports contain bounded model/SDK/patch and aggregate memory/battery/USB counts.
They exclude serials, fingerprints, MAC/IP, account identities, raw app inventory,
private app data and unrestricted logs. Visible launcher packages are shown only
in a local settings chooser, capped at 200. Mutations are delegated to Android
settings and performed by the user; NPSE cannot silently grant, clear or disable.

The control-center audit is an in-memory bounded session list of fixed event
codes and UTC timestamps, never shell output/package identifiers. It is not a
persistent/tamper-evident forensic audit. No sensitive write operations are
implemented. Persisted audited mutations need a separate future security design.

No INTERNET permission, network listener or cloud telemetry is added. Local JSON
export needs a consent dialog and explicit supported local document provider.
Shell results are intentionally omitted after the picker moves the app out of
the foreground. The original MR recovery report has its own consent and schema.
No capture indicator, MediaProjection permission, device lock or USB consent is
bypassed. Installing a library does not grant any elevated permission.
