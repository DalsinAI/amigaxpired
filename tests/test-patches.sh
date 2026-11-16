#!/bin/sh
# Host test for patches/: the X-pired source xpired.lock pins, the patches
# applied, what each one is for, PathJoin on its own, and the whole game
# built on the host against SDL 2 with compat/ (the same layer the Amiga
# build uses, with a stand-in font).
#   XP_TGZ=<the tarball> sh test-patches.sh   (else it is fetched)
# Copyright (c) 2026 AmiCradle Team. GPL 2 or later, like X-pired.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
. "$HERE/../xpired.lock"
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
fail=0
no() { echo "FAIL: $1"; fail=1; }

TGZ=${XP_TGZ:-$T/xpired.tar.gz}
[ -f "$TGZ" ] || curl -fsSL -o "$TGZ" "$XP_URL"
[ "$(sha256sum "$TGZ" | cut -c1-64)" = "$XP_SHA256" ] || { echo "FAIL: source SHA-256"; exit 1; }
tar xzf "$TGZ" -C "$T"
S="$T/src"
for p in "$HERE"/../patches/*.patch; do
    patch -s -d "$S" -p1 < "$p" || no "patch does not apply: $(basename "$p")"
done
C="$S/xpired.c"

# 0001: every data path relative to the game's drawer, which is made the
# current directory; nothing joins "PROGDIR:" and a name with a '/'
grep -q '"\./img/\|"\./snd/' "$C" && no '"./" data paths left (AmigaDOS has no ".")'
grep -q '"%s/xpired\|"%s/img' "$C" && no 'a "%s/" path join left'
grep -q 'char \*SHARE_DIR="PROGDIR:";' "$C" || no 'SHARE_DIR is not PROGDIR: on AmigaOS'
grep -q 'defined(UNIX) || defined(__amigaos__)' "$C" || no 'chdir(SHARE_DIR) not on AmigaOS'
grep -q 'strcpy(GAME_HOME,SHARE_DIR);' "$C" || no 'GAME_HOME is not SHARE_DIR off UNIX'
grep -q 'IMG_Load(strncmp(T,"./",2)?T:T+2)' "$C" || no 'level backgrounds keep "./"'
[ "$(grep -c 'PathJoin(' "$C")" = 8 ] || no "PathJoin: $(grep -c 'PathJoin(' "$C") uses, not 8 (one definition, seven calls)"
# 0002: SDL 2's typed character and the music looping in SDL2_mixer
grep -q 'UC=sdl12on2_char(&event->key.keysym);' "$C" || no 'keysym.unicode not replaced for SDL 2'
grep -q 'Mix_FadeInMusic(Music,-1,3000);' "$C" || no 'music does not loop by itself on SDL 2'

# PathJoin, taken out of the patched source
sed -n '/^static char \*PathJoin/,/^}/p' "$C" > "$T/pj.h"
cat > "$T/pj.c" <<'EOF'
#include <stdio.h>
#include <string.h>
#include "pj.h"
static int bad;
static void t(const char *d, const char *n, const char *want)
{
    char b[256];
    if (strcmp(PathJoin(b, d, n), want)) { printf("FAIL: PathJoin(%s, %s) = %s\n", d, n, b); bad = 1; }
}
int main(void)
{
    t("PROGDIR:", "xpired.cfg", "PROGDIR:xpired.cfg");
    t("PROGDIR:", "img/icon.bmp", "PROGDIR:img/icon.bmp");
    t("SYS:Games/Xpired", "xpired.lvl", "SYS:Games/Xpired/xpired.lvl");
    t("/usr/local/share/xpired", "xpired.lvl", "/usr/local/share/xpired/xpired.lvl");
    t("/home/a/.xpired/", "xpired.dmo", "/home/a/.xpired/xpired.dmo");
    t(".", "xpired.cfg", "./xpired.cfg");
    return bad;
}
EOF
cc -Wall -o "$T/pj" "$T/pj.c" && "$T/pj" || no "PathJoin"

# the whole game on the host: SDL 2 + compat/, as the Amiga build has it
if pkg-config --exists sdl2 SDL2_image SDL2_mixer 2>/dev/null; then
    printf '#include "SDL_gfxPrimitives.h"\nstatic unsigned char f[2048];\nconst unsigned char *xp_font8(void) { return f; }\n' > "$T/font.c"
    # shellcheck disable=SC2046
    cc -w -I"$HERE/../compat/include" $(pkg-config --cflags sdl2) -o "$T/xpired" \
        "$C" "$HERE/../compat/sdl12on2.c" "$HERE/../compat/gfxprim.c" "$T/font.c" \
        $(pkg-config --libs sdl2 SDL2_image SDL2_mixer) -lm || no "the game does not build on SDL 2 with compat/"
    # and it starts: loads its sprites, sounds and 31 levels and sits on the
    # title page, with no window or sound card (killed after 4 s)
    if [ -x "$T/xpired" ]; then
        rc=0
        (cd "$S" && SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy timeout 4 "$T/xpired" > "$T/run.log" 2>&1) || rc=$?
        [ "$rc" = 124 ] || { no "the game ended by itself (exit $rc)"; cat "$T/run.log"; }
        grep -q "Error loading level\|Can't allocate" "$T/run.log" && { no "the game could not load"; cat "$T/run.log"; }
    fi
else
    echo "(no host SDL2, SDL2_image and SDL2_mixer: the host build of the game is skipped)"
fi

[ $fail = 0 ] && echo "patches: all checks passed"
exit $fail
