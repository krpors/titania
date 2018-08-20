#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>

#include <SDL.h>

struct point {
	float x;
	float y;
};

/*
 * Checks whether two SDL_Color structs are equal by comparing
 * the members r,g,b and a.
 */
bool is_color_equal(SDL_Color* one, SDL_Color* two);

#endif // UTIL_H
