#include "gfx.h"
#include "util.h"

//#############################################################################
// Animation types.
//#############################################################################

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
		a->curr = a->curr % a->n; // circular buffer behaviour
		a->counter = current_time;
	}
}

const SDL_Rect* anim_current(struct anim* a) {
	assert(a->curr >= 0);
	return &a->rectangles[a->curr];
}