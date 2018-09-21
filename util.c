#include "util.h"

#include <assert.h>
#include <stdbool.h>

#include <SDL.h>
#include <SDL_image.h>

extern SDL_Renderer* gRenderer;

SDL_Rect rect_to_SDLRect(const struct rect* r) {
	SDL_Rect ret = {
		.x = r->x,
		.y = r->y,
		.w = r->w,
		.h = r->h,
	};

	return ret;
}

bool is_color_equal(SDL_Color* one, SDL_Color* two) {
	return one->r == two->r
		&& one->g == two->g
		&& one->b == two->b
		&& one->a == two->a;
}

float random_float(float min, float max) {
	float scale = rand() / (float) RAND_MAX;
	return min + scale * (max - min);
}
