# Synthetic official-tool reference images

These are original test metadata, **not OEM firmware, device dumps, commercial signing material or an approved device profile**. No production/private signing key is committed.

Generated locally with the separately obtained AOSP `avbtool.py` from `android-15.0.0_r1` (tool version 1.3.0). `expected.json` records its HTTPS source URL and locally computed source SHA-256. A source digest identifies downloaded bytes; it is not a separate upstream release signature. The upstream Python implementation is not vendored or executed by CTest; its header has an MIT-style license.

- `unsigned-property.img`: unsigned metadata with a harmless synthetic property, hashtree-disabled flag, and a rollback index beyond JSON's exact numeric range.
- `signed-property.img`: metadata signed only with a newly generated ephemeral synthetic RSA2048 key. The original key was deleted by the generation process. This is not an OEM trust anchor or application release signing certificate.
- `corrupt-signature.img`: one signature byte changed from the signed fixture. Official `verify_image` rejected it; see `verification.txt`.

Official `info_image` output is retained. NEXVARY's envelope fields match declared values for all three images, while its four trust/compatibility booleans stay false for **both valid and corrupt signatures**. This intentionally verifies the boundary: our parser does not authenticate signatures and must not imply that it does.

The generator is `research/advanced-mobile/generate_avb_reference.py`. It requires a separately supplied matching source file, Python and OpenSSL. It performs no downloads or phone operations. Re-running it creates a new random fixture key and therefore new signed-image hashes; review regenerated fixtures together with their expected manifest. Do not regenerate merely to force a failing parser test to pass.
