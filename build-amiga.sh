#!/bin/sh
# Builds X-pired for GFOS (AmigaOS 3.2.3 with GFOpenUp, on the GFOpenKick
# ROM or a Kickstart) with a stove: 68040 with its FPU (GFOpenGPU's SDL 2 kit
# is hard float), on the stove's SDL 2, SDL2_image and SDL2_mixer. X-pired
# is written for SDL 1.2 and SDL_gfx: compat/ gives it the 1.2 calls it
# makes on SDL 2, and its four drawing calls on SDL 2 surfaces with the
# system's topaz 8 for text.
#   STOVE=.../stoves/os32-gcc16-open6 OUT=out ./build-amiga.sh
# X-pired is fetched as xpired.lock gives it (URL and SHA-256), with
# patches/ on it; nothing third-party is kept here.
# OUT gets Parts/Xpired: a GFOpenUp part (its Part file, the Xpired drawer
# for SYS:Games and its icons; it needs the GFOpenGPU part), installed with
#   GFOpenUpTool INSTALL Parts/Xpired
# and unpacked/Games, the same drawer for a lab copy.
# Copyright (c) 2026 AmiCradle Team. GPL 2 or later, like X-pired.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
. "$HERE/xpired.lock"
STOVE=${STOVE:-"$HOME/AmiCradle/kitchen/stoves/os32-gcc16-open6"}
if [ -x "$STOVE/prefix/bin/m68k-amigaos-gcc" ]; then TC="$STOVE/prefix"; else TC="$STOVE"; fi
# outside the checkout: Cradle cleans it (git clean -ffdx) before every job
WORK="${WORK:-$HOME/.cache/amigaxpired/build}"; OUT="${OUT:-$WORK/out}"
PATH="$TC/bin:$PATH"; export PATH
mkdir -p "$WORK" "$OUT"; OUT=$(cd "$OUT" && pwd)
t0=$(date +%s)

# --- X-pired as xpired.lock pins it, with patches/ ---------------------------------
TGZ="$WORK/xpired-$XP_VERSION.tar.gz"
if [ "$(sha256sum "$TGZ" 2>/dev/null | cut -c1-64)" != "$XP_SHA256" ]; then
    curl -fsSL -o "$TGZ.part" "$XP_URL"
    [ "$(sha256sum "$TGZ.part" | cut -c1-64)" = "$XP_SHA256" ] || { echo "xpired source: SHA-256 mismatch"; exit 1; }
    mv "$TGZ.part" "$TGZ"
fi
SRC="$WORK/xpired/src"
PSUM=$(cat "$HERE"/patches/*.patch | sha256sum | cut -c1-16)
if [ "$(cat "$WORK/xpired/.patched" 2>/dev/null)" != "$XP_SHA256-$PSUM" ]; then
    rm -rf "$WORK/xpired"; mkdir -p "$WORK/xpired"
    tar xzf "$TGZ" -C "$WORK/xpired"
    for p in "$HERE"/patches/*.patch; do
        patch -s -d "$SRC" -p1 < "$p" || { echo "patch failed: $p"; exit 1; }
    done
    echo "$XP_SHA256-$PSUM" > "$WORK/xpired/.patched"
fi

# --- SDL 2 from the stove's SDK -------------------------------------------------------
SDK="$TC/m68k-amigaos"
for f in include/SDL2/SDL.h include/SDL2/SDL_image.h include/SDL2/SDL_mixer.h lib/libSDL2.a lib/libSDL2_image.a lib/libSDL2_mixer.a; do
    [ -f "$SDK/$f" ] || { echo "the stove has no $f"; exit 2; }
done

# --- compile ----------------------------------------------------------------------
# -fno-tree-loop-distribute-patterns: GCC 16's loop miscompile (the kit's
# sdl2-config gives it too). Only the game: the level editor (xpiredit.c)
# is not part of the port.
CF="-noixemul -m68040 -m68881 -O2 -fomit-frame-pointer -fno-tree-loop-distribute-patterns"
INC="-I$HERE/compat/include -I$SDK/include/SDL2"
O="$WORK/obj"; rm -rf "$O"; mkdir -p "$O"
m68k-amigaos-gcc $CF -w $INC -c "$SRC/xpired.c" -o "$O/xpired.o"
for f in sdl12on2 gfxprim font8_amiga; do
    m68k-amigaos-gcc $CF -Wall $INC -c "$HERE/compat/$f.c" -o "$O/$f.o"
done
m68k-amigaos-gcc $CF -c "$HERE/amiga/amiga_start.c" -o "$O/amiga_start.o"
echo "compile: $(( $(date +%s) - t0 )) s"

# --- the link ---------------------------------------------------------------------
m68k-amigaos-gcc -noixemul -m68040 -m68881 -o "$O/Xpired.debug" \
    "$O/xpired.o" "$O/sdl12on2.o" "$O/gfxprim.o" "$O/font8_amiga.o" "$O/amiga_start.o" \
    -L"$SDK/lib" -lSDL2_image -lSDL2_mixer -lSDL2 -lm -lpthread -latomic > "$O/link.log" 2>&1 || { head -60 "$O/link.log"; exit 1; }

# --- the GFOpenUp part: Parts/Xpired ------------------------------------------------
P="$OUT/Parts/Xpired"; rm -rf "$P"; D="$P/Games/Xpired"; mkdir -p "$D"
m68k-amigaos-strip -o "$D/Xpired" "$O/Xpired.debug"
cp -R "$SRC/img" "$SRC/snd" "$D/"
cp "$SRC/xpired.lvl" "$SRC/xpired.cfg" "$SRC/xpired.dmo" "$D/"
cp "$SRC/readme.txt" "$D/X-pired.txt"
cp "$HERE/README.md" "$D/ReadMe"
cp "$HERE/LICENSE" "$D/COPYING.txt"
I="$HERE/amiga/icons"
cp "$I/Xpired.info" "$D/Xpired.info"
for f in ReadMe X-pired.txt COPYING.txt; do cp "$I/ReadMe.info" "$D/$f.info"; done
cp "$I/Xpired-drawer.info" "$P/Games/Xpired.info"
sh "$HERE/scripts/write-part.sh" "$P" "$XP_VERSION"
rm -rf "$OUT/unpacked"; mkdir -p "$OUT/unpacked"; cp -R "$P/Games" "$OUT/unpacked/"
ls -l "$D/Xpired"
echo "part: $(grep -c '^copy' "$P/Part") files, $(du -sk "$P" | cut -f1) KB"
echo "total: $(( $(date +%s) - t0 )) s"
