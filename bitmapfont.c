#include "bitmapfont.h"
#include "util.h"

#include <assert.h>
#include <stdbool.h>

#include <SDL_image.h>


bool bitmapfont_init(struct bitmapfont* bmf, SDL_Renderer* renderer, const char* path, const char* glyphs) {
	assert(bmf != NULL);
	assert(path != NULL);

	bmf->renderer = renderer;

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

	// We get the first pixel at the top left corner of the
	// image. This pixel denotes the separator of the glyphs
	// in the image.
	Uint8 index = *(Uint8 *)surf->pixels;
	SDL_Color sepColor = fmt->palette->colors[index];

	printf("Separator sepColor is: %d,%d,%d\n", sepColor.r, sepColor.g, sepColor.b);

	int z = 0;

	bmf->rects = malloc(1 * sizeof(SDL_Rect*));

	// If this boolean is set to true, we are about to read the
	// width of a glyph in the iteration.
	bool read_glyph = false;

	size_t glyph_count = 0;

	for (int i = 0; i < surf->w; i++) {
		Uint8 bla = ((Uint8*) surf->pixels)[i];
		SDL_Color currentColor = fmt->palette->colors[bla];

		// Is t he current pixel color a separator color?
		bool curr_is_sep = is_color_equal(&currentColor, &sepColor);

		if (curr_is_sep) {
			if (read_glyph) {

				SDL_Rect* r1 = malloc(1 * sizeof(SDL_Rect));
				r1->x = i - z;
				r1->y = 0;
				r1->w = z;
				r1->h = surf->h;
				bmf->rects[glyph_count] = r1;

				//printf("Glyph %c: (%d,%d)-(%d w × %d h)\n", glyphs[glyph_count], r.x, r.y, r.w, r.h);
				glyph_count++;
				bmf->rects = realloc(bmf->rects, (glyph_count + 1) * sizeof(SDL_Rect*));
			}
			z = 0;
			read_glyph = false;
		} else {
			read_glyph = true;
			z++;
		}
	}
	SDL_UnlockSurface(surf);

	// We check the amount of glyphs found in the image with the
	// glyphs in the const char* argument of the function. They
	// should be equal (or it won't make much sense).
	if (glyph_count != strlen(glyphs)) {
		fprintf(stderr,
			"glyph count (%li) in image does not equal "
			"the glyph count in the string (%li)\n", glyph_count, strlen(glyphs));
		return false;
	}

	// Everything went OK. Dupe the glyphs string.
	bmf->glyphs = strdup(glyphs);
	if (bmf->glyphs == NULL) {
		// failed to dupe, should hardly be the case, ever.
		perror("unable to duplicate string");
		return false;
	}

	bmf->texture = SDL_CreateTextureFromSurface(renderer, surf);
	if (bmf->texture == NULL) {
		fprintf(stderr, "Unable to create texture from surface: %s\n", SDL_GetError());
		// TODO: freeeee
		return false;
	}

#if 0
	for (int i = 0; i < (int)glyph_count; ++i) {
		SDL_Rect* r = bmf->rects[i];
		printf("glyph %d => %d,%d\n", i, r->x, r->w);
	}
#endif

	return true;
}

void bitmapfont_free(struct bitmapfont* bmf) {
	SDL_FreeSurface(bmf->surf);
	free(bmf->glyphs);
	free(bmf->rects);

	bmf->surf = NULL;
	bmf->glyphs = NULL;
	bmf->rects = NULL;
}

void bitmapfont_render(struct bitmapfont* bmf, SDL_Renderer* renderer, const char* txt) {
	// The 'kerning' done based on the width of the glyph.
	// We increase the spacing each time after rendering a
	// glyph.
	int spacing = 0;
	for (size_t i = 0; i < strlen(txt); i++) {
		for (size_t j = 0; j < strlen(bmf->glyphs); j++) {
			if (txt[i] == bmf->glyphs[j]) {
				SDL_Rect* r = bmf->rects[j];

				SDL_Rect src = { .x = r->x, .y = r->y, .w = r->w, .h = r->h };
				SDL_Rect dst = { .x = spacing , .y = 80, .w = r->w, .h = r->h };
				SDL_RenderCopy(renderer, bmf->texture,  &src, &dst);
				spacing += r->w;
				//printf("Matched %c to index %li (%d, %d)\n", txt[i], j, r->x, r->w);
			}
		}
	}
}
