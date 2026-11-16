/* The SDL 1.2 video calls X-pired makes, on SDL 2 (GFOpenUp's SDL2.module):
 * one window whose surface is the 1.2 "screen" (compat/sdl12on2.c).
 * Copyright (c) 2026 AmiCradle Team. GPL 2 or later, like X-pired. */
#ifndef SDL12ON2_H
#define SDL12ON2_H

typedef SDL_Keycode SDLKey;

/* SDL 1.2's surface and video mode flags. SDL 2 takes no flags for a
 * surface; the colour key and the surface alpha are switched on by
 * SDL_SetColorKey and SDL_SetAlpha below. */
#define SDL_SWSURFACE   0
#define SDL_HWSURFACE   0
#define SDL_HWPALETTE   0
#define SDL_SRCCOLORKEY 0x00001000
#define SDL_SRCALPHA    0x00010000
#define SDL_FULLSCREEN  0x80000000u

typedef struct SDL_VideoInfo {
    SDL_PixelFormat *vfmt;   /* the format SDL_SetVideoMode will give */
} SDL_VideoInfo;

const SDL_VideoInfo *SDL_GetVideoInfo(void);
/* One window of w x h; again for a new size or fullscreen. bpp is the
 * window's own (SDL_GetVideoInfo). */
SDL_Surface *SDL_SetVideoMode(int w, int h, int bpp, Uint32 flags);
/* x, y, w, h all 0: the whole window */
void SDL_UpdateRect(SDL_Surface *screen, Sint32 x, Sint32 y, Uint32 w, Uint32 h);
/* A copy in the window's format, keeping the colour key and surface alpha */
SDL_Surface *SDL_DisplayFormat(SDL_Surface *s);
/* SDL_SRCALPHA: blend the whole surface at alpha; 0: opaque again */
int SDL_SetAlpha(SDL_Surface *s, Uint32 flag, Uint8 alpha);
void SDL_WM_SetCaption(const char *title, const char *icon);
void SDL_WM_SetIcon(SDL_Surface *icon, Uint8 *mask);

static inline int SDL_EnableKeyRepeat(int delay, int interval)
{
    (void)delay; (void)interval;
    return 0;
}

static inline int SDL_EnableUNICODE(int enable)
{
    (void)enable;              /* sdl12on2_char() gives the character */
    return 0;
}

/* SDL 1.2's event filter, called for each event as it is queued: 0 drops
 * it. SDL 2's takes a user pointer as well. */
typedef int (*SDL12_EventFilter)(const SDL_Event *event);
void sdl12on2_set_filter(SDL12_EventFilter filter);
#define SDL_SetEventFilter(f) sdl12on2_set_filter(f)

/* SDL 1.2's keysym.unicode for the keys X-pired reads as text (its
 * password line): the printable ASCII key, upper case with Shift or Caps
 * Lock; Backspace 8 and Return 13; 0 for any other key. */
static inline Uint16 sdl12on2_char(const SDL_Keysym *k)
{
    SDL_Keycode s = k->sym;
    if (s == SDLK_BACKSPACE || s == SDLK_RETURN)
        return (Uint16)s;
    if (s == SDLK_KP_ENTER)
        return 13;
    if (s < 32 || s > 126)
        return 0;
    if ((k->mod & (KMOD_SHIFT | KMOD_CAPS)) && s >= 'a' && s <= 'z')
        return (Uint16)(s - 'a' + 'A');
    return (Uint16)s;
}
#endif
