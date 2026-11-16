/* boxRGBA, rectangleRGBA, filledCircleRGBA and stringRGBA, the SDL_gfx
 * calls X-pired makes, written for any SDL 2 surface of 1 to 4 bytes a
 * pixel. A pixel below alpha 255 is blended as SDL_gfx 1.2 blends it on a
 * surface with no alpha channel: each of red, green and blue moves
 * (new - old) * a / 256 of the way, and an alpha channel, when there is
 * one, takes the drawn alpha. The surface's clip rectangle holds.
 * Copyright (c) 2026 AmiCradle Team. GPL 2 or later, like X-pired. */
#include <SDL.h>
#include "SDL_gfxPrimitives.h"

static Uint32 get_pixel(const SDL_Surface *s, int x, int y)
{
    const Uint8 *p = (const Uint8 *)s->pixels + y * s->pitch + x * s->format->BytesPerPixel;
    switch (s->format->BytesPerPixel) {
    case 1: return *p;
    case 2: return *(const Uint16 *)p;
    case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        return (Uint32)p[0] << 16 | (Uint32)p[1] << 8 | p[2];
#else
        return p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16;
#endif
    default: return *(const Uint32 *)p;
    }
}

static void put_pixel(SDL_Surface *s, int x, int y, Uint32 v)
{
    Uint8 *p = (Uint8 *)s->pixels + y * s->pitch + x * s->format->BytesPerPixel;
    switch (s->format->BytesPerPixel) {
    case 1: *p = (Uint8)v; break;
    case 2: *(Uint16 *)p = (Uint16)v; break;
    case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        p[0] = (Uint8)(v >> 16); p[1] = (Uint8)(v >> 8); p[2] = (Uint8)v;
#else
        p[0] = (Uint8)v; p[1] = (Uint8)(v >> 8); p[2] = (Uint8)(v >> 16);
#endif
        break;
    default: *(Uint32 *)p = v; break;
    }
}

/* one pixel, already inside the clip rectangle, the surface locked */
static void plot(SDL_Surface *s, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    Uint8 or_, og, ob, oa;
    if (a == 255) {
        put_pixel(s, x, y, SDL_MapRGBA(s->format, r, g, b, 255));
        return;
    }
    SDL_GetRGBA(get_pixel(s, x, y), s->format, &or_, &og, &ob, &oa);
    or_ = (Uint8)(or_ + (((int)r - or_) * a >> 8));
    og  = (Uint8)(og  + (((int)g - og)  * a >> 8));
    ob  = (Uint8)(ob  + (((int)b - ob)  * a >> 8));
    put_pixel(s, x, y, SDL_MapRGBA(s->format, or_, og, ob, s->format->Amask ? a : 255));
}

/* a span of row y from x1 to x2 (inclusive), clipped */
static void hspan(SDL_Surface *s, int x1, int x2, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    const SDL_Rect *c = &s->clip_rect;
    int x;
    if (y < c->y || y >= c->y + c->h)
        return;
    if (x1 < c->x) x1 = c->x;
    if (x2 > c->x + c->w - 1) x2 = c->x + c->w - 1;
    for (x = x1; x <= x2; x++)
        plot(s, x, y, r, g, b, a);
}

static int lock(SDL_Surface *s)
{
    if (!s || !s->format)
        return -1;
    if (SDL_MUSTLOCK(s) && SDL_LockSurface(s) < 0)
        return -1;
    return 0;
}

static void unlock(SDL_Surface *s)
{
    if (SDL_MUSTLOCK(s))
        SDL_UnlockSurface(s);
}

int boxRGBA(SDL_Surface *dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2,
            Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    int y, t;
    if (x1 > x2) { t = x1; x1 = x2; x2 = (Sint16)t; }
    if (y1 > y2) { t = y1; y1 = y2; y2 = (Sint16)t; }
    if (dst && a == 255) {
        /* solid: one fill, as SDL_gfx does */
        SDL_Rect rc;
        rc.x = x1; rc.y = y1; rc.w = x2 - x1 + 1; rc.h = y2 - y1 + 1;
        return SDL_FillRect(dst, &rc, SDL_MapRGBA(dst->format, r, g, b, 255)) < 0 ? -1 : 0;
    }
    if (lock(dst) < 0)
        return -1;
    for (y = y1; y <= y2; y++)
        hspan(dst, x1, x2, y, r, g, b, a);
    unlock(dst);
    return 0;
}

int rectangleRGBA(SDL_Surface *dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2,
                  Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    int y, t;
    if (x1 > x2) { t = x1; x1 = x2; x2 = (Sint16)t; }
    if (y1 > y2) { t = y1; y1 = y2; y2 = (Sint16)t; }
    if (lock(dst) < 0)
        return -1;
    hspan(dst, x1, x2, y1, r, g, b, a);
    if (y2 != y1)
        hspan(dst, x1, x2, y2, r, g, b, a);
    for (y = y1 + 1; y < y2; y++) {
        hspan(dst, x1, x1, y, r, g, b, a);
        if (x2 != x1)
            hspan(dst, x2, x2, y, r, g, b, a);
    }
    unlock(dst);
    return 0;
}

int filledCircleRGBA(SDL_Surface *dst, Sint16 x, Sint16 y, Sint16 rad,
                     Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    int dy, dx;
    if (rad < 0)
        return -1;
    if (lock(dst) < 0)
        return -1;
    /* each row once: the widest dx with dx*dx + dy*dy <= rad*rad (+ rad,
     * which rounds the rim as a midpoint circle does) */
    for (dy = -rad; dy <= rad; dy++) {
        int lim = rad * rad + rad - dy * dy;
        dx = 0;
        while ((dx + 1) * (dx + 1) <= lim)
            dx++;
        hspan(dst, x - dx, x + dx, y + dy, r, g, b, a);
    }
    unlock(dst);
    return 0;
}

int stringRGBA(SDL_Surface *dst, Sint16 x, Sint16 y, const char *s,
               Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    const unsigned char *font = xp_font8();
    const SDL_Rect *c;
    int row, col;
    if (!s || lock(dst) < 0)
        return -1;
    c = &dst->clip_rect;
    for (; *s; s++, x += 8) {
        const unsigned char *gl = font + 8 * (unsigned char)*s;
        for (row = 0; row < 8; row++) {
            int py = y + row;
            if (py < c->y || py >= c->y + c->h)
                continue;
            for (col = 0; col < 8; col++) {
                int px = x + col;
                if ((gl[row] & (0x80 >> col)) && px >= c->x && px < c->x + c->w)
                    plot(dst, px, py, r, g, b, a);
            }
        }
    }
    unlock(dst);
    return 0;
}
