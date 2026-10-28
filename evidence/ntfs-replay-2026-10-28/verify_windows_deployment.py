import sys
if sys.flags.optimize:raise RuntimeError('Optimization must be disabled')
import hashlib,json,struct,zipfile
from pathlib import Path
root=Path('evidence/ntfs-replay-2026-10-28');static=json.loads((root/'portable-static.json').read_text());receipts=root/'windows-production/dist/evidence'
z=zipfile.ZipFile(sys.argv[1]);names=z.namelist();assert len(names)==2 and set(names)=={'NEXVARY-MR-Windows-x64-setup-unsigned.exe','SHA256SUMS.txt'}
exe=z.read(names[0] if names[0].endswith('.exe') else names[1]);offset=struct.unpack_from('<I',exe,60)[0];assert exe[:2]==b'MZ' and exe[offset:offset+4]==b'PE\0\0'
setup=hashlib.sha256(exe).hexdigest();manifest={}
for line in z.read('SHA256SUMS.txt').decode('utf-8-sig').splitlines():
 if line.strip():
  h,n=line.split('  ',1);assert n not in manifest;manifest[n]=h
assert manifest=={'NEXVARY-MR-Windows-x64-setup-unsigned.exe':setup,'NEXVARY-MR-Windows-x64-portable-unsigned.zip':static['portableZipSha256']}
report={'setupSha256':setup,'portableZipSha256':static['portableZipSha256'],'actualSetupPEVerified':True}
for label in ['portable','installed']:
 r=json.loads((receipts/(label+'.result.json')).read_text());a=json.loads((receipts/(label+'-government-policy-independent-verification.json')).read_text())
 assert r['exitCode']==0 and r['sourceCommit']==static['profile']['commit'] and r['qtSdkExcludedFromPath'] is True and r['testFontOverrideCleared'] is True
 assert r['executableSha256']==static['executables']['NEXVARY-MR.exe'] and r['importWorkerSha256']==static['executables']['mr_evidence_import_worker.exe']
 assert a['valid'] is True and a['events']==23 and a['retainedPrefixVerified']==23
 report[label]={'actualMainWorkerHashesMatched':True,'exitCode':0,'independentAuditEvents':a['events'],'auditValid':a['valid']}
l=json.loads((receipts/'installer-lifecycle.json').read_text());assert l['installExit']==0 and l['uninstallExit']==0 and l['launchMode']=='installed-in-place' and l['installedExecutableRemoved'] is True and l['perUserUninstallRegistrationRemoved'] is True
report['lifecycle']={k:v for k,v in l.items() if k!='installationDirectory'}
report['scope']='Current desktop/worker deployment only; NTFS v2/replay is test-target-only, not an enabled case adapter; physical host offline.'
(root/'hosted-binary-verification.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
