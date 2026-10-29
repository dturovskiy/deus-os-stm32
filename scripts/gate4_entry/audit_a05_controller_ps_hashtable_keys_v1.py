#!/usr/bin/env python3
"""Narrow source-only check of duplicate PowerShell [ordered] hash literal keys.

Tracks the exact failure class seen in immutable owner v1 AST Evidence.
A static key/delimiter check is NOT a substitute for native PowerShell AST.
"""
from pathlib import Path
import re
import json
from audit_a05_cp314_pwsh_delimiters_v1 import delimiters

ROOT=Path(__file__).resolve().parents[3]
OLD=ROOT/"OS/scripts/gate4_entry/apply_a05_controller_isolated_deny_v1_20261029.ps1"
NEW=ROOT/"OS/scripts/gate4_entry/apply_a05_controller_isolated_deny_v2_20261029.ps1"
def need(x,msg):
    if not x:raise AssertionError(msg)
def ordered_literal_keys(source,name):
    marker="$"+name+"=[ordered]@{"
    need(source.count(marker)==1,"ORDERED_LITERAL_NOT_UNIQUE_"+name)
    begin=source.index(marker)+len(marker)
    # Known top-level [ordered] literal terminator, located at the same
    # indentation as its own closing brace. Exact for this reviewed source.
    original=source[begin:]
    m=re.search(r"(?m)^(\s{2}|\s{4})\}\s*$",original)
    need(m is not None,"ORDERED_LITERAL_NOT_TERMINATED_"+name)
    body=original[:m.start()]
    rows=re.findall(r"(?:^|[;\n])\s*([A-Za-z_]\w*)\s*=",body)
    need(len(rows)>=8,"LITERAL_TOO_SMALL_TO_VERIFY_"+name)
    duplicate=[]
    seen=set()
    for key in rows:
        canonical=key.casefold()
        if canonical in seen:duplicate.append(key)
        seen.add(canonical)
    return rows,duplicate

def audit():
    v1=OLD.read_text(encoding="utf-8")
    v2=NEW.read_text(encoding="utf-8")
    oldkeys,olddup=ordered_literal_keys(v1,"typed")
    newkeys,newdup=ordered_literal_keys(v2,"typed")
    oldrecord,oldrecorddup=ordered_literal_keys(v1,"record")
    newrecord,newrecorddup=ordered_literal_keys(v2,"record")
    need(olddup==["receipt_sha256"],"V1_EXPECTED_ORIGINAL_DUPLICATE_KEY")
    need(not oldrecorddup and not newdup and not newrecorddup,
         "V2_DUPLICATE_ORDERED_HASH_LITERAL_KEY")
    need("receipt_sha256" in newkeys and
         "expected_receipt_sha256" in newkeys and
         len(newkeys)==len(oldkeys),"V2_REPAIR_DROPPED_OR_DUPLICATED_FACT")
    need(delimiters(v2)["balanced"],"V2_UNBALANCED_PARENS")
    need(v2.count("DEUS_RUN_RESULT_PATH")>=2 and
         v2.count("WriteNew ([IO.Path]::GetFullPath($env:DEUS_RUN_RESULT_PATH))")==1,
         "RESULT_WRITER_DRIFT")
    print(json.dumps({"result":"A05_CONTROLLER_V2_HASH_KEYS_STATIC_PASS",
      "historical_v1_duplicate_keys":olddup,
      "v2_typed_literal_keys":len(newkeys),
      "v2_record_literal_keys":len(newrecord),
      "v2_typed_duplicate_keys":newdup,
      "v2_record_duplicate_keys":newrecorddup,
      "delimiters":"BALANCED",
      "actual_powershell_ast":"NOT_RUN"},sort_keys=True))
if __name__=="__main__":audit()
