# Host tests: make -C tests test (the SDL 2 checks need libsdl2-dev,
# libsdl2-image-dev and libsdl2-mixer-dev; XP_TGZ=<tarball> saves a download)
SDL_CFLAGS = $(shell pkg-config --cflags sdl2)
SDL_LIBS = $(shell pkg-config --libs sdl2)
test:
	sh test-part.sh
	sh test-patches.sh
	$(CC) -Wall -I../compat/include $(SDL_CFLAGS) -o test-compat test-compat.c ../compat/gfxprim.c ../compat/sdl12on2.c $(SDL_LIBS) && SDL_VIDEODRIVER=dummy ./test-compat && rm -f test-compat
.PHONY: test
