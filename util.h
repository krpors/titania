#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>

#include <SDL.h>

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
	do { fprintf(stderr, filename ":%d:" function "    " fmt, __FILE__, __LINE__, __func__, ## __VA_ARGS__); } while (0)
// Note: the double ## is to swallow the preceding comma in case the list is empty (gcc specific)
#else
#define debug_print(fmt, ...) (void)0;
#endif // NDEBUG

/*
 * Defines a point in the 2D cartesian plane.
 */
struct point {
	float x;
	float y;
};

/*
 * A rectangle. This one differs from the SDL_Rect in that
 * the members are floats instead of ints.
 */
struct rect {
	float x;
	float y;
	float w;
	float h;
};

SDL_Rect rect_to_SDLRect(const struct rect* r);

/*
 * Checks whether two SDL_Color structs are equal by comparing
 * the members r,g,b and a.
 */
bool is_color_equal(SDL_Color* one, SDL_Color* two);

float random_float(float min, float max);

struct circular_list_node {
	void* data;
	struct circular_list_node* next;
};

struct circular_list {
	struct circular_list_node* head;
	struct circular_list_node* curr;
	struct circular_list_node* tail;
};

struct circular_list* circular_list_create();
void circular_list_add(struct circular_list* cl, void* data);
void* circular_list_next(struct circular_list* cl);
void circular_list_free(struct circular_list* cl);

#endif // UTIL_H
