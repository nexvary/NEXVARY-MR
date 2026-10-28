import sys
if sys.flags.optimize: raise RuntimeError('Run with Python optimization disabled')
import argparse,hashlib,io,json,struct,zipfile
from pathlib import Path,PurePosixPath

def digest(b):return hashlib.sha256(b).hexdigest()
def members(z):
 out={}
 for info in z.infolist():
  if info.is_dir():continue
  name=info.filename
  if name in out or '\\' in name or PurePosixPath(name).is_absolute() or '..' in PurePosixPath(name).parts or info.file_size>256*1024*1024:raise ValueError('unsafe/oversized/duplicate archive member')
  out[name]=info
 if sum(x.file_size for x in out.values())>1024*1024*1024:raise ValueError('archive aggregate limit')
 return out

def pe(b):
 assert b[:2]==b'MZ' and len(b)>=64
 off=struct.unpack_from('<I',b,60)[0]
 assert off+24<=len(b) and b[off:off+4]==b'PE\0\0' and struct.unpack_from('<H',b,off+4)[0]==0x8664

def main():
 p=argparse.ArgumentParser();p.add_argument('portable');p.add_argument('--provider-sha256',required=True);p.add_argument('--commit',required=True);p.add_argument('--output',required=True);a=p.parse_args()
 data=Path(a.portable).read_bytes();assert digest(data)==a.provider_sha256
 outer=zipfile.ZipFile(io.BytesIO(data));outer_files=members(outer);assert len(outer_files)==1
 name=next(iter(outer_files));assert name.endswith('portable-unsigned.zip');portable=outer.read(name)
 inner=zipfile.ZipFile(io.BytesIO(portable));files=members(inner);prefix='NEXVARY-MR/';assert all(n.startswith(prefix) for n in files)
 content={n[len(prefix):]:inner.read(n) for n in files}
 manifest={}
 for line in content['SHA256SUMS.txt'].decode('utf-8-sig').splitlines():
  h,n=line.split('  ',1);n=n.replace('\\','/');assert n not in manifest and len(h)==64;manifest[n]=h
 assert set(manifest)==set(content)-{'SHA256SUMS.txt'}
 for n,h in manifest.items():assert digest(content[n])==h,n
 profile=json.loads(content['BUILD_PROFILE.json'].decode('utf-8-sig'));assert profile['commit']==a.commit and profile['buildProfile']=='production-read-only'
 sbom=json.loads(content['SBOM.spdx.json'].decode('utf-8-sig'));records={}
 for r in sbom['files']:
  n=r['fileName'].removeprefix('./').replace('\\','/');assert n not in records;records[n]=r
 assert set(records)==set(content)-{'SHA256SUMS.txt','SBOM.spdx.json'}
 for n,r in records.items():assert any(c['algorithm']=='SHA256' and c['checksum']==digest(content[n]) for c in r['checksums']),n
 hashes={}
 for n in ['NEXVARY-MR.exe','mr_evidence_import_worker.exe']:
  pe(content[n]);hashes[n]=digest(content[n])
 report={'providerSha256':digest(data),'portableZipSha256':digest(portable),'profile':profile,'manifestFilesVerified':len(manifest),'spdxFilesVerified':len(records),'executables':hashes,'peMachine':'AMD64','scope':'Static full manifest/file-set/SPDX/PE verification; launch acceptance is separate'}
 Path(a.output).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
if __name__=='__main__':main()
