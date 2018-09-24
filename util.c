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

//#############################################################################
// Circular list implementation.
//#############################################################################

struct circular_list* circular_list_create() {
	struct circular_list* cl = calloc(1, sizeof(struct circular_list));
	return cl;
}

void circular_list_add(struct circular_list* cl, void* data) {
	assert(cl != NULL);
	assert(data != NULL);

	// Reallocate our data dynamic array, holding the size of the data (which
	// should be a pointer size), and assign the data to the index.
	cl->data = realloc(cl->data, ++cl->len * sizeof(data));
	cl->data[cl->len - 1] = data;
}

void* circular_list_next(struct circular_list* cl) {
	assert(cl != NULL);
	// The modulo makes sure we will 'loop' over the array.
	return cl->data[cl->current_index++ % cl->len];
}

void circular_list_free(struct circular_list* cl) {
	free(cl->data);
	cl->data = NULL;
	free(cl);
	cl = NULL;
}
