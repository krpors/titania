#include "bitmapfont.h"

#include <assert.h>
#include <stdbool.h>

#include <SDL_image.h>

static bool coloreq(SDL_Color* one, SDL_Color* two) {
	return one->r == two->r && one->g == two->g && one->b == two->b;
}

bool bitmapfont_init(struct bitmapfont* bmf, const char* path) {
	assert(bmf != NULL);

	bmf->surf = IMG_Load(path);
	if (bmf->surf == NULL) {
		fprintf(stderr, "%s\n", IMG_GetError());
		return false;
	}

	// Prevent lots of ->...->--> ...
	SDL_Surface* surf = bmf->surf;
	SDL_PixelFormat* fmt = bmf->surf->format;

	// Fonts must be 8 bits per pixel.
	if (fmt->BitsPerPixel != 8) {
		fprintf(stderr, "font image must be 8bpp\n");
		return false;
	}

	printf("Surface w x h = (%d, %d)\n", surf->w, surf->h);

	SDL_LockSurface(surf);
	Uint8 index = *(Uint8 *)surf->pixels;

	SDL_Color sepColor = fmt->palette->colors[index];
	printf("Separator sepColor is: %d,%d,%d\n", sepColor.r, sepColor.g, sepColor.b);

	int z = 0;

	bool read_glyph = false;

	for (int i = 0; i < surf->w; i++) {
		Uint8 bla = ((Uint8*) surf->pixels)[i];
		SDL_Color currentColor = fmt->palette->colors[bla];

		//printf("current sepColor : %3d, %3d, %3d\n", currentColor.r, currentColor.g, currentColor.b);
		bool curr_is_sep = coloreq(&currentColor, &sepColor);

		if (curr_is_sep) {
			if (read_glyph) {
				printf("glyph start - end: (%d - %d)\n", i - z, i);
			}
			z = 0;
			read_glyph = false;
		} else {
			read_glyph = true;
			z++;
		}

	}

	SDL_UnlockSurface(surf);

	return true;
}

void bitmapfont_free(struct bitmapfont* bmf) {
	SDL_FreeSurface(bmf->surf);
	bmf->surf = NULL;
}
