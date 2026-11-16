#!/bin/sh
# Writes the Part file GFOpenUpTool INSTALL reads (gfopenup src/openuptool.c):
# one "copy" line per file under the part drawer, each to SYS:<its path>.
#   write-part.sh <part drawer> <version>
# Copyright (c) 2026 AmiCradle Team. GPL 2 or later, like X-pired.
set -eu
P=$1; V=$2
{
    echo "part Xpired $V"
    echo "; X-pired: push crates, blow up walls and reach the exit before time runs out"
    echo "needs 68040"
    echo "needs part GFOpenGPU"
    (cd "$P" && find Games -type f ! -name Part | LC_ALL=C sort) | while read -r f; do
        echo "copy \"$f\" \"SYS:$f\""
    done
} > "$P/Part"
