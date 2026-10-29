#!/usr/bin/env python3
"""Offline smoke for fresh committed Git source without off-Git STM32 custody.

Exports the local committed HEAD into a temporary directory. Runs source-only
BundleGuard and Gate4 negative regressions without owner Evidence, signed
packages, hardware, network, signing secrets or mutation of original Git.
"""
import os
from pathlib import Path
import re
import subprocess
import sys
import tarfile
import tempfile
ROOT=Path(__file__).resolve().parents[1]
def run(args,cwd,env,timeout):
 return subprocess.run(args,cwd=cwd,env=env,stdout=subprocess.PIPE,
                       stderr=subprocess.STDOUT,text=True,timeout=timeout)
def main():
 env=os.environ.copy()
 env["PYTHONDONTWRITEBYTECODE"]="1"
 env["GIT_TERMINAL_PROMPT"]="0"
 with tempfile.TemporaryDirectory(prefix="deus_source_checkout_") as tmp:
  clone=Path(tmp)/"OS"
  clone.mkdir()
  archive=Path(tmp)/"committed-head.tar"
  with archive.open("wb") as out:
   a=subprocess.run(["git","archive","--format=tar","HEAD"],cwd=ROOT,env=env,stdout=out,
                     stderr=subprocess.PIPE,text=True,timeout=23)
  if a.returncode:
   print("COMMITTED_HEAD_EXPORT=FAIL "+a.stderr[-1200:]);return 1
  with tarfile.open(archive) as members:
   members.extractall(clone,filter="data")
  if any((clone/x).exists() for x in ("Evidence","ProtectedRecovery","Bundle","Reports","Tools")):
   print("OFF_GIT_ARTIFACT_LEAK_IN_CLONE=FAIL");return 1
  print("COMMITTED_SOURCE_EXPORT=PASS NO_OFFGIT_ARTIFACT_ROOTS=PASS")
  for name,folder in (("bundle","scripts/operator_bundle_guard"),("gate4","scripts/gate4_entry")):
   a=run([sys.executable,"-B","-m","unittest","discover","-s",folder,
           "-p","test_*.py","-v"],clone,env,45)
   matches=re.findall(r"Ran (\d+) tests?",a.stdout)
   summaries=re.findall(r"(?m)^OK(?: \\(skipped=\\d+\\))?$",a.stdout)
   last=summaries[-1] if summaries else (a.stdout.strip().splitlines()[-1] if a.stdout.strip() else "<no output>")
   print(f"ISOLATED_{name.upper()}_TESTS={'PASS' if a.returncode==0 else 'FAIL'} count={matches[-1] if matches else 'UNKNOWN'} detail={last}")
   if a.returncode:
    failed=re.findall(r"(?m)^(?:FAIL|ERROR): ([^\n]+)",a.stdout)
    print("ISOLATED_FAILURES="+repr(failed))
    print(a.stdout[-1400:])
    return 1
  print("CLEAN_COMMITTED_SOURCE=PASS BENCH_SIGNED_PACKAGES_AND_WINDOWS_DOTNET=NOT_ATTESTED")
  return 0
if __name__=="__main__":sys.exit(main())
