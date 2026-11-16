#!/bin/sh
# Host test for scripts/write-part.sh: the Part file GFOpenUpTool reads.
# Copyright (c) 2026 AmiCradle Team. GPL 2 or later, like X-pired.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
mkdir -p "$T/Games/Xpired/snd"
: > "$T/Games/Xpired/Xpired"; : > "$T/Games/Xpired.info"; : > "$T/Games/Xpired/snd/beep.wav"
sh "$HERE/../scripts/write-part.sh" "$T" 1.22
fail=0
expect() { grep -qxF "$1" "$T/Part" || { echo "FAIL: no line: $1"; fail=1; }; }
expect 'part Xpired 1.22'
expect 'needs 68040'
expect 'needs part GFOpenGPU'
expect 'copy "Games/Xpired.info" "SYS:Games/Xpired.info"'
expect 'copy "Games/Xpired/Xpired" "SYS:Games/Xpired/Xpired"'
expect 'copy "Games/Xpired/snd/beep.wav" "SYS:Games/Xpired/snd/beep.wav"'
[ "$(head -1 "$T/Part")" = 'part Xpired 1.22' ] || { echo "FAIL: first line"; fail=1; }
[ "$(grep -c '^copy' "$T/Part")" = 3 ] || { echo "FAIL: copy count (the Part file itself must not be listed)"; fail=1; }
sh "$HERE/../scripts/write-part.sh" "$T" 1.22   # a second run: same file
[ "$(grep -c '^copy' "$T/Part")" = 3 ] || { echo "FAIL: second run lists the Part file"; fail=1; }
[ $fail = 0 ] && echo "Part file: all checks passed"
exit $fail
