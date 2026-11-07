"""Independent synthetic retained-report graph truth; not a parser/device corpus."""
import hashlib
import json
from pathlib import Path
import sys

REFERENCE = Path(__file__).resolve().parents[1] / "samples/artifact-index/reference.json"


def fixture():
    sha = lambda data: hashlib.sha256(data).hexdigest()
    original_a, original_b = b"SYNTHETIC shared original", b"SYNTHETIC different original"
    a, b, c, d = [sha(data) for data in (original_a, original_b, b"SYNTHETIC blob C", b"SYNTHETIC blob D")]
    sources = [("e1", "d1", a, len(original_a)), ("e2", "d1", a, len(original_a)), ("e3", "d2", b, len(original_b))]
    runs = []
    for i, (eid, did, digest, size) in enumerate(sources, 1):
        report = dict(schema="nexvary-authorized-import/v1", adapter="sqlite", parserVersion=1,
                      status="imported", sourceSha256=digest, sourceSize=size, findingsTruncated=False, observedCount=2,
                      provenance=dict(caseId="case-truth", evidenceId=eid, deviceId=did),
                      findings=[dict(reference="table/names/rowid/1/column/name", kind="sqlite-value", value="أحمد", valueTruncated=False),
                                dict(reference="table/blobs/rowid/1/column/data", kind="sqlite-blob-sha256", value=c if i < 3 else d, valueTruncated=False)])
        raw = json.dumps(report, sort_keys=True, ensure_ascii=False, separators=(",", ":"))
        runs.append(dict(id=f"r{i}", parser="authorized-import/sqlite", evidenceId=eid, sourceSha256=digest,
                         sourceSize=size, reportJson=raw, reportSha256=sha(raw.encode())))
    # Explicit truth: names and repeated text create ZERO identity/person edges.
    edges = [ ["case", "case-truth", "registers-device", "device", device] for device in ("d1", "d2") ]
    for i, (eid, did, digest, _) in enumerate(sources, 1):
        edges += [["device", did, "registers-evidence", "evidence", eid],
                  ["evidence", eid, "input-to-analysis", "analysis", f"r{i}"],
                  ["evidence", eid, "records-sha256", "recorded-sha256", digest],
                  ["analysis", f"r{i}", "reports-finding", "finding", f"r{i}:0"],
                  ["analysis", f"r{i}", "reports-finding", "finding", f"r{i}:1"],
                  ["finding", f"r{i}:1", "records-sha256", "recorded-sha256", c if i < 3 else d]]
    return dict(schema="nexvary-synthetic-artifact-truth/v1", synthetic=True,
                scope="Hand-labelled structural provenance/digest truth over synthetic retained reports; not parser, identity or device validation",
                snapshot=dict(id="case-truth", revision=7, synthetic=True,
                              devices=[dict(id="d1", model="Same label"), dict(id="d2", model="Same label")],
                              evidence=[dict(id=eid, deviceId=did, name="same-name.sqlite", sha256=digest, size=size) for eid, did, digest, size in sources],
                              analyses=runs),
                expected=dict(rows=6, nodes=19, edges=sorted(edges),
                              digestGroups={a: ["evidence/e1", "evidence/e2"], c: ["finding/r1:1", "finding/r2:1"]},
                              arabicLiteralMatches=3, nameBasedEdges=0, crossCaseEdges=0))


def main():
    expected = fixture()
    if sys.argv[1:] == ["--write"]:
        REFERENCE.parent.mkdir(parents=True, exist_ok=True)
        REFERENCE.write_text(json.dumps(expected, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    else:
        assert expected == json.loads(REFERENCE.read_text(encoding="utf-8")), "Reference differs from independent truth"
        assert len(expected["expected"]["edges"]) == 20
        print("Independent synthetic artifact reference: 6 findings, 19 nodes, 20 typed edges, 2 digest groups; zero name/person/cross-case links")


if __name__ == "__main__":
    main()
