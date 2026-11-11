"""Build MR metallic variants of the bundled MIT Fluent vectors; no bitmap icons."""
from pathlib import Path
import colorsys
import hashlib
import json
import re
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
ET.register_namespace('', 'http://www.w3.org/2000/svg')
NS = '{http://www.w3.org/2000/svg}'
items = []
for i, source in enumerate(sorted((ROOT / 'assets/icons').glob('*.svg'))):
    root = ET.parse(source).getroot()
    accent = ['#6875EA', '#28BDEB', '#D6556F'][i % 3]
    # Preserve conventional success/warning meanings in the semantic status glyphs.
    semantic = source.stem in {'check_mark', 'checkmark_circle', 'shield_checkmark', 'warning', 'error_circle'}
    rgb = tuple(int(accent[j:j+2], 16) / 255 for j in (1, 3, 5))
    hue = colorsys.rgb_to_hls(*rgb)[0]
    def metal_color(match):
        raw = match.group()[1:]
        if len(raw) == 3:
            raw = ''.join(c * 2 for c in raw)
        channels = tuple(int(raw[j:j+2], 16) / 255 for j in (0, 2, 4))
        _, light, sat = colorsys.rgb_to_hls(*channels)
        if semantic or sat < .12:
            return match.group()
        out = colorsys.hls_to_rgb(hue, max(.15, min(.91, light)), .62)
        return '#' + ''.join(f'{round(c*255):02X}' for c in out)
    body = ''.join(ET.tostring(child, encoding='unicode') for child in root)
    body = re.sub(r'#[0-9a-fA-F]{6}\b|#[0-9a-fA-F]{3}\b', metal_color, body)
    # Original vector coordinates remain intact. Chrome surround is independently authored.
    svg = f'''<svg xmlns="http://www.w3.org/2000/svg" width="64" height="64" viewBox="0 0 64 64">
<defs>
 <linearGradient id="mrPlate" x1="0" y1="0" x2="1" y2="1"><stop stop-color="#253E56"/><stop offset=".43" stop-color="#0B1726"/><stop offset="1" stop-color="#173657"/></linearGradient>
 <linearGradient id="mrSilver" x1="0" y1="0" x2="1" y2="1"><stop stop-color="#F3FAFF"/><stop offset=".3" stop-color="#9AB9CB"/><stop offset=".52" stop-color="#E1F1F9"/><stop offset="1" stop-color="#4E738E"/></linearGradient>
</defs>
<rect x="3" y="3" width="58" height="58" rx="13" fill="#02070C"/>
<rect x="5" y="4" width="54" height="54" rx="12" fill="url(#mrPlate)" stroke="url(#mrSilver)" stroke-width="1.4"/>
<path d="M17 7H47" stroke="#CBE9F8" stroke-opacity=".7" stroke-linecap="round"/>
<path d="M17 56H47" stroke="{accent}" stroke-opacity=".9" stroke-width="2" stroke-linecap="round"/>
<g transform="translate(11 10) scale(1.75)">{body}</g>
</svg>'''
    target = ROOT / 'assets/icons-metallic' / source.name
    target.write_text(svg)
    items.append({'file': source.name, 'source_sha256': hashlib.sha256(source.read_bytes()).hexdigest(), 'sha256': hashlib.sha256(target.read_bytes()).hexdigest(), 'accent': accent})
(ROOT / 'assets/icons-metallic/PROVENANCE.json').write_text(json.dumps({'license':'MIT; see packaging/licenses/FLUENT-ICONS-MIT.txt', 'base_provenance':'assets/icons/PROVENANCE.json', 'variant':'Independently authored chrome surround and indigo/cyan/red material treatment; semantic colors preserved', 'files':items}, indent=2)+'\n')
print(f'Built {len(items)} metallic vector icons')
