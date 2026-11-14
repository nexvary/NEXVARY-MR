"""Restore hash-verified, vendored MIT Fluent full-color SVGs without recoloring."""
from pathlib import Path
import hashlib,json
ROOT=Path(__file__).resolve().parents[1]
manifest=json.loads((ROOT/'assets/icons-professional/PROVENANCE.json').read_text())
for entry in manifest['files']:
 source=entry.get('raw_file',entry.get('alias_of',entry['file']))
 data=(ROOT/'assets/icon-sources/fluent'/source).read_bytes()
 if hashlib.sha256(data).hexdigest()!=entry['sha256']:raise SystemExit('Vendored icon digest mismatch: '+source)
 (ROOT/'assets/icons-professional'/entry['file']).write_bytes(data)
print('Restored',len(manifest['files']),'official color SVG mappings; no network or palette changes.')
