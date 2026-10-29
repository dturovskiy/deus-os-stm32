#!/usr/bin/env python3
"""Narrow portable PowerShell lexical delimiter audit for A-05 v1 AST defect.

NOT a substitute for real PowerShell 7 Parser.ParseFile. Detects this exact
unbalanced source expression, checks block and paren pairing ignoring comments
and quoted text. No execution, downloads, device or host changes.
"""
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[3]
V1=ROOT/"Bundle/_stage_stm32_os_gate4_a05_cp314_isolated_deploy_v1_20261028/apply.ps1"
V2=ROOT/"Bundle/_stage_stm32_os_gate4_a05_cp314_isolated_deploy_v2_20261029/apply.ps1"

def delimiters(source):
    stack=[]
    mode="NORMAL"
    line=1
    col=0
    pairs={")":"(","]":"[","}":"{"}
    error=None
    i=0
    while i<len(source):
        c=source[i]
        if c=="\n":line+=1;col=0
        else:col+=1
        if mode=="COMMENT":
            if c=="\n":mode="NORMAL"
        elif mode=="SINGLE":
            if c=="'":
                if i+1<len(source) and source[i+1]=="'":
                    i+=1;col+=1
                else:mode="NORMAL"
        elif mode=="DOUBLE":
            if c==chr(96) and i+1<len(source):
                i+=1;col+=1
            elif c=='"':
                mode="NORMAL"
        else:
            if c=="#":mode="COMMENT"
            elif c=="'":mode="SINGLE"
            elif c=='"':mode="DOUBLE"
            elif c==chr(96) and i+1<len(source):i+=1;col+=1
            elif c in "([{":stack.append((c,line,col))
            elif c in ")]}":
                if not stack or stack[-1][0]!=pairs[c]:
                    error={"line":line,"column":col,"unexpected":c,
                           "expected":stack[-1][0] if stack else "NONE",
                           "unclosed_from_line":stack[-1][1] if stack else -1}
                    break
                stack.pop()
        i+=1
    if error:return {"balanced":False,"reason":"MISMATCH","error":error}
    if stack:return {"balanced":False,"reason":"UNCLOSED","error":{
        "open":stack[-1][0],"line":stack[-1][1],"column":stack[-1][2]}}
    if mode in ("SINGLE","DOUBLE"):return {"balanced":False,"reason":"OPEN_QUOTE"}
    return {"balanced":True,"reason":"NONE","error":None}
def audit():
    old=V1.read_text(encoding="utf-8")
    new=V2.read_text(encoding="utf-8")
    a=delimiters(old);b=delimiters(new)
    if a["balanced"]:raise AssertionError("V1_SHOULD_HAVE_UNBALANCED_DELIMITER")
    if a["error"].get("unclosed_from_line")!=250:
        raise AssertionError("V1_ORIGINAL_AST_AREA_DRIFT:"+str(a))
    if not b["balanced"]:
        raise AssertionError("V2_STILL_HAS_UNBALANCED_DELIMITER:"+str(b))
    assert "AddCheck 'ORIGINAL_DOWNLOAD_EVIDENCE_SHA'" in new
    print(json.dumps({"result":"A05_CP314_POWERSHELL_DELIMITER_REPAIR_SOURCE_PASS",
       "v1_defect":a,"v2_balanced":True,
       "native_powershell_ast":"NOT_YET_EXECUTED",
       "windows_deployment":"NOT_YET_EXECUTED"},sort_keys=True))
if __name__=="__main__":audit()
