#ifndef BITMAPFONT_H
#define BITMAPFONT_H

#include <stdbool.h>

#include <SDL.h>

struct bitmapfont {
	SDL_Surface* surf;
};

bool bitmapfont_init(struct bitmapfont* bmf, const char* path);
void bitmapfont_free(struct bitmapfont* bmf);

#endif // BITMAPFONT_H
