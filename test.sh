#!/usr/bin/env bash
# Build the decoder and check that 0x40490FDB decodes to ~3.14159.
set -euo pipefail
cd "$(dirname "$0")"
CC="${CC:-gcc}"
bin="$(mktemp -u).h2f"
"$CC" -O2 -o "$bin" hex-2-float.c -lm
out=$(echo 0x40490FDB | "$bin")
rm -f "$bin"
if echo "$out" | grep -q "3.14159"; then
    echo "PASS: 0x40490FDB decodes to ~3.14159"
    exit 0
else
    echo "FAIL: did not decode 0x40490FDB to ~3.14159"
    echo "$out"
    exit 1
fi
