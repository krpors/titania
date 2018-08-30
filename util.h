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

/*
 * This part defines a macro so we can print debugging lines, but only when
 * the NDEBUG macro is undefined (much like from <assert.h>. The output is
 * colored for now, and can be controlled via the `filename' and `function'
 * #defines.
 *
 * The macro debug_print only has an implementation when the project is built
 * using the `debug' target.
 */
#ifndef NDEBUG
#define filename "\x1b[34;1m%s\x1b[0m"
#define function "\x1b[33;1m%s()\x1b[0m"
#define debug_print(fmt, ...) \
            do { fprintf(stderr, filename ":%d:" function "    " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__); } while (0)
#else
#define debug_print(fmt, ...) (void)0;
#endif // NDEBUG

#endif // UTIL_H
