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

#ifndef NDEBUG
#define debug_print(fmt, ...) \
            do { fprintf(stderr, "\x1b[34;1m%s\x1b[0m:%d:\x1b[32m%s()\x1b[0m    " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__); } while (0)
#else
#define debug_print(fmt, ...) (void)0;
#endif // NDEBUG

#endif // UTIL_H
