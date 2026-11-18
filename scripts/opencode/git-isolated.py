#!/usr/bin/python3
import json, subprocess, sys
from pathlib import Path
args=json.loads(sys.argv[1])
if not isinstance(args,list) or not args or any(not isinstance(s,str) or len(s)>500 for s in args) or len(args)>101: raise SystemExit(64)
if args[0] not in {"status","diff","branches","log","add","commit","push","fetch"}: raise SystemExit(64)
p="/home/ubuntu/nexvary/projects/NEXVARY-MR"
if Path(p).resolve()!=Path(p) or Path(p+"/.git").is_symlink(): raise SystemExit(64)
network=args[0] in {"push","fetch"}
cmd=["/usr/bin/systemd-run","--quiet","--wait","--pipe","--collect"]
props=["User=ubuntu","Group=ubuntu","WorkingDirectory="+p,"ProtectHome=tmpfs","BindPaths="+p,"BindReadOnlyPaths=/etc/nexvary-mr-git/config:"+p+"/.git/config","ProtectSystem=strict","PrivateTmp=yes","PrivateDevices=yes","NoNewPrivileges=yes","ProtectKernelTunables=yes","ProtectControlGroups=yes","InaccessiblePaths=/proc","TemporaryFileSystem=/run","RuntimeMaxSec=90","MemoryMax=384M","TasksMax=32"]
if network: props+=["BindReadOnlyPaths=/home/ubuntu/.ssh/nexvary_mr_deploy:/run/nexvary-git-key /home/ubuntu/.ssh/known_hosts:/run/nexvary-git-hosts","BindReadOnlyPaths=/run/systemd/resolve","RestrictAddressFamilies=AF_UNIX AF_INET AF_INET6"]
else: props+=["PrivateNetwork=yes"]
for prop in props:cmd+=["-p",prop]
cmd+=["--","/usr/bin/env","-i","PATH=/usr/bin:/bin","HOME=/tmp","GIT_CONFIG_NOSYSTEM=1","GIT_CONFIG_GLOBAL=/dev/null","GIT_TERMINAL_PROMPT=0","/usr/bin/python3","-B","/usr/local/libexec/nexvary-mr-git-worker.py",json.dumps(args)]
raise SystemExit(subprocess.run(cmd).returncode)
