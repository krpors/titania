#include "player.h"

#include <SDL.h>

void player_init(struct player* p) {
	p->map = NULL;
	p->x  = 60;
	p->y  = 60;
	p->dx = 0.2f;
	p->dy = 0;
	p->w = 18;
	p->h = 18;
}

void player_left(struct player* p) {
	p->left = true;
	p->right = false;
}

void player_right(struct player* p) {
	p->left = false;
	p->right = true;
}

void player_stop(struct player* p) {
	p->left  = false;
	p->right = false;
}

static bool player_is_colliding(struct player* p) {
	return true;
}

void player_update(struct player* p) {

	float newx = p->x;

	if (p->left) {
		newx = p->x - p->dx;
	}

	if (p->right) {
		newx = p->x + p->dx;
	}

	p->x = newx;
}

void player_draw(struct player* p, SDL_Renderer* r) {
	SDL_Rect rekt = {
		.x = p->x,
		.y = p->y,
		.w = p->w,
		.h = p->h
	};

	SDL_SetRenderDrawColor(r, 200, 200, 200, 155);
	SDL_RenderFillRect(r, &rekt);
}
