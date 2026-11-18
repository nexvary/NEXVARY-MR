#!/usr/bin/python3
"""Fixed-repository Git dispatcher. Runs only inside the isolated service."""
import json, os, re, subprocess, sys
from pathlib import Path
P=Path("/home/ubuntu/nexvary/projects/NEXVARY-MR")
BRANCH="feat/intelligence-ux-polish-20261117"
REMOTE="git@github.com:nexvary/NEXVARY-MR.git"
BASE=["/usr/bin/git","--no-pager","-c","core.hooksPath=/dev/null","-c","core.fsmonitor=false","-c","commit.gpgsign=false","-c","tag.gpgsign=false","-c","protocol.allow=never","-c","protocol.ssh.allow=always","-c","core.sshCommand=/usr/bin/ssh -F /dev/null -i /run/nexvary-git-key -o IdentitiesOnly=yes -o BatchMode=yes -o StrictHostKeyChecking=yes -o UserKnownHostsFile=/run/nexvary-git-hosts -o GlobalKnownHostsFile=/dev/null -o ProxyCommand=none -o PermitLocalCommand=no"]
def git(args, capture=False):
 return subprocess.run(BASE+args,check=True,stdout=subprocess.PIPE if capture else None,stderr=subprocess.PIPE if capture else None,text=True)
def safe_path(s):
 p=P/s
 if not s or s.startswith("-") or any(c in s for c in "\0\n\r") or Path(s).is_absolute() or ".." in Path(s).parts: raise ValueError("Invalid path")
 if any(part in {".git",".opencode",".ssh",".aws",".github","node_modules",".venv"} or part.startswith(".env") for part in Path(s).parts): raise ValueError("Protected path")
 if p.resolve()!=p.absolute() or (p.exists() and (not p.is_file() or p.stat().st_nlink!=1 or p.stat().st_size>5_000_000)): raise ValueError("Not an authorized regular file")
 if re.search(r"(?i)(private|credential|token|secret|\.pem$|\.ppk$)",s): raise ValueError("Sensitive path")
 return s
def main():
 args=json.loads(sys.argv[1]); op=args[0]; rest=args[1:]
 if P.resolve()!=P or (P/".git").is_symlink() or not (P/".git").is_dir(): raise ValueError("Repository escape")
 if any((P/".git"/f).exists() for f in ["commondir","objects/info/alternates","config.worktree","shallow.lock"]): raise ValueError("Unsupported repository indirection")
 os.chdir(P)
 branch=git(["symbolic-ref","--short","HEAD"],True).stdout.strip()
 if branch!=BRANCH: raise ValueError("Only the approved development branch is writable")
 if op=="status" and not rest: git(["--no-optional-locks","status","--short","--branch"])
 elif op=="diff" and not rest: git(["--no-optional-locks","diff","--no-ext-diff","--no-textconv","--stat"])
 elif op=="branches" and not rest: git(["branch","--list"])
 elif op=="log" and not rest: git(["log","-5","--oneline"])
 elif op=="add" and 0<len(rest)<=100: git(["add","--"]+[safe_path(p) for p in rest])
 elif op=="commit" and len(rest)==1 and 1<=len(rest[0])<=200 and "\n" not in rest[0]:
  names=git(["diff","--cached","--name-only","-z"],True).stdout.split("\0")
  for p in filter(None,names): safe_path(p)
  git(["commit","--no-verify","--no-gpg-sign","-m",rest[0]])
 elif op=="push" and not rest:
  git(["push","--no-verify","--porcelain",REMOTE,"HEAD:refs/heads/"+BRANCH])
 elif op=="fetch" and not rest:
  git(["fetch","--no-tags","--no-recurse-submodules",REMOTE,"refs/heads/"+BRANCH+":refs/remotes/origin/"+BRANCH,"refs/heads/fix/intelligence-p0-completion:refs/remotes/origin/fix/intelligence-p0-completion"])
 else: raise ValueError("Unsupported Git operation")
try: main()
except (ValueError,KeyError,IndexError,json.JSONDecodeError) as e: print("DENIED:",e);sys.exit(64)
except subprocess.CalledProcessError as e:
 if e.stderr: print(e.stderr[:2000])
 sys.exit(e.returncode)
