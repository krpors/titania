#include "bitmapfont.h"
#include "util.h"

#include <assert.h>
#include <stdbool.h>
#include <stdarg.h>

#include <SDL_image.h>

//#############################################################################
// Private functions for the bitmap font.
//#############################################################################

/*
 * Given the first pixel found at the top-left corner of the image (which is
 * the separator color), this function will find all 'glyphs' separated by that
 * color. Every glyph is just a SDL_Rect with a specific x coordinate and width.
 * The y coordinate will always be zero and the height will always be the height
 * of the surface.
 */
static void bitmapfont_find_glyphs(struct bitmapfont* bmf) {
	SDL_PixelFormat* fmt = bmf->surface->format;

	// We're going to read some pixels, so SDL requires us to lock the surface
	// before doing anything with it.
	SDL_LockSurface(bmf->surface);

	// We get the first pixel at the top left corner of the image. This pixel
	// color denotes the separator of the glyphs in the image.
	Uint8 index = *(Uint8 *)bmf->surface->pixels;
	SDL_Color sepColor = fmt->palette->colors[index];

	// The width of the current glyph in pixels.
	int glyph_width = 0;

	// If this boolean is set to true, we are about to read the
	// width of a glyph in the iteration.
	bool read_glyph = false;

	// We only need to iterate over the width of the image, since each glyph
	// is a rectangle anyway, and it should be separated by the separator color.
	for (int x = 0; x < bmf->surface->w; x++) {
		Uint8 bla = ((Uint8*) bmf->surface->pixels)[x];
		SDL_Color currentColor = fmt->palette->colors[bla];

		// Is the current pixel color a separator color?
		bool curr_is_sep = is_color_equal(&currentColor, &sepColor);

		if (curr_is_sep) {
			if (read_glyph) {

				// Create a rectangle on the heap, add it to the `rects' array.
				SDL_Rect* r1 = malloc(1 * sizeof(SDL_Rect));
				r1->x = x - glyph_width;
				r1->y = 0;
				r1->w = glyph_width;
				r1->h = bmf->surface->h;
				bmf->rects[bmf->glyphs_len] = r1;

				bmf->glyphs_len++;

				// Reallocate the array with extra space for another
				// rectangle.  We don't care too much about optimalization,
				// so we increase it with 1 sizeof each time.
				bmf->rects = realloc(bmf->rects, (bmf->glyphs_len + 1) * sizeof(SDL_Rect*));
			}
			glyph_width = 0;
			read_glyph = false;
		} else {
			read_glyph = true;
			glyph_width++;
		}
	}
	SDL_UnlockSurface(bmf->surface);
}

//#############################################################################
// Public functions for the bitmap font.
//#############################################################################

struct bitmapfont* bitmapfont_create(SDL_Renderer* renderer, const char* path, const char* glyphs) {
	struct bitmapfont* bmf = malloc(sizeof(struct bitmapfont));

	// Pre-initialize some member so we can safely free them when things run
	// into an error.
	bmf->glyphs_len = 0;
	bmf->rects = malloc(1 * sizeof(SDL_Rect*));
	bmf->texture = NULL;

	// The renderer is used to create a texture from a surface.
	bmf->renderer = renderer;

	// Dupe the glyphs string.
	bmf->glyphs = strdup(glyphs);
	if (bmf->glyphs == NULL) {
		// failed to dupe, should hardly be the case, ever.
		perror("unable to duplicate string");
		bitmapfont_free(bmf);
		return NULL;
	}

	bmf->surface = IMG_Load(path);
	if (bmf->surface == NULL) {
		fprintf(stderr, "%s\n", IMG_GetError());
		bitmapfont_free(bmf);
		return NULL;
	}

	SDL_PixelFormat* fmt = bmf->surface->format;

	// Fonts must be 8 bits per pixel.
	if (fmt->BitsPerPixel != 8) {
		fprintf(stderr, "font image must be 8 bits per pixel\n");
		bitmapfont_free(bmf);
		return NULL;
	}

	// Start finding the glyphs in the SDL_Surface.
	bitmapfont_find_glyphs(bmf);

	// We check the amount of glyphs found in the image with the
	// glyphs in the const char* argument of the function. They
	// should be equal (or it won't make much sense).
	if (bmf->glyphs_len != strlen(glyphs)) {
		fprintf(stderr,
			"glyph count (%li) in image does not equal "
			"the glyph count in the string (%li)\n", bmf->glyphs_len, strlen(glyphs));
		bitmapfont_free(bmf);
		return NULL;
	}


	bmf->texture = SDL_CreateTextureFromSurface(renderer, bmf->surface);
	if (bmf->texture == NULL) {
		fprintf(stderr, "Unable to create texture from surface: %s\n", SDL_GetError());
		bitmapfont_free(bmf);
		return NULL;
	}

	// After we are done, we don't need the surface so we can free it prematurely.
	SDL_FreeSurface(bmf->surface);
	bmf->surface = NULL;

	return bmf;
}

void bitmapfont_free(struct bitmapfont* bmf) {
	SDL_FreeSurface(bmf->surface);
	bmf->surface = NULL;

	SDL_DestroyTexture(bmf->texture);
	bmf->texture = NULL;

	free(bmf->glyphs);
	bmf->glyphs = NULL;

	// Free every malloc'd rectangle
	for (size_t i = 0; i < bmf->glyphs_len; i++) {
		free(bmf->rects[i]);
		bmf->rects[i] = NULL;
	}
	// And free the array itself.
	free(bmf->rects);
	bmf->rects = NULL;

	free(bmf);
}

void bitmapfont_render(struct bitmapfont* bmf, int x, int y, const char* txt) {
	// Note: this 'algorithm' is very naive as it is, but does the trick, for now.

	size_t txtLen = strlen(txt);
	// Iterate over all characters in the `txt'...
	for (size_t i = 0; i < txtLen; i++) {
		// Check if the current character in `txt' can be found in the `glyphs'
		// variable. If so, we use the index `j' to access the `rects' array.
		// That rect contains our rectangle to render the glyph.
		for (size_t j = 0; j < bmf->glyphs_len; j++) {
			if (txt[i] == bmf->glyphs[j]) {
				SDL_Rect* src = bmf->rects[j];
				SDL_Rect dst = { .x = x, .y = y, .w = src->w, .h = src->h };
				SDL_RenderCopy(bmf->renderer, bmf->texture, src, &dst);

				// The 'kerning' is done based on the width of the glyph. We
				// increase the x coordinate with the previous glyph's width
				// each time after rendering.
				x += src->w;
			}
		}
	}
}

void bitmapfont_renderf(struct bitmapfont* bmf, int x, int y, const char* fmt, ...) {
	// Note: this implementation has been taken from `man vsnprintf`.
	int size = 0;
	char* p = NULL;
	va_list ap;

	// Determine required size:
	va_start(ap, fmt);
	size = vsnprintf(p, size, fmt, ap);
	va_end(ap);

	if (size < 0) {
		// nothing to do.
		return;
	}

	size++; // for '\0'.
	p = malloc(size * sizeof(char));
	va_start(ap, fmt);
	size = vsnprintf(p, size, fmt, ap);
	if (size < 0) {
		free(p);
	}
	va_end(ap);

	// We got a formatted char*, render it!
	bitmapfont_render(bmf, x, y, p);
	free(p);
}
