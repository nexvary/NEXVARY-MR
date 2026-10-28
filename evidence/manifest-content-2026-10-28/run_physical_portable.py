import argparse,hashlib,io,json,os,platform,subprocess,sys,zipfile
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('archive');p.add_argument('sha256');p.add_argument('commit');a=p.parse_args()
root=Path(__file__).resolve().parent
static=root/'physical-static.json'
subprocess.run([sys.executable,str(root/'verify_windows_bundle.py'),a.archive,'--provider-sha256',a.sha256,'--commit',a.commit,'--output',str(static)],check=True,timeout=120)
b=Path(a.archive).read_bytes();outer=zipfile.ZipFile(io.BytesIO(b));inner=zipfile.ZipFile(io.BytesIO(outer.read(outer.namelist()[0])))
destination=root/'portable-runtime-verified';destination.mkdir(exist_ok=False);inner.extractall(destination)
bundle=destination/'NEXVARY-MR';exe=bundle/'NEXVARY-MR.exe';worker=bundle/'mr_evidence_import_worker.exe'
env=dict(os.environ);env['PATH']=str(bundle)+';'+os.environ['SYSTEMROOT']+'\\System32;'+os.environ['SYSTEMROOT'];env['QT_QUICK_BACKEND']='software'
for key in ['QT_PLUGIN_PATH','QML_IMPORT_PATH','QML2_IMPORT_PATH','QT_QPA_FONTDIR']:env[key]=''
source=root/'synthetic-input.json';source.write_bytes(b'{"synthetic":"deployed import worker"}')
source_hash=hashlib.sha256(source.read_bytes()).hexdigest()
child=subprocess.run([str(worker),'--input',str(source),'--sha256',source_hash,'--adapter','json'],cwd=bundle,env=env,capture_output=True,timeout=15)
(root/'physical-worker.stderr.txt').write_bytes(child.stderr);(root/'physical-worker.json').write_bytes(child.stdout);assert child.returncode==0
report=json.loads(child.stdout);assert report['status']=='imported' and report['sourceSha256']==source_hash and report['findings'][0]['value']=='deployed import worker'
assert hashlib.sha256(source.read_bytes()).hexdigest()==source_hash
picture=root/'physical.png';run=subprocess.run([str(exe),'--smoke-test','--data-dir',str(root/'synthetic-case'),'--screenshot',str(picture)],cwd=bundle,env=env,capture_output=True,timeout=90)
(root/'physical.stdout.txt').write_bytes(run.stdout);(root/'physical.stderr.txt').write_bytes(run.stderr);assert run.returncode==0,run.returncode;assert picture.stat().st_size>1000
policy=json.loads((root/'physical-government-offline.json').read_text(encoding='utf-8-sig'))
assert policy['defaultDeniedCategories']==6 and policy['offlineAfterRevoke'] and policy['startupBoundaryDenials'] and policy['portalStoppedAfterRevoke'] and policy['signedPolicyAudit']
audit=subprocess.run([sys.executable,str(bundle/'verification'/'verify_policy_audit.py'),str(root/'physical-government-policy-audit.json'),'--anchor',str(root/'physical-government-policy-anchor.json')],capture_output=True,timeout=30)
(root/'physical-independent-audit.json').write_bytes(audit.stdout);assert audit.returncode==0;verified=json.loads(audit.stdout);assert verified['valid']
static_report=json.loads(static.read_text());hashes={n:hashlib.sha256((bundle/n).read_bytes()).hexdigest() for n in static_report['executables']};assert hashes==static_report['executables']
result={'sourceCommit':a.commit,'platform':platform.system(),'version':platform.version(),'profile':static_report['profile'],'providerSha256':a.sha256,'executables':hashes,'manifestFilesVerified':static_report['manifestFilesVerified'],'spdxFilesVerified':static_report['spdxFilesVerified'],'mainExit':run.returncode,'workerExit':child.returncode,'sdkExcludedFromPath':True,'softwareRenderer':True,'policyDefaultDeniedCategories':6,'auditValid':True,'auditEvents':verified['events'],'installationPerformed':False,'scope':'Existing Windows 10 host; synthetic direct portable smoke, not clean install/Windows 11/hardware-renderer/device acceptance'}
(root/'physical-result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
