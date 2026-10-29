#!/usr/bin/env python3
"""Narrow source-only check of duplicate PowerShell [ordered] hash literal keys.

Tracks the exact failure class seen in immutable owner v1 AST Evidence.
A static key/delimiter check is NOT a substitute for native PowerShell AST.
"""
from pathlib import Path
import hashlib
import re
import json
from audit_a05_cp314_pwsh_delimiters_v1 import delimiters

HERE=Path(__file__).resolve().parent
# Historical v1 was quarantined off Git. Only consult the original if present.
OLD=HERE.parents[2]/"Archive/QuarantinedOperatorSource/Gate4/20261029/apply_a05_controller_isolated_deny_v1_20261029.ps1"
OLD_SHA="76B7B7323DF4865FAC9D70CD6481B31A9B8B8C7F57B21A9ED9DC807DF6F22AED"
NEW=HERE/"apply_a05_controller_isolated_deny_v2_20261029.ps1"
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
    v2=NEW.read_text(encoding="utf-8")
    newkeys,newdup=ordered_literal_keys(v2,"typed")
    newrecord,newrecorddup=ordered_literal_keys(v2,"record")
    need(not newdup and not newrecorddup,
         "V2_DUPLICATE_ORDERED_HASH_LITERAL_KEY")
    need("receipt_sha256" in newkeys and
         "expected_receipt_sha256" in newkeys,
         "V2_REPAIR_DROPPED_OR_DUPLICATED_FACT")
    need(delimiters(v2)["balanced"],"V2_UNBALANCED_PARENS")
    need(v2.count("DEUS_RUN_RESULT_PATH")>=2 and
         v2.count("WriteNew ([IO.Path]::GetFullPath($env:DEUS_RUN_RESULT_PATH))")==1,
         "RESULT_WRITER_DRIFT")
    if OLD.is_file():
        need(not OLD.is_symlink() and
             hashlib.sha256(OLD.read_bytes()).hexdigest().upper()==OLD_SHA,
             "HISTORICAL_V1_ORIGINAL_DRIFT")
        v1=OLD.read_text(encoding="utf-8")
        oldkeys,olddup=ordered_literal_keys(v1,"typed")
        _,oldrecorddup=ordered_literal_keys(v1,"record")
        need(olddup==["receipt_sha256"] and not oldrecorddup and
             len(newkeys)==len(oldkeys),
             "ORIGINAL_V1_DUPLICATE_OR_V2_FACT_COUNT")
        history="PINNED_ORIGINAL_V1_READONLY"
    else:
        # Public source checkout has no quarantined v1; exercise the detector
        # against a synthetic duplicate of the *current* v2, never claim v1 proof.
        token="$typed=[ordered]@{"
        need(v2.count(token)==1,"V2_TYPED_MARKER_MISSING")
        synthetic=v2.replace(token,token+"\n    receipt_sha256='SYNTHETIC';",1)
        synthetic_keys,synthetic_dups=ordered_literal_keys(synthetic,"typed")
        need(synthetic_dups==["receipt_sha256"] and
             len(synthetic_keys)==len(newkeys)+1,
             "SYNTHETIC_DUPLICATE_NOT_DETECTED:"+repr((synthetic_dups,len(synthetic_keys),len(newkeys))))
        olddup=synthetic_dups
        history="HISTORICAL_V1_OFF_GIT_NOT_ATTESTED_SYNTHETIC_NEGATIVE_ONLY"
    print(json.dumps({"result":"A05_CONTROLLER_V2_HASH_KEYS_STATIC_PASS",
      "historical_v1_source":history,
      "historical_v1_duplicate_keys":olddup,
      "v2_typed_literal_keys":len(newkeys),
      "v2_record_literal_keys":len(newrecord),
      "v2_typed_duplicate_keys":newdup,
      "v2_record_duplicate_keys":newrecorddup,
      "delimiters":"BALANCED",
      "actual_powershell_ast":"NOT_RUN"},sort_keys=True))
if __name__=="__main__":audit()
