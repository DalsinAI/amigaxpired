Xpired for GFOS
===============

X-pired 1.22, the action puzzle by Miroslav Kucera and friends (push crates, blow up walls, put out fires and reach the exit before time runs out), for GFOS (AmigaOS 3.2 with GFOpenUp) on a 68040 with GFOpenUp's SDL 2 (the GFOpenGPU part).

Start it from the Xpired icon in SYS:Games/Xpired. The arrow keys move, Ctrl acts, Esc goes back; Alt+S opens the set-up (shades and volumes), Alt+X quits. A level's password, typed on the title page, starts at that level. The settings and the demo record are kept in the game's drawer (xpired.cfg, xpired.dmo).

How it is built
---------------

build-amiga.sh fetches X-pired's Linux source release (xpired.lock: URL and SHA-256), puts patches/ on it and builds it with the os32-gcc16-open6 stove:

- patches/0001: the data, the settings and the demo record in PROGDIR: (AmigaOS has no "./").
- patches/0002: SDL 2's typed characters for the password line, and the music looping in SDL2_mixer.
- compat/: the SDL 1.2 calls X-pired makes, on SDL 2 (one window whose surface is the 1.2 screen), and its four SDL_gfx drawing calls written here, with the text in the system's own topaz 8.

Only the game is ported, not the level editor (xpiredit).

Tests: make -C tests test.

X-pired is GPL 2 or later (X-pired.txt in the drawer); so are these port files (COPYING.txt).

Designed, tested and delivered by The AmiCradle Team.
