/* Host test for compat/: the SDL_gfx calls (compat/gfxprim.c) pixel by
 * pixel on 32-, 24- and 16-bit surfaces, and the SDL 1.2 calls
 * (compat/sdl12on2.c) on SDL 2's dummy video driver.
 * Copyright (c) 2026 AmiCradle Team. GPL 2 or later, like X-pired. */
#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include "SDL_gfxPrimitives.h"

static int bad;
#define CHECK(c, ...) do { if (!(c)) { printf("FAIL %d: ", __LINE__); printf(__VA_ARGS__); printf("\n"); bad = 1; } } while (0)

/* the stand-in font: 'A' is a box outline, every other glyph blank */
static unsigned char font[2048];
const unsigned char *xp_font8(void) { return font; }

static Uint32 px(SDL_Surface *s, int x, int y)
{
    Uint8 *p = (Uint8 *)s->pixels + y * s->pitch + x * s->format->BytesPerPixel;
    switch (s->format->BytesPerPixel) {
    case 2: return *(Uint16 *)p;
    case 3: return p[0] | p[1] << 8 | p[2] << 16;
    default: return *(Uint32 *)p;
    }
}

static void rgb(SDL_Surface *s, int x, int y, Uint8 *r, Uint8 *g, Uint8 *b)
{
    SDL_GetRGB(px(s, x, y), s->format, r, g, b);
}

static int is(SDL_Surface *s, int x, int y, int r, int g, int b, int tol)
{
    Uint8 R, G, B;
    rgb(s, x, y, &R, &G, &B);
    return abs(R - r) <= tol && abs(G - g) <= tol && abs(B - b) <= tol;
}

static int count(SDL_Surface *s, int r, int g, int b)
{
    int x, y, n = 0;
    for (y = 0; y < s->h; y++)
        for (x = 0; x < s->w; x++)
            n += is(s, x, y, r, g, b, 0);
    return n;
}

static void primitives(Uint32 format, int tol)
{
    SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, 40, 30, 0, format);
    const char *name = SDL_GetPixelFormatName(format);
    int n, y;
    SDL_FillRect(s, NULL, SDL_MapRGB(s->format, 0, 0, 0));

    /* solid box, corners inclusive and given either way round */
    boxRGBA(s, 12, 8, 2, 3, 255, 0, 0, 255);
    CHECK(count(s, 255, 0, 0) == 11 * 6, "%s: solid box %d pixels, not 66", name, count(s, 255, 0, 0));
    CHECK(is(s, 2, 3, 255, 0, 0, 0) && is(s, 12, 8, 255, 0, 0, 0) && is(s, 13, 8, 0, 0, 0, 0), "%s: solid box corners", name);

    /* blended box: half way from red to blue at a = 128 */
    boxRGBA(s, 2, 3, 2, 3, 0, 0, 255, 128);
    CHECK(is(s, 2, 3, 128, 0, 128, tol + 1), "%s: blend at a 128", name);
    /* a = 0 changes nothing */
    boxRGBA(s, 3, 3, 3, 3, 0, 255, 0, 0);
    CHECK(is(s, 3, 3, 255, 0, 0, 0), "%s: a 0 drew", name);

    /* rectangle: the outline only */
    SDL_FillRect(s, NULL, 0);
    rectangleRGBA(s, 20, 10, 29, 19, 0, 255, 0, 255);
    CHECK(count(s, 0, 255, 0) == 36, "%s: rectangle %d pixels, not 36", name, count(s, 0, 255, 0));
    CHECK(is(s, 24, 14, 0, 0, 0, 0), "%s: rectangle filled", name);

    /* filled circle: symmetric, the centre and rim in, past the rim out */
    SDL_FillRect(s, NULL, 0);
    filledCircleRGBA(s, 20, 15, 5, 255, 255, 255, 255);
    n = count(s, 255, 255, 255);
    CHECK(n > 3.0 * 25 && n < 3.4 * 36, "%s: circle area %d", name, n);
    CHECK(is(s, 20, 15, 255, 255, 255, 0) && is(s, 25, 15, 255, 255, 255, 0) && is(s, 20, 10, 255, 255, 255, 0), "%s: circle centre or rim", name);
    CHECK(is(s, 26, 15, 0, 0, 0, 0) && is(s, 20, 21, 0, 0, 0, 0) && is(s, 24, 19, 0, 0, 0, 0), "%s: circle too big", name);
    for (y = 0; y < s->h; y++) {
        int x;
        for (x = 0; x <= 20; x++)
            CHECK(is(s, x, y, 255, 255, 255, 0) == is(s, 40 - x, y, 255, 255, 255, 0) || 40 - x >= s->w,
                  "%s: circle not symmetric at %d,%d", name, x, y);
    }

    /* text: 'A' at (4, 2), the box outline of the stand-in glyph; clipped */
    SDL_FillRect(s, NULL, 0);
    stringRGBA(s, 4, 2, "A ", 255, 255, 0, 255);
    CHECK(count(s, 255, 255, 0) == 28, "%s: text %d pixels, not 28", name, count(s, 255, 255, 0));
    CHECK(is(s, 4, 2, 255, 255, 0, 0) && is(s, 11, 9, 255, 255, 0, 0) && is(s, 5, 3, 0, 0, 0, 0), "%s: glyph place", name);
    SDL_FillRect(s, NULL, 0);
    {
        SDL_Rect c = { 0, 0, 8, 30 };
        SDL_SetClipRect(s, &c);
        stringRGBA(s, 4, 2, "A", 255, 255, 0, 255);
        boxRGBA(s, 0, 20, 39, 20, 255, 255, 0, 100);
        SDL_SetClipRect(s, NULL);
        CHECK(is(s, 7, 2, 255, 255, 0, 0) && is(s, 8, 2, 0, 0, 0, 0), "%s: text past the clip", name);
        CHECK(!is(s, 7, 20, 0, 0, 0, 0) && is(s, 8, 20, 0, 0, 0, 0), "%s: blend past the clip", name);
    }
    /* wholly off the surface: no harm */
    boxRGBA(s, -50, -50, -10, -10, 1, 2, 3, 100);
    stringRGBA(s, 500, 500, "AAAA", 1, 2, 3, 255);
    filledCircleRGBA(s, -100, -100, 10, 1, 2, 3, 77);
    SDL_FreeSurface(s);
}

static int filtered;
static int filter12(const SDL_Event *e)
{
    filtered++;
    return e->type != SDL_USEREVENT;   /* drop the user events */
}

static void video(void)
{
    const SDL_VideoInfo *vi;
    SDL_Surface *screen, *spr, *disp;
    SDL_Event e;
    Uint32 key;
    Uint8 a;
    SDL_BlendMode bm;
    SDL_Keysym k;

    SDL_WM_SetCaption("before the window", NULL);
    vi = SDL_GetVideoInfo();
    CHECK(vi && vi->vfmt && vi->vfmt->BitsPerPixel >= 24, "SDL_GetVideoInfo");
    screen = SDL_SetVideoMode(400, 400, vi->vfmt->BitsPerPixel, SDL_SRCALPHA | SDL_HWSURFACE);
    CHECK(screen && screen->w == 400 && screen->h == 400, "SDL_SetVideoMode");
    if (!screen)
        return;
    CHECK(SDL_GetWindowFromID(1) && !strcmp(SDL_GetWindowTitle(SDL_GetWindowFromID(1)), "before the window"), "the caption given before the window");
    SDL_UpdateRect(screen, 0, 0, 0, 0);
    SDL_UpdateRect(screen, 10, 10, 50, 50);
    SDL_UpdateRect(screen, 380, 380, 50, 50);  /* runs off the edge: clipped */

    /* a sprite as LoadSprite makes it: colour key, surface alpha, display format */
    spr = SDL_CreateRGBSurface(SDL_SRCALPHA, 20, 20, 24, 0xff0000, 0xff00, 0xff, 0);
    SDL_SetColorKey(spr, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(spr->format, 255, 0, 255));
    SDL_SetAlpha(spr, SDL_SRCALPHA, 100);
    disp = SDL_DisplayFormat(spr);
    CHECK(disp && disp->format->format == screen->format->format, "SDL_DisplayFormat: the window's format");
    CHECK(disp && SDL_GetColorKey(disp, &key) == 0 && key == SDL_MapRGB(disp->format, 255, 0, 255), "SDL_DisplayFormat drops the colour key");
    CHECK(disp && SDL_GetSurfaceAlphaMod(disp, &a) == 0 && a == 100, "SDL_DisplayFormat drops the surface alpha");
    CHECK(disp && SDL_GetSurfaceBlendMode(disp, &bm) == 0 && bm == SDL_BLENDMODE_BLEND, "SDL_DisplayFormat drops the blending");
    /* the key and alpha at work: magenta leaves the screen as it was, the
     * rest blends 100/255 of the way */
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    SDL_FillRect(disp, NULL, SDL_MapRGB(disp->format, 255, 0, 255));
    {
        SDL_Rect r = { 0, 0, 10, 20 };
        SDL_FillRect(disp, &r, SDL_MapRGB(disp->format, 255, 255, 255));
    }
    {
        SDL_Rect d = { 100, 100, 0, 0 };
        SDL_BlitSurface(disp, NULL, screen, &d);
    }
    CHECK(is(screen, 115, 105, 0, 0, 0, 0), "the colour key didn't hold");
    CHECK(is(screen, 105, 105, 100, 100, 100, 2), "the surface alpha didn't blend");
    SDL_SetAlpha(disp, 0, 0);
    CHECK(SDL_GetSurfaceBlendMode(disp, &bm) == 0 && bm == SDL_BLENDMODE_NONE, "SDL_SetAlpha(0) leaves blending on");
    SDL_WM_SetIcon(spr, NULL);
    SDL_FreeSurface(disp);
    SDL_FreeSurface(spr);

    /* the 1.2 event filter: called for each event, 0 drops it */
    SDL_SetEventFilter(filter12);
    SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
    filtered = 0;
    memset(&e, 0, sizeof e);
    e.type = SDL_USEREVENT; SDL_PushEvent(&e);
    e.type = SDL_KEYDOWN; e.key.keysym.sym = SDLK_LEFT; SDL_PushEvent(&e);
    CHECK(filtered == 2, "the filter saw %d events, not 2", filtered);
    CHECK(SDL_PollEvent(&e) && e.type == SDL_KEYDOWN && !SDL_PollEvent(&e), "the filter didn't drop the user event");
    SDL_SetEventFilter(NULL);

    /* keysym.unicode as 1.2 gave it */
    memset(&k, 0, sizeof k);
    k.sym = SDLK_a; CHECK(sdl12on2_char(&k) == 'a', "a");
    k.mod = KMOD_LSHIFT; CHECK(sdl12on2_char(&k) == 'A', "Shift a");
    k.mod = KMOD_CAPS; CHECK(sdl12on2_char(&k) == 'A', "Caps a");
    k.mod = 0; k.sym = SDLK_7; CHECK(sdl12on2_char(&k) == '7', "7");
    k.sym = SDLK_BACKSPACE; CHECK(sdl12on2_char(&k) == 8, "Backspace");
    k.sym = SDLK_RETURN; CHECK(sdl12on2_char(&k) == 13, "Return");
    k.sym = SDLK_KP_ENTER; CHECK(sdl12on2_char(&k) == 13, "Enter");
    k.sym = SDLK_LEFT; CHECK(sdl12on2_char(&k) == 0, "Left");
    k.sym = SDLK_LCTRL; CHECK(sdl12on2_char(&k) == 0, "Ctrl");

    /* Alt+Return's fullscreen and back: the same window */
    screen = SDL_SetVideoMode(400, 400, vi->vfmt->BitsPerPixel, SDL_FULLSCREEN);
    CHECK(screen != NULL, "fullscreen");
    screen = SDL_SetVideoMode(400, 400, vi->vfmt->BitsPerPixel, 0);
    CHECK(screen && screen->w == 400, "back to a window");
}

int main(void)
{
    int i;
    for (i = 0; i < 8; i++) {          /* 'A': an 8 x 8 box outline */
        font[8 * 'A' + i] = (i == 0 || i == 7) ? 0xFF : 0x81;
    }
    primitives(SDL_PIXELFORMAT_RGB888, 0);
    primitives(SDL_PIXELFORMAT_ARGB8888, 0);
    primitives(SDL_PIXELFORMAT_RGB24, 0);
    primitives(SDL_PIXELFORMAT_RGB565, 8);
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("FAIL: SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    video();
    SDL_Quit();
    if (!bad)
        printf("compat: all checks passed\n");
    return bad;
}
