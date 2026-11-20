# Recovery assessment integration contract

This component is a local, non-privileged decision aid, not a data-recovery engine.

## Inputs

`assessment_cli.py` accepts one JSON object via standard input or `--input path.json`:

```json
{"source":"external_card","factory_reset":true,"authorized":true,"readable":true}
```

Sources: `external_card`, `accessible_backup`, `cloud_export`, `internal_storage`. The two mandatory boolean flags are `factory_reset` and `authorized`. `readable` is optional and may be null.

## Output

A successful assessment returns a JSON object with `schema: 1`, `verdict`, `next_action`, and `limitations`. Invalid requests return an error on stderr and exit code 2. Do not interpret `acquire_copy` as recovered data.

## Desktop integration constraints

- Execute as a bounded child process, with no shell interpolation.
- Obtain explicit owner authorization before acquisition; never run privileged or destructive operations based solely on the verdict.
- Treat input and output as untrusted. Validate schema, response size, exit code, and timeout.
- Show limitations alongside any recommendation; never display a recovery percentage without measured evidence.
- Do not distribute Python as a mandatory Windows dependency until packaging and licensing have been reviewed.
- The Qt UI is not yet connected to this component.

## Verification

```sh
python -m unittest discover -s research/android-recovery -p 'test_*.py' -v
```

CI runs this suite. A factory reset of modern encrypted internal Android storage normally destroys the required encryption keys; Shizuku cannot restore them.
