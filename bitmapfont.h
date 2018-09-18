#ifndef BITMAPFONT_H
#define BITMAPFONT_H

#include <stdbool.h>

#include <SDL.h>

/*
 * The bitmapfont contains information for rendering a char* on an SDL_Renderer.
 * A bitmapfont is initialized using an image, where each glyph is separated by
 * a 'separation color'. The separation color is the first pixel read in the
 * image, in the top-left corner.
 */
struct bitmapfont {
	SDL_Renderer* renderer; // Used to create a texture from a surface.
	SDL_Surface* surface;   // Image file to surface. Used to read pixels.
	char* glyphs;           // The characters.
	size_t glyphs_len;      // Amount of glyphs.
	SDL_Rect** rects;       // The individual rectangles per glyph.
	SDL_Texture* texture;   // The texture containing the glyphs image.
};

/*
 * Initialize the bitmapfont pointed at by `bmf'. The renderer is used to
 * create a texture from the surface. The `path' is the file to the font
 * texture, and the `glyphs'
 *
 * TODO: document the way the glyphs and the font image works together.
 */

struct bitmapfont* bitmapfont_create(SDL_Renderer* r, const char* path, const char* glyphs);

/*
 * Frees resources.
 */
void bitmapfont_free(struct bitmapfont* bmf);

/*
 * Renders the given `txt' at the position x, y on the `renderer' of the bitmapfont.
 */
void bitmapfont_render(struct bitmapfont* bmf, int x, int y, const char* txt);

/*
 * This function will render the formatted string to the renderer at starting
 * position (x, y). It will be slightly slower than bitmapfont_render since it
 * will use `vsnprintf' twice (once for knowing the size of the string, once
 * for actually formatting it to the target char*).
 */
void bitmapfont_renderf(struct bitmapfont* bmf, int x, int y, const char* fmt, ...);

#endif // BITMAPFONT_H
