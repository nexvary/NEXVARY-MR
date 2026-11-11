# Approved MR visual reference

`mr-osint-background.png` is the exact user-supplied/approved background image,
provided in this conversation as 248145.png. Original pixels retained, 1536 x 864.
SHA-256: c0a08516c605b3af8b4f9ca21a4e7d642b872044b65cf0fa3c04a9bf68c3b569.
No Lampyre/client artwork is used in this asset.

The user's newer explicit color selection supersedes the previous light OSINT
palette. Default dark MR tokens use chair-like near-black #040507, midnight
#08121B, petrol #0A2231, cyan #23B9E7 and silver #BCCED8. Interface colors are
curated from the visual reference family; they are not all literal single-pixel
samples. Semantic success/error colors remain readable and distinct.

Resource is bundled for both the application and Qt UI tests. OsintBackdrop
applies a separate UI dimming overlay; the PNG is not modified. Existing light
mode remains available; OSINT uses the newly approved dark palette.
