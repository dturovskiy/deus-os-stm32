#!/usr/bin/env python3
"""Read-only workspace layout and Git hygiene inventory for Deus OS.

Traverses names and sizes, not protected recovery file contents. No changes,
no USB, no network, no git mutations. Print a JSON inventory for review.
"""
from __future__ import annotations
import collections
import json
import os
from pathlib import Path
import re
import subprocess
import sys

REPO=Path(__file__).resolve().parents[1]
WORKSPACE=REPO.parent
SIBLING_ROLES={"Archive":"off-Git historical source and quarantined operator archive","Bundle":"operator input/staging","Evidence":"original execution evidence",
               "ProtectedRecovery":"signed images and protected recovery",
               "Reports":"off-Git audits/decisions","Tools":"vendor tool installations",
               "Logs":"operator local logs","HostDependencies":"host runtime dependencies",
               "Wheelhouse":"offline external dependencies","private_docs_archive":"private history"}
SKIP_CONTENT={"ProtectedRecovery","private_docs_archive","HostDependencies","Wheelhouse","Tools"}
SAFE_TRAP=re.compile(r"(?i)(^|/)(?:\.env(?:\..*)?|id_(?:rsa|ecdsa|ed25519)|.*\.(?:pkg|pem|key|pfx|p12|dpapi|zip|7z|tar|gz|bin|elf|hex|o|obj|log)|credentials\.json|secrets\.json|.*\.evidence\.zip)$")
MDLINK=re.compile(r"(?<!!)\[[^\]\n]*\]\(([^)\n]+)\)")
PLAN=re.compile(r"^(.*)(_ACCEPTANCE_PLAN|_PLAN)\.md$")
def git(*args):
 cp=subprocess.run(["git",*args],cwd=REPO,stdout=subprocess.PIPE,stderr=subprocess.PIPE,timeout=45)
 if cp.returncode:raise RuntimeError("GIT_FAIL "+repr(args)+" "+cp.stderr.decode(errors="replace")[-350:])
 return cp.stdout.decode("utf8","replace")
def files_beneath(base,excluded_dirs=()):
 count=0;total=0;size_top=[];symlink=[];ext=collections.Counter();topdir=collections.Counter()
 excluded=set(excluded_dirs)|{'.git','__pycache__','bin','obj','.vs','build','node_modules','.venv','venv','dist','target'}
 for d,dirs,files in os.walk(base,followlinks=False):
  if count>30000:break
  parent=Path(d)
  for x in list(dirs):
   p=parent/x
   if p.is_symlink():
    symlink.append(str(p.relative_to(base)));dirs.remove(x)
   elif x in excluded:dirs.remove(x)
  for name in files:
   p=parent/name
   if p.is_symlink():symlink.append(str(p.relative_to(base)));continue
   try:sz=p.stat().st_size
   except OSError:continue
   count+=1;total+=sz
   rel=p.relative_to(base)
   topdir[rel.parts[0]]+=1
   ext[p.suffix.lower() or "<none>"]+=1
   if sz>=500000:size_top.append((sz,str(rel)))
 return {"files":count,"bytes":total,"top_dir_count":topdir.most_common(15),
         "extensions":ext.most_common(15),"large_files":sorted(size_top,reverse=True)[:12],
         "symlink_count":len(symlink),"symlink_sample":symlink[:15]}
def main():
 tracked=git("ls-files","--cached").splitlines()
 untracked=git("ls-files","--others","--exclude-standard").splitlines()
 ignored=[] # bounded: generated SDK/toolchain trees are checked by targeted ignore probes
 status=git("status","--short","--untracked-files=all").splitlines()
 branch=git("branch","--show-current").strip()
 head=git("rev-parse","HEAD").strip()
 duplicates=[p for p in tracked if not (REPO/p).exists()]
 top_track=collections.Counter(p.split("/")[0] for p in tracked)
 top_untracked=collections.Counter(p.split("/")[0] for p in untracked)
 top_ignored=collections.Counter(p.split("/")[0] for p in ignored)
 risks=[p for p in tracked if SAFE_TRAP.search(p) or any(seg in ("bin","obj","build","Evidence","Bundle","ProtectedRecovery","Reports") for seg in p.split("/"))]
 big=[(int((REPO/p).stat().st_size),p) for p in tracked if (REPO/p).is_file() and (REPO/p).stat().st_size>100000]
 gitlinks=collections.defaultdict(list);broken=[]
 docpaths=sorted((REPO/"docs").glob("*.md"))
 for md in [REPO/"README.md",REPO/"CHANGELOG.md",*docpaths]:
  if not md.is_file():continue
  content=md.read_text("utf-8",errors="replace")
  for hit in MDLINK.findall(content):
   t=hit.strip().split(" ",1)[0].strip("<>").split("#",1)[0]
   if not t or t.startswith(("http:","https:","mailto:","data:","#")) or "{{" in t:continue
   if "%" in t:continue
   target=(md.parent/t).resolve()
   if not target.exists():
    if len(broken)<100:broken.append((str(md.relative_to(REPO)),t))
   else:gitlinks[str(md.relative_to(REPO))].append(t)
 plan_ids=set();accept_ids=set()
 for d in docpaths:
  if d.name.endswith('_ACCEPTANCE_PLAN.md'):
   accept_ids.add(d.name[:-len('_ACCEPTANCE_PLAN.md')])
  elif d.name.endswith('_PLAN.md'):
   plan_ids.add(d.name[:-len('_PLAN.md')])
 names={}
 for sibling in sorted(WORKSPACE.iterdir()):
  if sibling.name=="OS":continue
  if sibling.is_dir():
   inv=(files_beneath(sibling,excluded_dirs={"__pycache__",".git"}) if sibling.name not in ('Tools','HostDependencies','Wheelhouse','private_docs_archive','ProtectedRecovery') else {'scan':'bounded_top_level_only','files_in_recursive_tree':'NOT_ENUMERATED'})
   immediate=sorted((x.name for x in sibling.iterdir()))
   names[sibling.name]={"role":SIBLING_ROLES.get(sibling.name,"UNSPECIFIED"),
                         "inventory":inv,"immediate_count":len(immediate),
                         "immediate_sample":immediate[:27]}
  elif sibling.is_file():
   names[sibling.name]={"role":"UNCLASSIFIED_WORKSPACE_ROOT_FILE","bytes":sibling.stat().st_size}
 repo_inv=files_beneath(REPO,excluded_dirs={".git","__pycache__"})
 docs_counts=collections.Counter()
 for path in docpaths:
  n=path.name
  if n.endswith("_ACCEPTANCE_PLAN.md"):docs_counts["acceptance_plans"]+=1
  elif n.endswith("_PLAN.md"):docs_counts["plans"]+=1
  elif "AUDIT" in n or "EVIDENCE" in n:docs_counts["history_and_audit"]+=1
  else:docs_counts["other"]+=1
 sample_untracked={d:[p for p in untracked if p.startswith(d+"/")][:16] for d in top_untracked}
 ignore_probes=["build/os.bin","firmware.elf","package.pkg","x.pem",".env","Evidence/receipt.evidence.zip",
                "Bundle/operator.zip","host/src/DeusOs.Control.Cli/bin/Release/net10.0/deus-cp.dll",
                "host/src/DeusOs.Control.Cli/obj/project.assets.json",
                "scripts/gate3_operator/source_candidate_v2/bin/Debug/net10.0/helper.dll",
                "scripts/bench/__pycache__/foo.pyc","Reports/secret.txt","Notes.tmp"]
 ign=subprocess.run(["git","check-ignore","--no-index","-v",*ignore_probes],cwd=REPO,stdout=subprocess.PIPE,stderr=subprocess.PIPE,timeout=15)
 ign_lines=ign.stdout.decode(errors="replace").splitlines()
 report_index_file=WORKSPACE/"Reports/INDEX.md"
 report_links=[]
 report_broken=[]
 report_index_text=report_index_file.read_text("utf-8",errors="replace") if report_index_file.is_file() else ""
 if report_index_text:
  for dest in MDLINK.findall(report_index_text):
   rel=dest.split("#",1)[0].strip()
   if not rel or rel.startswith(("http:","https:","#")):continue
   report_links.append(rel)
   if not (report_index_file.parent/rel).exists():report_broken.append(rel)
 report_md=[p.relative_to(WORKSPACE/"Reports").as_posix() for p in (WORKSPACE/"Reports").rglob("*.md") if p.name not in ("INDEX.md","README.md")]
 report_unindexed=[p for p in report_md if p not in report_index_text]
    # Current worktree content screen only. No matched values are printed.
 secret_signatures={
  "private_key_header":re.compile(rb"-----BEGIN (?:RSA |EC |OPENSSH |DSA )?PRIVATE KEY-----"),
  "github_pat":re.compile(rb"(?:gh[pousr]_[A-Za-z0-9]{20,}|github_pat_[A-Za-z0-9_]{20,})"),
  "aws_access_key":re.compile(rb"\b(?:AKIA|ASIA)[A-Z0-9]{16}\b"),
  "openai_style_key":re.compile(rb"\bsk-(?:proj-|svcacct-)?[A-Za-z0-9_-]{20,}"),
 }
 secret_hits=[]
 inspected=0
 for name in sorted(set(tracked+untracked)):
  f=REPO/name
  if not f.is_file() or f.is_symlink() or f.stat().st_size>2_000_000:continue
  data=f.read_bytes()
  inspected+=1
  hits=[label for label,rx in secret_signatures.items() if rx.search(data)]
  if hits:secret_hits.append({"file":name,"signature_classes":hits})
 summary={
  "repo":str(REPO),"workspace":str(WORKSPACE),"branch":branch,"head":head,
  "git":{"tracked":len(tracked),"untracked":len(untracked),"ignored":"NOT_FULLY_ENUMERATED_BOUNDED_AUDIT",
         "dirty_status_entries":len(status),
         "tracked_top":top_track.most_common(),
         "untracked_top":top_untracked.most_common(),
         "ignored_top":top_ignored.most_common(),
         "tracked_risky_names":risks,"tracked_missing_files":duplicates,
         "tracked_large_files":sorted(big,reverse=True)[:20],
         "untracked_examples":sample_untracked,"ignore_probe_results":ign_lines},
  "repository_filesystem":repo_inv,
  "current_worktree_secret_signature_screen":{"files_inspected":inspected,"findings":secret_hits,"scope":"known signature shapes only; no values printed"},
  "off_git_siblings":names,
  "reports_index":{"links_count":len(report_links),"broken_links":report_broken[:30],"unindexed_md":report_unindexed[:50],"unindexed_md_count":len(report_unindexed)},
  "docs":{"count":len(docpaths),"categories":dict(docs_counts),
          "design_plan_count":len(plan_ids),"acceptance_plan_count":len(accept_ids),
          "missing_acceptance_pairs":sorted(plan_ids-accept_ids),
          "missing_design_pairs":sorted(accept_ids-plan_ids),
          "markdown_broken_relative_links_sample":broken,
          "markdown_broken_relative_link_sample_count":len(broken)}
 }
 print(json.dumps(summary,ensure_ascii=False,indent=2))
if __name__=="__main__":
 try:main()
 except Exception as ex:
  print(json.dumps({"audit":"FAIL","exception":repr(ex)},ensure_ascii=False));sys.exit(1)
