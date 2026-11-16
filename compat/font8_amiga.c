/* stringRGBA's 8 x 8 glyphs, from the system's own topaz 8 (GFOS's
 * graphics.library): the printable characters are drawn once into a
 * one-plane bitmap with Text() and read back, a byte a row. Characters the
 * font has no glyph for stay blank.
 * Copyright (c) 2026 AmiCradle Team. GPL 2 or later, like X-pired. */
#include <string.h>
#include <exec/types.h>
#include <graphics/gfx.h>
#include <graphics/rastport.h>
#include <graphics/text.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include "SDL_gfxPrimitives.h"

#define FIRST 32
#define COUNT 224          /* 32 to 255: ASCII and Latin-1 */

static unsigned char glyphs[256 * 8];

const unsigned char *xp_font8(void)
{
    static int done;
    struct TextAttr ta = { (STRPTR)"topaz.font", 8, 0, 0 };
    struct TextFont *tf;
    struct BitMap bm;
    struct RastPort rp;
    PLANEPTR plane;
    int c, row;

    if (done)
        return glyphs;
    done = 1;
    tf = OpenFont(&ta);
    if (!tf)
        return glyphs;
    if (tf->tf_YSize != 8 || tf->tf_XSize != 8 || (tf->tf_Flags & FPF_PROPORTIONAL)) {
        CloseFont(tf);
        return glyphs;
    }
    plane = AllocRaster(8 * COUNT, 8);
    if (!plane) {
        CloseFont(tf);
        return glyphs;
    }
    InitBitMap(&bm, 1, 8 * COUNT, 8);
    bm.Planes[0] = plane;
    InitRastPort(&rp);
    rp.BitMap = &bm;
    SetRast(&rp, 0);
    SetFont(&rp, tf);
    SetAPen(&rp, 1);
    SetDrMd(&rp, JAM1);
    for (c = FIRST; c < FIRST + COUNT; c++) {
        char ch = (char)c;
        if (c == 127 || (c >= 128 && c < 160))
            continue;      /* control codes */
        if (c < tf->tf_LoChar || c > tf->tf_HiChar)
            continue;
        Move(&rp, (c - FIRST) * 8, tf->tf_Baseline);
        Text(&rp, (STRPTR)&ch, 1);
    }
    WaitBlit();
    for (c = FIRST; c < FIRST + COUNT; c++)
        for (row = 0; row < 8; row++)
            glyphs[8 * c + row] = plane[row * bm.BytesPerRow + (c - FIRST)];
    FreeRaster(plane, 8 * COUNT, 8);
    CloseFont(tf);
    return glyphs;
}
