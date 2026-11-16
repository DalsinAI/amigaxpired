/* SDL 1.2's video calls on SDL 2, for X-pired: one window whose surface is
 * the 1.2 "screen" (compat/include/sdl12on2.h).
 * Copyright (c) 2026 AmiCradle Team. GPL 2 or later, like X-pired. */
#include <SDL.h>

static SDL_Window *window;
static Uint32 window_format = SDL_PIXELFORMAT_RGB888;
static SDL12_EventFilter filter12;
static char caption[128] = "X-pired";

const SDL_VideoInfo *SDL_GetVideoInfo(void)
{
    static SDL_VideoInfo info;
    static SDL_PixelFormat *fmt;
    Uint32 want = window ? SDL_GetWindowPixelFormat(window) : window_format;
    if (want == SDL_PIXELFORMAT_UNKNOWN)
        want = SDL_PIXELFORMAT_RGB888;
    if (!fmt || fmt->format != want) {
        if (fmt)
            SDL_FreeFormat(fmt);
        fmt = SDL_AllocFormat(want);
    }
    info.vfmt = fmt;
    return &info;
}

SDL_Surface *SDL_SetVideoMode(int w, int h, int bpp, Uint32 flags)
{
    Uint32 wf = (flags & SDL_FULLSCREEN) ? SDL_WINDOW_FULLSCREEN : 0;
    (void)bpp;
    if (window) {
        /* the same window, at the new size and fullscreen or not */
        SDL_SetWindowFullscreen(window, wf);
        if (!wf)
            SDL_SetWindowSize(window, w, h);
    } else {
        /* the caption SDL_WM_SetCaption gave before there was a window */
        window = SDL_CreateWindow(caption, SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED, w, h, wf);
        if (!window)
            return NULL;
    }
    window_format = SDL_GetWindowPixelFormat(window);
    return SDL_GetWindowSurface(window);
}

void SDL_UpdateRect(SDL_Surface *screen, Sint32 x, Sint32 y, Uint32 w, Uint32 h)
{
    SDL_Rect r;
    if (!window)
        return;
    if (!x && !y && !w && !h) {
        SDL_UpdateWindowSurface(window);
        return;
    }
    r.x = x; r.y = y; r.w = (int)w; r.h = (int)h;
    if (screen && !SDL_IntersectRect(&r, &screen->clip_rect, &r))
        return;
    SDL_UpdateWindowSurfaceRects(window, &r, 1);
}

SDL_Surface *SDL_DisplayFormat(SDL_Surface *s)
{
    SDL_Surface *screen;
    if (!s)
        return NULL;
    screen = window ? SDL_GetWindowSurface(window) : NULL;
    /* SDL_ConvertSurface keeps the colour key, alpha and blend mode */
    return screen ? SDL_ConvertSurface(s, screen->format, 0)
                  : SDL_ConvertSurfaceFormat(s, window_format, 0);
}

int SDL_SetAlpha(SDL_Surface *s, Uint32 flag, Uint8 alpha)
{
    if (!s)
        return -1;
    if (flag & SDL_SRCALPHA) {
        SDL_SetSurfaceAlphaMod(s, alpha);
        return SDL_SetSurfaceBlendMode(s, SDL_BLENDMODE_BLEND);
    }
    SDL_SetSurfaceAlphaMod(s, 255);
    return SDL_SetSurfaceBlendMode(s, SDL_BLENDMODE_NONE);
}

void SDL_WM_SetCaption(const char *title, const char *icon)
{
    (void)icon;
    SDL_strlcpy(caption, title ? title : "", sizeof caption);
    if (window)
        SDL_SetWindowTitle(window, caption);
}

void SDL_WM_SetIcon(SDL_Surface *icon, Uint8 *mask)
{
    (void)mask;
    /* before SDL_SetVideoMode, as 1.2 wants it: nothing to show it on yet */
    if (window && icon)
        SDL_SetWindowIcon(window, icon);
}

static int filter2(void *user, SDL_Event *e)
{
    (void)user;
    return filter12 ? filter12(e) : 1;
}

void sdl12on2_set_filter(SDL12_EventFilter f)
{
    filter12 = f;
    (SDL_SetEventFilter)(f ? filter2 : NULL, NULL);  /* SDL 2's, not the macro */
}
