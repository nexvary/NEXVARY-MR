"""Security regression tests for the fixed-repository Git dispatcher (no network)."""
import json
from pathlib import Path
from types import SimpleNamespace
import tempfile
import unittest
from unittest.mock import patch

SOURCE=Path(__file__).with_name("git-worker.py")
class BrokerTests(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory()
  self.addCleanup(self.tmp.cleanup)
  self.root=Path(self.tmp.name)
  (self.root/".git").mkdir()
  (self.root/"source.cpp").write_text("// fixture\n")
  self.ns={"__name__":"broker_test"}
  exec(compile(SOURCE.read_text().split("\ntry: main()")[0],str(SOURCE),"exec"),self.ns)
  self.ns["P"]=self.root
  self.calls=[]
  self.branch=self.ns["BRANCH"]
  self.staged="source.cpp\0"
  def git(args,capture=False):
   self.calls.append(args)
   value=self.branch if args[:1]==["symbolic-ref"] else self.staged if args[:2]==["diff","--cached"] else ""
   return SimpleNamespace(stdout=value)
  self.ns["git"]=git
 def invoke(self,*args):
  import os
  cwd=os.getcwd()
  try:
   with patch("sys.argv",["worker",json.dumps(args)]):self.ns["main"]()
  finally:os.chdir(cwd)
 def test_push_has_fixed_remote_and_exact_development_ref(self):
  self.invoke("push")
  self.assertEqual(self.calls[-1],["push","--no-verify","--porcelain",self.ns["REMOTE"],"HEAD:refs/heads/"+self.branch])
 def test_fetch_cannot_select_upload_program_or_remote(self):
  with self.assertRaises(ValueError):self.invoke("fetch","--upload-pack=sh")
  self.invoke("fetch")
  self.assertIn(self.ns["REMOTE"],self.calls[-1])
 def test_arbitrary_options_and_main_push_rejected(self):
  for args in [("-c","core.sshCommand=sh"),("push","main"),("push","--force"),("reset","--hard")]:
   with self.assertRaises(ValueError):self.invoke(*args)
 def test_protected_branch_cannot_commit_or_push(self):
  self.branch="main"
  for args in [("commit","test"),("push",)]:
   with self.assertRaises(ValueError):self.invoke(*args)
 def test_paths_cannot_escape_or_stage_credentials_and_policy(self):
  for name in ["../outside","/etc/passwd",".git/config",".opencode/opencode.json",".github/workflows/ci.yml",".aws/credentials",".env","private.pem","access_token.txt"]:
   with self.subTest(name=name),self.assertRaises(ValueError):self.invoke("add",name)
 def test_symlink_and_hardlink_rejected(self):
  import os
  (self.root/"link").symlink_to(self.root/"source.cpp")
  os.link(self.root/"source.cpp",self.root/"hard")
  for name in ["link","hard"]:
   with self.assertRaises(ValueError):self.invoke("add",name)
 def test_existing_protected_index_blocks_commit(self):
  self.staged=".opencode/opencode.json\0"
  with self.assertRaises(ValueError):self.invoke("commit","test")
 def test_message_is_an_argv_not_shell_code(self):
  message="test: $(touch /tmp/must-not-run); literal"
  self.invoke("commit",message)
  self.assertEqual(self.calls[-1],["commit","--no-verify","--no-gpg-sign","-m",message])
 def test_hooks_and_user_ssh_configuration_disabled(self):
  cmd=self.ns["BASE"]
  self.assertIn("core.hooksPath=/dev/null",cmd)
  self.assertIn("core.fsmonitor=false",cmd)
  ssh=next(x for x in cmd if x.startswith("core.sshCommand="))
  for flag in ["-F /dev/null","StrictHostKeyChecking=yes","ProxyCommand=none","PermitLocalCommand=no"]:self.assertIn(flag,ssh)
 def test_authorized_source_add_uses_path_separator(self):
  self.invoke("add","source.cpp")
  self.assertEqual(self.calls[-1],["add","--","source.cpp"])
if __name__=="__main__":unittest.main()
