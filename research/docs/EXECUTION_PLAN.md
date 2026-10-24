# Engineering delivery gates

## Gate 1 — Research data foundation
- Validate compatibility JSON against schema in CI
- Implement source registry and provenance requirements
- Reject VERIFIED without reproducible test report and reviewer
- Add Atlas records with status REPORTED and primary source references

## Gate 2 — Commercial integration
- Read-only Atlas browser in existing Qt/QML Device Intelligence
- Model/build/patch filters and clear unknown state
- No cross-model inference; no action buttons from research data
- RTL Arabic and English, genuine licensed colored icons
- Qt and Windows CI tests, genuine screenshots

## Gate 3 — Laboratory reproducibility
- Owned/authorized test-device register
- Experiment runner for non-destructive diagnostic observations
- Immutable test artifacts with SHA-256
- Reviewer sign-off and regression coverage

## Gate 4 — Institutional integration
- Cross-reference Law Enforcement Case ID and legal scope without copying sensitive records into research datasets
- Case-level provenance, audit events, evidence integrity
- Offline operation, permissions, encrypted persistence and data minimization

## Gate 5 — Release readiness
- Reproducible Windows builds and installers
- Device capability claims backed by specific tested model/build
- Documentation, licensing, third-party notices and security review

Each gate requires a tested commit, CI evidence and explicit list of unverified items. Do not merge incomplete features into main.
