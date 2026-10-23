#!/usr/bin/env python3
"""DEUS OS explicit-locator artifact manifest checker. Never searches disk or signs/opens devices."""
from __future__ import annotations
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import stat
import sys

SCHEMA = 'DEUS_OS_RECOVERY_ARTIFACT_REGISTRY_V1'
HEX64 = re.compile(r'[A-Fa-f0-9]{64}\Z')
IDENT = re.compile(r'[a-z][a-z0-9_]{2,79}\Z')
ALIAS = re.compile(r'[A-Z][A-Z0-9_]{2,59}\Z')
KINDS = {'SIGNED_APPLICATION_PACKAGE', 'UNSIGNED_APPLICATION_BIN', 'OPERATOR_BUNDLE', 'RECOVERY_PACKAGE'}
STATES = {'HISTORICAL_REFERENCE_ONLY', 'NOT_FOUND_IN_BOUNDED_SCAN', 'VERIFIED_CUSTODY', 'NOT_YET_PRODUCED'}
MAX_ENTRIES = 150
MAX_LOCATORS = 10
MAX_FILE = 16 * 1024 * 1024

class Refuse(ValueError):
    pass

def require(ok, code):
    if not ok:
        raise Refuse(code)

def parse_json(path):
    data = path.read_bytes()
    require(len(data) <= 256_000, 'REGISTRY_OVERSIZE')
    def no_duplicate(pairs):
        d = {}
        for k, v in pairs:
            require(k not in d, 'DUPLICATE_JSON_KEY')
            d[k] = v
        return d
    return json.loads(data.decode('utf-8', 'strict'), object_pairs_hook=no_duplicate)

def valid_relative(value):
    require(isinstance(value, str) and 1 <= len(value) <= 180, 'BAD_RELATIVE_FILENAME')
    require(not any(c in value for c in ('\\', ':', '\x00', '\n', '\r')), 'RELATIVE_PATH_FORBIDDEN_CHAR')
    p = PurePosixPath(value)
    require(not p.is_absolute() and all(part not in ('.', '..', '') for part in value.split('/')), 'PATH_TRAVERSAL')
    require(not any(part.startswith('.') for part in value.split('/')), 'HIDDEN_PATH_COMPONENT')
    return value

def validate_registry(reg):
    require(type(reg) is dict and reg.get('schema') == SCHEMA, 'REGISTRY_SCHEMA')
    require(type(reg.get('entries')) is list and len(reg['entries']) <= MAX_ENTRIES, 'ENTRY_COUNT')
    ids = set()
    for item in reg['entries']:
        require(type(item) is dict and isinstance(item.get('id'), str) and IDENT.fullmatch(item['id']), 'ENTRY_ID')
        require(item['id'] not in ids, 'DUPLICATE_ENTRY_ID')
        ids.add(item['id'])
        require(item.get('kind') in KINDS, 'ENTRY_KIND')
        require(item.get('custody') in STATES, 'ENTRY_CUSTODY')
        digest = item.get('sha256')
        require(digest is None or isinstance(digest, str) and bool(HEX64.fullmatch(digest)), 'ENTRY_DIGEST')
        size = item.get('bytes')
        require(size is None or type(size) is int and 0 < size <= MAX_FILE, 'ENTRY_SIZE')
        require((digest is None) == (size is None), 'INCOMPLETE_IDENTITY')
        require(item.get('custody') != 'VERIFIED_CUSTODY' or digest is not None, 'VERIFIED_WITHOUT_DIGEST')
        locators = item.get('locators', [])
        require(type(locators) is list and len(locators) <= MAX_LOCATORS, 'LOCATOR_COUNT')
        locator_keys = set()
        for loc in locators:
            require(type(loc) is dict and isinstance(loc.get('alias'), str) and ALIAS.fullmatch(loc['alias']), 'LOCATOR_ALIAS')
            valid_relative(loc.get('relative'))
            key = (loc['alias'], loc['relative'].casefold())
            require(key not in locator_keys, 'DUPLICATE_LOCATOR')
            locator_keys.add(key)
        require(item.get('custody') != 'VERIFIED_CUSTODY' or len(locators) > 0, 'VERIFIED_WITHOUT_LOCATOR')
        # This is a *file* identity registry, never a key/DPAPI/escrow locator.
        for loc in locators:
            suffix = PurePosixPath(loc['relative']).suffix.lower()
            require(suffix in ('.pkg', '.bin', '.zip'), 'SECRET_OR_UNSUPPORTED_FILE_TYPE')
    return len(ids)

def root_map(values):
    out = {}
    for value in values:
        require('=' in value, 'ROOT_ALIAS_FORMAT')
        alias, path = value.split('=', 1)
        require(ALIAS.fullmatch(alias) and alias not in out, 'ROOT_ALIAS_DUPLICATE_OR_INVALID')
        p = Path(path).expanduser()
        require(p.is_absolute(), 'ROOT_MUST_BE_ABSOLUTE')
        require(not p.drive.startswith('\\\\'), 'NETWORK_ROOT_FORBIDDEN')
        require(p != Path(p.anchor), 'ROOT_OF_DRIVE_FORBIDDEN')
        require(p.exists() and p.is_dir() and not p.is_symlink() and not getattr(p, 'is_junction', lambda: False)(), 'ROOT_NOT_REAL_DIRECTORY')
        out[alias] = p.resolve(strict=True)
    return out

def digest_file(path):
    h = hashlib.sha256()
    with path.open('rb') as f:
        while True:
            part = f.read(64 * 1024)
            if not part:
                break
            h.update(part)
    return h.hexdigest().upper()

def inspect_item(item, roots):
    attempts = []
    matched = False
    mismatch = False
    for loc in item.get('locators', []):
        a = loc['alias']
        label = loc['relative']
        record = {'alias': a, 'relative': label}
        if a not in roots:
            record['status'] = 'ROOT_NOT_MAPPED'
        else:
            root = roots[a]
            cur = root
            parts = label.split('/')
            try:
                for part in parts:
                    cur = cur / part
                    if cur.is_symlink() or getattr(cur, 'is_junction', lambda: False)():
                        raise Refuse('SYMLINK_OR_JUNCTION_BLOCKED')
                target = cur.resolve(strict=False)
                require(root == target or root in target.parents, 'PATH_OUTSIDE_ROOT')
                if not target.exists():
                    record['status'] = 'NOT_FOUND'
                else:
                    mode = target.stat().st_mode
                    require(stat.S_ISREG(mode), 'NOT_REGULAR_FILE')
                    size = target.stat().st_size
                    require(size <= MAX_FILE, 'FILE_SIZE_BOUND_EXCEEDED')
                    if item['bytes'] is None or item['sha256'] is None:
                        record['status'] = 'PRESENT_UNPINNED_NOT_ACCEPTED'
                    elif size != item['bytes']:
                        record['status'] = 'SIZE_MISMATCH'
                        mismatch = True
                    elif digest_file(target) == item['sha256'].upper():
                        record['status'] = 'SHA256_MATCH'
                        matched = True
                    else:
                        record['status'] = 'SHA256_MISMATCH'
                        mismatch = True
            except (OSError, Refuse) as exc:
                record['status'] = 'ACCESS_OR_POLICY_REJECTED'
                # Do not leak an actual host path or exception containing local secrets.
                record['reason'] = exc.args[0] if isinstance(exc, Refuse) else 'FILESYSTEM_ERROR'
        attempts.append(record)
    status = 'EXACT_MATCH' if matched else ('MISMATCH' if mismatch else 'NOT_VERIFIED')
    return {'id': item['id'], 'status': status, 'locators': attempts}

def report(reg, roots):
    results = [inspect_item(item, roots) for item in reg['entries']]
    return {
        'schema': 'DEUS_OS_ARTIFACT_LEDGER_CHECK_RESULT_V1',
        'qualification': 'OFFLINE_EXPLICIT_FILE_BYTES_ONLY_NOT_RECOVERY_READY',
        'results': results,
        'exact_matches': sum(r['status'] == 'EXACT_MATCH' for r in results),
        'unverified': sum(r['status'] == 'NOT_VERIFIED' for r in results),
        'mismatches': sum(r['status'] == 'MISMATCH' for r in results),
        'discovered_automatically': False,
        'signing_tested': False,
        'device_access': False,
        'gate3_accepted': False,
        'gate4_authorized': False,
    }

def main(argv=None):
    p = argparse.ArgumentParser(description='DEUS OS exact-locator artifact checker; no recursive searches')
    p.add_argument('--registry', type=Path, required=True)
    p.add_argument('--root', action='append', default=[], metavar='ALIAS=ABSOLUTE_DIRECTORY')
    args = p.parse_args(argv)
    try:
        reg = parse_json(args.registry)
        n = validate_registry(reg)
        roots = root_map(args.root)
        result = report(reg, roots)
        result['registry_entries'] = n
        print(json.dumps(result, indent=2, sort_keys=True, ensure_ascii=False))
        return 2 if result['mismatches'] else 0
    except (OSError, UnicodeError, json.JSONDecodeError, Refuse) as exc:
        label = str(exc) if isinstance(exc, Refuse) else exc.__class__.__name__
        print(json.dumps({'result':'FAIL', 'classification':'ARTIFACT_LEDGER_INVALID', 'reason':label}), file=sys.stderr)
        return 3

if __name__ == '__main__':
    raise SystemExit(main())
