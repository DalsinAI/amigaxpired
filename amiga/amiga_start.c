/* The Amiga port's stack and version string.
 * Copyright (c) 2026 AmiCradle Team. GPL 2 or later, like X-pired. */

/* A Shell gives 4 KB of stack; SDL 2 wants more. libnix's startup sets
 * this much up. */
unsigned long __stack = 256 * 1024;
static const char version[] __attribute__((used)) =
    "$VER: Xpired 1.22 (16.11.2026)";
