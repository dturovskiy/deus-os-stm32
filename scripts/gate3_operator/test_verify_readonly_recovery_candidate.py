#!/usr/bin/env python3
"""Synthetic negative checks for source-bound Gate3 metadata verifier. No device IO."""
import importlib.util
from pathlib import Path

p = Path("scripts/gate3_operator/verify_readonly_recovery_candidate.py").resolve()
spec = importlib.util.spec_from_file_location("g3_readonly_verifier", p)
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)
count = 0

def expect_reject(name, value, parameters):
    global count
    try:
        m.check_package(value, **parameters)
    except ValueError:
        count += 1
        print("PASS negative", name)
        return
    raise AssertionError("NEGATIVE_ACCEPTED_" + name)

for name, params in m.PACKAGES.items():
    length, digest, version, image_len, image_sha = params
    raw = m.regular_bytes(m.STORE / name, 60000)
    kw = dict(total=length, digest=digest, version=version,
              image_bytes=image_len, image_sha=image_sha)
    m.check_package(raw, **kw)
    count += 1
    print("PASS original", name)
    modified = bytearray(raw)
    modified[0] ^= 1
    expect_reject("whole-hash", bytes(modified), kw)
    expect_reject("wrong-version", raw, {**kw, "version": version + 1})
    modified = bytearray(raw)
    modified[0] ^= 1
    expect_reject("wrong-product-rehashed", bytes(modified),
                  {**kw, "digest": m.sha256(modified)})
    modified = bytearray(raw)
    modified[12] ^= 1
    expect_reject("tampered-header-rehashed", bytes(modified),
                  {**kw, "digest": m.sha256(modified)})
    modified = bytearray(raw)
    modified[-1] ^= 1
    expect_reject("tampered-payload-rehashed", bytes(modified),
                  {**kw, "digest": m.sha256(modified)})
    expect_reject("truncated-file", raw[:-1], kw)
assert count == 14, count
print("GATE3_READONLY_METADATA_NEGATIVE_TESTS=PASS COUNT=14 LIVE_USB=NO FLASH=NO")
