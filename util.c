#include "util.h"

#include <assert.h>
#include <stdbool.h>

#include <SDL.h>
#include <SDL_image.h>

extern SDL_Renderer* gRenderer;

bool is_color_equal(SDL_Color* one, SDL_Color* two) {
	return one->r == two->r
		&& one->g == two->g
		&& one->b == two->b
		&& one->a == two->a;
}

/*****************************************************************************/

// TODO: own file perhaps?
struct anim* anim_create(int frame_time) {
	struct anim* a = malloc(sizeof(struct anim));
	a->n = 0;
	a->curr = 0;
	a->frame_time = frame_time;
	a->counter = SDL_GetTicks();
	a->rectangles = malloc(1 * sizeof(SDL_Rect));
	return a;
}

void anim_free(struct anim* a) {
	free(a->rectangles);
	free(a);
}

void anim_add(struct anim* a, int x, int y, int w, int h) {
	SDL_Rect r = {x, y, w, h};
	a->rectangles[a->n] = r;
	a->n++;
	a->rectangles = realloc(a->rectangles, (a->n + 1) * sizeof(SDL_Rect));
}

void anim_reset(struct anim* a) {
	a->curr = 0;
}

void anim_next(struct anim* a) {
	int current_time = SDL_GetTicks();
	if (current_time > a->counter + a->frame_time) {
		a->curr++;
		// Make sure we stay in bounds. Repeatedly calling anim_next() will
		// result in a circular array behaviour: when next()ing on the last entry,
		// the next call will result result in the first element.
		if (a->curr > a->n - 1 || a->curr < 0) {
			a->curr = 0;
		}

		a->counter = current_time; 
	}
}

const SDL_Rect* anim_current(struct anim* a) {
	assert(a->curr >= 0);
	return &a->rectangles[a->curr];
}