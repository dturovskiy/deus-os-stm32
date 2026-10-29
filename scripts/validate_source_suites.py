#!/usr/bin/env python3
"""Deterministic offline Python regression entry for CI/local reproducibility.

Usage: python scripts/validate_source_suites.py {bundle|gate3|gate4|bench}
Test imports run without external services; no firmware/USB operations.
"""
from pathlib import Path
import os
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]
SUITES={
    "bundle":("scripts/operator_bundle_guard","test_*.py"),
    "gate3":("scripts/gate3_operator","test_*.py"),
    "gate4":("scripts/gate4_entry","test_*.py"),
    "bench":("scripts/bench","test_*.py"),
}
def main():
    if len(sys.argv)!=2 or sys.argv[1] not in SUITES:
        print("usage: validate_source_suites.py bundle|gate3|gate4|bench",file=sys.stderr)
        return 2
    root,pattern=SUITES[sys.argv[1]]
    env=os.environ.copy()
    env["PYTHONDONTWRITEBYTECODE"]="1"
    command=[sys.executable,"-B","-m","unittest","discover","-s",root,"-p",pattern,"-v"]
    result=subprocess.run(command,cwd=ROOT,env=env,timeout=42)
    return result.returncode
if __name__=="__main__":
    sys.exit(main())
