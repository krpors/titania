#ifndef BITMAPFONT_H
#define BITMAPFONT_H

#include <stdbool.h>

#include <SDL.h>

struct bitmapfont {
	SDL_Renderer* renderer;
	SDL_Surface* surf;
	int glyph_count;
	char* glyphs;
	SDL_Rect** rects;
	SDL_Texture* texture;
};

bool bitmapfont_init(struct bitmapfont* bmf, SDL_Renderer* renderer, const char* path, const char* glyphs);
void bitmapfont_free(struct bitmapfont* bmf);
void bitmapfont_render(struct bitmapfont* bmf, SDL_Renderer* renderer, const char* txt);

#endif // BITMAPFONT_H
