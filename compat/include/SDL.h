/* X-pired includes <SDL.h> as SDL 1.2 wrote it; this is found first
 * (-Icompat/include) and brings SDL 2 with the 1.2 calls X-pired makes.
 * Copyright (c) 2026 AmiCradle Team. GPL 2 or later, like X-pired. */
#ifndef XP_COMPAT_SDL_H
#define XP_COMPAT_SDL_H
#include_next <SDL.h>
#include "sdl12on2.h"
#endif
