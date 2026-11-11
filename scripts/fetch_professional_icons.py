"""Fetch pinned, MIT-licensed Fluent icons and build unframed MR color variants."""
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from urllib.parse import quote
from urllib.request import urlopen
import hashlib
import json
import re
import colorsys

ROOT=Path(__file__).resolve().parents[1]
meta=json.loads((ROOT/'assets/icons/PROVENANCE.json').read_text())
revision=meta['revision']
base='https://raw.githubusercontent.com/microsoft/fluentui-system-icons/'+revision+'/'
raw_dir=ROOT/'assets/icon-sources/fluent'
out_dir=ROOT/'assets/icons-professional'
raw_dir.mkdir(parents=True,exist_ok=True);out_dir.mkdir(parents=True,exist_ok=True)
def fetch(entry):
    with urlopen(base+quote(entry['upstream'],safe='/'),timeout=45) as response:
        data=response.read(256*1024)
    assert b'<svg' in data and len(data)<256*1024,entry['file']
    expected=entry.get('sha256')
    assert not expected or hashlib.sha256(data).hexdigest()==expected,entry['file']
    (raw_dir/entry['file']).write_bytes(data)
    text=data.decode()
    if entry['file']=='home.svg':
        palette={'#944600':'#172540','#CD8E02':'#465C83','#FFD394':'#EFF4F8','#FFB357':'#819AAF','#FF921F':'#8F9DF7','#EB4824':'#29398E','#FF5B00':'#596ADD','#FFB24C':'#A9B5FA'}
        text=re.sub(r'#[0-9A-Fa-f]{6}',lambda m:palette.get(m[0].upper(),m[0]),text)
    elif entry['file'] not in {'shield_checkmark.svg','checkmark_circle.svg','warning.svg','error_circle.svg'}:
        def recolor(match):
            rgb=tuple(int(match[0][j:j+2],16)/255 for j in (1,3,5))
            h,l,s=colorsys.rgb_to_hls(*rgb)
            # Warm decorative accents become restrained crimson, existing silver stays silver.
            if s>.35 and (h<.16 or h>.94):
                rgb=colorsys.hls_to_rgb(.965,max(.26,min(.82,l)),min(.72,s))
                return '#'+''.join(f'{round(v*255):02X}' for v in rgb)
            return match[0]
        text=re.sub(r'#[0-9A-Fa-f]{6}',recolor,text)
    (out_dir/entry['file']).write_text(text)
    return {'file':entry['file'],'url':base+quote(entry['upstream'],safe='/'),'source_sha256':hashlib.sha256(data).hexdigest(),'sha256':hashlib.sha256(text.encode()).hexdigest()}
with ThreadPoolExecutor(max_workers=6) as pool:
    results=list(pool.map(fetch,meta['files']))
with urlopen(base+'LICENSE',timeout=45) as response:
    license_data=response.read(128*1024)
assert b'MIT' in license_data
(raw_dir/'LICENSE').write_bytes(license_data)
# Retain the existing independently authored check mark used by the checkbox control.
check=(ROOT/'assets/icons/check_mark.svg').read_bytes()
(out_dir/'check_mark.svg').write_bytes(check)
results.append({'file':'check_mark.svg','source':'existing MR checkbox glyph','sha256':hashlib.sha256(check).hexdigest()})
(out_dir/'PROVENANCE.json').write_text(json.dumps({'repository':'https://github.com/microsoft/fluentui-system-icons','revision':revision,'license':'MIT; assets/icon-sources/fluent/LICENSE','treatment':'Unframed original geometry; selected silver/indigo/crimson palette adjustments. Semantic status colors preserved.','files':results},indent=2)+'\n')
print(f'Downloaded {len(meta["files"])} official vectors and license; built {len(results)} unframed assets')
