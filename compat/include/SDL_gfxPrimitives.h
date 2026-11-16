/* The four drawing calls X-pired makes from SDL_gfx 1.2 (boxRGBA,
 * rectangleRGBA, filledCircleRGBA and stringRGBA), written here
 * (compat/gfxprim.c) on SDL 2 surfaces, with the text in the system's own
 * 8 x 8 font (compat/font8_amiga.c: topaz 8).
 * Copyright (c) 2026 AmiCradle Team. GPL 2 or later, like X-pired. */
#ifndef XP_GFXPRIMITIVES_H
#define XP_GFXPRIMITIVES_H
#include <SDL.h>

/* Inclusive corners, as SDL_gfx draws them; a below 255 blends. 0 when
 * drawn (or wholly clipped), -1 when the surface can't be locked. */
int boxRGBA(SDL_Surface *dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2,
            Uint8 r, Uint8 g, Uint8 b, Uint8 a);
int rectangleRGBA(SDL_Surface *dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2,
                  Uint8 r, Uint8 g, Uint8 b, Uint8 a);
int filledCircleRGBA(SDL_Surface *dst, Sint16 x, Sint16 y, Sint16 rad,
                     Uint8 r, Uint8 g, Uint8 b, Uint8 a);
/* 8 x 8 cells from (x, y), one per byte of s */
int stringRGBA(SDL_Surface *dst, Sint16 x, Sint16 y, const char *s,
               Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* 256 glyphs of 8 rows, the top row first, bit 7 the left pixel */
const unsigned char *xp_font8(void);
#endif
