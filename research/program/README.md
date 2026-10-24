# NEXVARY Advanced Mobile Technology Research Program

An offline-first, evidence-based R&D foundation for NEXVARY-MR. This package is a proposed repository addition, **not yet merged or deployed**.

## Three parallel tracks
1. **Commercial product:** mobile repair, firmware intelligence, USB diagnostics, OEM service workflows, device compatibility.
2. **Research laboratory:** mobile architecture knowledge base, test plans, reproducible experiments on owned/authorized lab devices, independent published forensic tool analysis.
3. **Institutional capabilities:** case workflows, provenance, custody, evidence integrity, role-based access, training and defensible reporting.

## First unit: Advanced Forensic Technology Intelligence

Study documented architecture, research, published security analyses and interoperability of Cellebrite, Oxygen Forensics, MSAB, Magnet, Hacking Team RCS and related products. **Do not import leaked source, cracked binaries, credentials, spyware payloads, exploit chains, or proprietary data.** Record only lawfully accessible sources and defensible observations. The unit does not provide PIN/FRP bypass or extraction from locked devices.

## Evidence classes
- `verified`: reproduced with attached lab evidence, device/software versions and independent reviewer.
- `documented`: credible public source supports the claim, but NEXVARY has not reproduced it.
- `hypothesis`: untested technical proposition; must not be advertised as a product capability.
- `rejected`: disproven by repeatable experiment or reliable source.

## Workflow
1. Register a public source with publication date, publisher, URL and license/access notes.
2. Record the specific technical claim with limitations and citation.
3. Define an experiment on synthetic data or an authorized laboratory device.
4. Record reproducible results and hash the attached evidence; require reviewer approval before classifying as `verified`.
5. Link verified, authorized, non-destructive capabilities to the NEXVARY-MR product backlog; keep research-only results segregated.

## Structure
- `research/knowledge/`: architecture and competitor research
- `research/sources/`: source registry
- `research/compatibility/`: testable compatibility entries
- `research/experiments/`: test plans
- `research/results/`: immutable-style result entries (append revisions; preserve history in Git)
- `tools/validate_research.py`: offline schema/consistency checks
- `tests/test_research.py`: validator tests

## Local validation
`python tools/validate_research.py`
`python -m unittest discover -s tests -v`

## Integration steps for NEXVARY-MR
- Review current repository layout and choose a stable location, e.g. `research/advanced-mobile/`.
- Integrate validation into GitHub Actions; require source URLs and provenance.
- Implement read-only research catalog in existing Qt/QML UI; do not copy data into hardcoded UI strings.
- Use device catalog identifiers to link compatibility to existing Device Intelligence.
- Add search, filters, review workflow and export after tests pass.
- Capture genuine Windows screenshots and record CI run URLs before marking implemented.
