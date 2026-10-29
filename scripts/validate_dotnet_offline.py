#!/usr/bin/env python3
"""Offline, no-restore Host reproducibility check; never downloads dependencies."""
import os
from pathlib import Path
import shutil
import subprocess
import sys
ROOT=Path(__file__).resolve().parents[1]
TARGETS={
  "cli":"host/src/DeusOs.Control.Cli/DeusOs.Control.Cli.csproj",
  "recovery":"host/tests/DeusOs.Control.Recovery.Acceptance/DeusOs.Control.Recovery.Acceptance.csproj",
  "core":"host/tests/DeusOs.Control.Core.Tests/DeusOs.Control.Core.Tests.csproj",
  "transport":"host/tests/DeusOs.Control.Transport.Tests/DeusOs.Control.Transport.Tests.csproj",
}
def main():
 if len(sys.argv)!=2 or sys.argv[1] not in TARGETS:
  print("usage: validate_dotnet_offline.py cli|recovery|core|transport",file=sys.stderr);return 2
 exe=shutil.which("dotnet")
 if not exe:
  sdk_root=os.environ.get("DOTNET_ROOT")
  candidate=(Path(sdk_root)/("dotnet.exe" if os.name=="nt" else "dotnet")) if sdk_root else None
  if candidate and candidate.is_file() and os.access(candidate,os.X_OK):
   exe=str(candidate)
  else:
   print("DOTNET_SDK_UNAVAILABLE_IN_VALIDATION_RUNNER");return 75
 target=ROOT/TARGETS[sys.argv[1]]
 if not target.is_file():print("PROJECT_NOT_FOUND");return 2
 env=os.environ.copy()
 env.update(DOTNET_SKIP_FIRST_TIME_EXPERIENCE="1",DOTNET_CLI_TELEMETRY_OPTOUT="1",DOTNET_NOLOGO="1",DOTNET_SYSTEM_GLOBALIZATION_INVARIANT="1")
 args=[exe,"build",str(target),"-c","Release","--no-restore","-warnaserror","-v:minimal"]
 try:
  r=subprocess.run(args,cwd=ROOT/"host",env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,
    text=True,timeout=43)
 except subprocess.TimeoutExpired as e:
  print("DOTNET_OFFLINE_NO_RESTORE=TIMEOUT\n"+str(e.output or "")[-3000:]);return 124
 print("DOTNET_OFFLINE_NO_RESTORE="+("PASS" if r.returncode==0 else "FAIL"))
 print("TARGET="+TARGETS[sys.argv[1]])
 print("EXIT="+str(r.returncode))
 print(r.stdout[-5000:])
 return r.returncode
if __name__=="__main__":sys.exit(main())
