#include "util.h"

#include <stdbool.h>

#include <SDL.h>

bool is_color_equal(SDL_Color* one, SDL_Color* two) {
	return one->r == two->r
		&& one->g == two->g
		&& one->b == two->b
		&& one->a == two->a;
}