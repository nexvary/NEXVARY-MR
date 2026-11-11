# Approved MR artwork and icons

The user approved two distinct visual roles:

- `mr-home-background.png`: home-only branded Nexvary search illustration. SHA-256: `f4217b39b398530c3a1f9663ed02fbf1cac28eef35463dd110accc1efa50cae9`.
- `mr-network-wave.png`: abstract cyan/silver network-wave background for working screens. SHA-256: `a671eabe165b97c344d0a0c90738422f9818a44a4e298258fa669547f0447758`.

Both files retain the exact supplied pixels. No masked analyst or spider image is
referenced by the application. The previous masked background is removed from
this branch; history remains in git. OsintBackdrop places the network wave with
a separate top-edge fade and dimming overlay; the original asset is unchanged.

The common MR palette remains deep black/petrol/cyan/silver. The current icon
source, license and presentation are documented below.


## Current navigation icon set

The earlier boxed metallic variants are retired from application resources.
Current QML uses assets/icons-professional: freshly fetched, pinned Microsoft
Fluent vectors with no backplates. Selected decorative colors are finished in
silver/indigo/crimson; semantic status colors are preserved. Original fetched
bytes and MIT license are in assets/icon-sources/fluent. The fetch script verifies
source hashes before producing the variants. The primary navigation uses the
same live app.sections model as the sidebar: 12 Government and 13 Repair entries.
