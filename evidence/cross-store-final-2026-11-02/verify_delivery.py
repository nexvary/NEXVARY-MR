import argparse,hashlib,io,json,re,stat,struct,sys,zipfile
from pathlib import Path,PurePosixPath
p=argparse.ArgumentParser();p.add_argument('--repo',type=Path,required=True);p.add_argument('--sha',required=True);p.add_argument('--portable',type=Path,required=True);p.add_argument('--installer',type=Path,required=True);p.add_argument('--evidence',type=Path,required=True);p.add_argument('--metadata',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
def require(ok,message):
 if not ok:raise ValueError(message)
def sha(data):return hashlib.sha256(data).hexdigest()
meta=json.loads(a.metadata.read_text());report={'sourceCommit':a.sha,'providerArchives':{}}
for key,path in [('portable',a.portable),('installer',a.installer),('evidence',a.evidence)]:
 d=sha(path.read_bytes());require(d==meta[key]['digest'].removeprefix('sha256:'),key+' provider digest mismatch');report['providerArchives'][key]={'artifactId':meta[key]['id'],'sha256':d,'size':path.stat().st_size}
def checked(z):
 infos=z.infolist();require(len(infos)<=6000,'ZIP entry bound');require(sum(i.file_size for i in infos)<=1024**3,'ZIP expanded size bound');require(len({i.filename for i in infos})==len(infos),'duplicate ZIP name')
 for i in infos:
  n=i.filename.replace('\\','/');path=PurePosixPath(n)
  require(not path.is_absolute() and '..' not in path.parts and ':' not in n,'unsafe ZIP path')
  require(stat.S_IFMT(i.external_attr>>16)!=stat.S_IFLNK,'ZIP symlink refused')
  require(i.file_size<=256*1024**2,'ZIP member bound')
 return {i.filename:i for i in infos if not i.is_dir()}
def manifest(data):
 result={}
 for line in data.decode('utf-8-sig').splitlines():
  if not line.strip():continue
  h,n=line.split('  ',1);n=n.replace('\\','/')
  require(re.fullmatch('[0-9a-f]{64}',h) and n not in result,'invalid/duplicate checksum entry');result[n]=h
 return result
def pe(data,amd64=False):
 require(data[:2]==b'MZ' and len(data)>64,'PE DOS header');off=struct.unpack_from('<I',data,60)[0];require(data[off:off+4]==b'PE\0\0','PE signature');machine=struct.unpack_from('<H',data,off+4)[0]
 if amd64:require(machine==0x8664,'AMD64 required')
 return hex(machine)
with zipfile.ZipFile(a.portable) as outer:
 files=checked(outer);require(set(files)=={'NEXVARY-MR-Windows-x64-portable-unsigned.zip'},'portable outer inventory');inner_bytes=outer.read(next(iter(files)))
report['portableZipSha256']=sha(inner_bytes)
with zipfile.ZipFile(io.BytesIO(inner_bytes)) as z:
 files=checked(z);prefix='NEXVARY-MR/';require(all(n.startswith(prefix) for n in files),'bundle root');files={n[len(prefix):]:i for n,i in files.items()}
 sums=manifest(z.read(prefix+'SHA256SUMS.txt'));require(set(sums)==set(files)-{'SHA256SUMS.txt'},'manifest exact file set')
 for n,h in sums.items():
  digest=hashlib.sha256()
  with z.open(prefix+n) as f:
   while chunk:=f.read(1024**2):digest.update(chunk)
  require(digest.hexdigest()==h,'inner file checksum mismatch: '+n)
 profile=json.loads(z.read(prefix+'BUILD_PROFILE.json').decode('utf-8-sig'));require(profile['commit']==a.sha and profile['buildProfile']=='production-read-only' and profile['signed'] is False,'source/profile mismatch')
 require('mr_readiness_benchmark.exe' not in files,'test benchmark must not ship')
 exes={n:sha(z.read(prefix+n)) for n in ['NEXVARY-MR.exe','mr_evidence_import_worker.exe']}
 machines={n:pe(z.read(prefix+n),True) for n in exes}
 spdx=json.loads(z.read(prefix+'SBOM.spdx.json').decode('utf-8-sig'));sb={}
 for f in spdx['files']:
  n=f['fileName'].removeprefix('./').replace('\\','/');require(n not in sb,'duplicate SPDX file');hs=[v['checksum'] for v in f['checksums'] if v['algorithm']=='SHA256'];require(len(hs)==1 and sums.get(n)==hs[0],'SPDX hash mismatch');sb[n]=hs[0]
 require(set(sb)==set(files)-{'SHA256SUMS.txt','SBOM.spdx.json'},'SPDX exact file set')
 guides={}
 for name,source in [('GOVERNMENT_CASE_TRANSACTIONS.md','docs/GOVERNMENT_CASE_TRANSACTIONS.md'),('READINESS_BENCHMARK.md','docs/READINESS_BENCHMARK.md'),('verification/verify_policy_audit.py','scripts/verify_policy_audit.py')]:
  data=z.read(prefix+name);require(data.replace(b'\r\n',b'\n')==(a.repo/source).read_bytes().replace(b'\r\n',b'\n'),'shipped source text mismatch '+name);guides[name]=sha(data)
 report.update(profile=profile,manifestFiles=len(sums),spdxFiles=len(sb),executables=exes,peMachines=machines,shippedGuideHashes=guides,testBenchmarkExcluded=True)
with zipfile.ZipFile(a.installer) as z:
 files=checked(z);require(set(files)=={'NEXVARY-MR-Windows-x64-setup-unsigned.exe','SHA256SUMS.txt'},'installer outer inventory');setup=z.read('NEXVARY-MR-Windows-x64-setup-unsigned.exe');sums=manifest(z.read('SHA256SUMS.txt'))
 require(sums=={'NEXVARY-MR-Windows-x64-setup-unsigned.exe':sha(setup),'NEXVARY-MR-Windows-x64-portable-unsigned.zip':report['portableZipSha256']},'distribution checksums');report['setupSha256']=sha(setup);report['installerPeMachine']=pe(setup)
sys.path.insert(0,str(a.repo/'scripts'));from verify_policy_audit import verify
with zipfile.ZipFile(a.evidence) as z:
 files=checked(z)
 def find(suffix):
  matches=[n for n in files if n.replace('\\','/').endswith(suffix)];require(len(matches)==1,'unique receipt '+suffix);return z.read(matches[0])
 def js(s):return json.loads(find(s).decode('utf-8-sig'))
 for label in ['portable','installed']:
  r=js('dist/evidence/'+label+'.result.json');v=js('dist/evidence/'+label+'-government-policy-independent-verification.json')
  require(r['exitCode']==0 and r['sourceCommit']==a.sha and r['qtSdkExcludedFromPath'] is True and r['testFontOverrideCleared'] is True,'deployed source/lifecycle '+label)
  require(r['executableSha256']==exes['NEXVARY-MR.exe'] and r['importWorkerSha256']==exes['mr_evidence_import_worker.exe'],'actual deployed bytes mismatch '+label)
  require(v['valid'] is True and v['events']==23 and v['retainedPrefixVerified']==23,'hosted independent audit '+label)
  verified=verify(find('dist/evidence/'+label+'-government-policy-audit.json'),js('dist/evidence/'+label+'-government-policy-anchor.json'))
  require(verified['valid'] is True and verified['events']==23,'downloaded public audit '+label)
  report[label]={'mainAndWorkerBytesMatched':True,'exitCode':0,'independentAuditEvents':verified['events'],'scope':'synthetic same-run retained anchor, not institutional external trust'}
 l=js('dist/evidence/installer-lifecycle.json');require(l['installExit']==0 and l['uninstallExit']==0 and l['installedExecutableRemoved'] is True and l['perUserUninstallRegistrationRemoved'] is True,'installer removal lifecycle')
 report['installerLifecycle']={k:v for k,v in l.items() if k!='installationDirectory'}
 totals={};all_totals={};skips=[]
 for n in files:
  if n.startswith('build/') and n.endswith('-results.txt'):
   text=z.read(n).decode('utf-8',errors='replace');found=re.findall(r'Totals: (\d+) passed, (\d+) failed, (\d+) skipped',text)
   if found:
    all_totals[n]=[{'passed':int(p),'failed':int(f),'skipped':int(s)} for p,f,s in found]
    if any(k in n for k in ['case_transaction','partition_transaction','policy_audit']):totals[n]=all_totals[n]
   skips.extend({'file':n,'observation':line} for line in text.splitlines() if line.startswith('SKIP '))
 report['nativeScopedTotals']=totals;report['allNativeTotals']=all_totals;report['intentionalSkipObservations']=skips
 interop=js('build/policy-audit-interop-results.json');require(interop['passed']==len(interop['checks']) and all(c['passed'] is True for c in interop['checks']),'signed interoperability corpus');report['signedInteropChecks']=interop['passed']
 import xml.etree.ElementTree as ET
 xml=find('build/qt-results.xml');require(b'<!DOCTYPE' not in xml,'JUnit doctype refused');suite=ET.fromstring(xml)
 report['windowsCtest']={k:int(suite.attrib.get(k,0)) for k in ['tests','failures','errors','skipped']}
 require(report['windowsCtest']['tests']>0 and report['windowsCtest']['failures']==0 and report['windowsCtest']['errors']==0,'Windows CTest summary')
report['scope']='Unsigned hosted evaluation. No physical Windows10/11, phone, process-tree/network isolation, signing, independent institutional or competitor acceptance.'
a.output.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
