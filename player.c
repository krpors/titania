#include "player.h"
#include "tilemap.h"

#include <assert.h>
#include <math.h>

#include <SDL.h>

void player_init(struct player* p) {
	p->map = NULL;
	p->x  = 70;
	p->y  = 70;
	p->dx = 0.2f;
	p->dy = 0.2f;
	p->w = 18;
	p->h = 18;

	p->g = 0.0f;
	p->falling = false;
}

void player_left(struct player* p) {
	p->left = true;
}

void player_right(struct player* p) {
	p->right = true;
}

void player_up(struct player* p) {
	p->up = true;
}

void player_down(struct player* p) {
	p->down = true;
}
void player_stop(struct player* p) {
	p->left  = false;
	p->right = false;
	p->down = false;
	p->up = false;
}

static bool player_is_colliding(struct player* p, float newx, float newy) {
	assert(p->map != NULL);

	// Get the tile coordinate of the top-left corner of the player's bounds.
	int tilex1 = floorf(newx / TILE_SIZE);
	int tiley1 = floorf(newy / TILE_SIZE);

	// Get the tile coordinate of the bottom-right corner of the player's bounds.
	int tilex2 = floorf((newx + p->w) / TILE_SIZE);
	int tiley2 = floorf((newy + p->h) / TILE_SIZE);

#ifndef NDEBUG
	printf("%s:%d %s()\n", __FILE__, __LINE__, __func__);
	printf("\tPlayer new position will be: (%3.0f,%3.0f)\n", newx, newy);
	printf("\tOccupied tiles: (%2d,%2d)-(%2d,%2d)\n", tilex1, tiley1, tilex2, tiley2);
#endif

	// Now iterate over (tilex1, tiley1) - (tilex2, tiley2), and check
	// whether the tile is collidable.
	for (int x = tilex1; x <= tilex2; x++) {
		for (int y = tiley1; y <= tiley2; y++) {
			int tile = tilemap_get(p->map, x, y);
			if (tile > TILE_NONE) {
				return true;
			}
		}
	}

	return false;
}

void player_update(struct player* p) {
	// First we have to have to possible new positions, so declare
	// those, starting with our current x and y positions.
	float newx = p->x;
	float newy = p->y;

	// If we're moving left, calculate our possible new x position.
	if (p->left) {
		newx = p->x - p->dx;
	}

	// If we're moving right, calculate our possible new y position.
	if (p->right) {
		newx = p->x + p->dx;
	}

	if (p->up) {
		newy = p->y - p->dy;
	}

	// If we are moving left, or right, and if we are NOT colliding
	// based on the new x position (but the same y position - this is
	// important!), update our x position with the new x position.
	if (p->left || p->right) {
		if (!player_is_colliding(p, newx, p->y)) {
			p->x = newx;
		}
	}

	// always exert some downward force (e.g. graviteh).
	p->g += 0.0001f;
	newy += p->g;

	// Check if we are colliding over the y axis, using the
	// current x position, and the newly calculated y position.
	if (player_is_colliding(p, p->x, newy)) {
		p->g = 0.0f;
		p->falling = false;
	} else {
		p->falling = true;
		p->y = newy;
	}
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
