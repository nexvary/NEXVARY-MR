# Uploaded Chimera installer — static inspection, 2026-10-18

Source: user-uploaded `ChimeraInstaller(2).exe`; not downloaded from a verified vendor URL in this session.

- Size: 15,162,256 bytes.
- SHA-256: `6a8b7df69be2bf2ba0bbdbc6c27898800bd362d5707888325b239d6f668938bc`.
- PE32 x86 Windows GUI executable, 12 sections; no CLR directory.
- Embedded description: Chimera mobile tool installer.
- Embedded file version: 13.88.1015.0. This is installer metadata, not a verified version of the subsequently installed service tool.
- Embedded company: Euroserver Sro.; signing certificate subject: Griff Gate Pte. Ltd., Singapore.
- Signing certificate issuer: DigiCert Trusted G4 Code Signing RSA4096 SHA384 2021 CA1; certificate validity 2024-08-30 through 2027-08-31.
- The calculated Authenticode image SHA-256 matches the signed image digest: `bd426879eeb7cdb0c7e5c015e4d1755996ad8eb7266734cd3a754812b23ea688`.
- RSA/SHA-256 signature over authenticated attributes verified with the matching embedded signer certificate; the signed content digest also matched.
- Full Windows trust policy, certificate revocation and timestamp-chain validation were not performed. A matching signature is not an antivirus verdict.
- PE sections include `.winlice` and `.boot`; most section names are blank. This alone does not establish maliciousness or expose the installed application's internal functionality.

The executable was not launched, installed, modified or uploaded to an external scanning service. No complete malware scan or sandbox behavior analysis was performed. Static installer inspection cannot prove device support, reveal server-side operations or establish a callable integration API. No proprietary binary code or assets were imported into NEXVARY-MR.
