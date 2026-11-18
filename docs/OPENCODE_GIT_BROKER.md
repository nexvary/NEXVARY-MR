# Restricted OpenCode Git execution

The root-owned local Unix broker dispatches structured argv into a short-lived systemd service running as ubuntu with NoNewPrivileges, ProtectSystem=strict, a private temporary directory, hidden home and /proc, and resource/time limits.

## Scope
Only the exact NEXVARY-MR repository and feat/intelligence-ux-polish-20261117 branch are accepted. Supported commands are status, diff, branches, log, add PATH..., commit MESSAGE, fetch and push. No arbitrary Git options, force-push, merges, resets, main pushes or shell programs are accepted. Native source editing remains separate.

The isolated Git process sees a root-owned replacement .git/config, empty global/system configuration, hooks disabled, signing disabled, no external diff/text conversion, a fixed GitHub remote, fixed SSH executable and strict existing host-key verification. No repository-defined SSH command, credential helper, hook, filter or proxy is trusted.

Only fetch/push have networking and a read-only bind of the existing deploy key. The key is not copied, its permissions are unchanged, and ordinary test sandboxes cannot see it or AWS credentials. The AWS environment is never inherited. The key-bearing process executes Git/SSH only, not repository scripts or tests.

Authorized add/commit paths must be regular non-symlink files inside the project, with sensitive paths, .git, .opencode, .github and credential-like filenames rejected. Existing staged paths are checked again at commit. Only the current approved branch can be pushed, using a fixed refspec. New development branches require an explicit broker policy update.

## Limits
This broker is an execution boundary for its commands, not a claim that all OpenCode native tools are operating-system sandboxed. Existing native-tool secret path denials, service isolation and API authentication remain necessary. The single-thread broker serializes Git commands. The existing job runner lock prevents duplicate background workers, but is not a universal lock against arbitrary concurrent Web requests.

Sources under scripts/opencode are review copies. Installed executables are root-owned; editing these copies does not alter the live broker. Backups are retained on the server. Do not commit local credentials or local .opencode/opencode.json.
Runtime regression: OpenCode executed the isolated billing tests and restricted Git publishing workflow.
