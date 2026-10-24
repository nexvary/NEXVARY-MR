# Active engineering direction — 2026-10-24

NEXVARY-MR prioritizes mobile repair, device intelligence, OEM architecture,
firmware analysis, USB diagnostics and professional training. New Digital
Forensic Laboratory features are paused. Preserve its existing UI/services and
regression coverage; make only critical defect/security fixes there.

The first new slice is USB Doctor: timestamped evidence, explicit unknowns,
source-scoped diagnosis and non-destructive next checks. See USB_DOCTOR.md.
It is not a replacement for model acceptance or a low-level OEM service adapter.

Next independent slices, after completing this slice's build/test/CI gates:

1. Field-level device intelligence/provenance using existing getprop and reviewed
   health/capability probes; no database-name-based support grants.
2. Firmware risk analysis: declared identity, layout/AVB limits and downgrade
   risks, with trusted-key verification only where justified by actual keys.
3. OEM adapter contracts separating discovery, compatibility, execution and
   post-condition verification. Existing ADB/Fastboot policy stays authoritative.
4. Isolated Professional Training Lab: virtual faults, evidence-driven decisions,
   scoring and action reports without addressing real devices.

Production/Laboratory are independently compiled profiles. Training simulation
must never share an executor or enable device writes; research is local analysis.
Authentication, ownership and customer-data protections remain in force.

Commercial comparison extends the existing COMPETITIVE_ENGINEERING.md and
../ui/competitive-ui-review.md. A vendor list or feature claim is not a NEXVARY
model/Android/build test. See CAPABILITY_EVIDENCE_MATRIX.md for this increment's
narrow, evidence-qualified mapping and explicit research gaps.
