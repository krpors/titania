#include "player.h"
#include "tilemap.h"

#include <assert.h>
#include <math.h>

#include <SDL.h>

void player_init(struct player* p) {
	p->map = NULL;
	p->x  = 120;
	p->y  = 70;
	p->dx = PLAYER_MIN_DX;
	p->dy = 200;
	p->w = 18;
	p->h = 18;

	p->falling = false;
	p->jumping = false;
}

void player_left(struct player* p) {
	p->left = true;
}

void player_right(struct player* p) {
	p->right = true;
}

void player_up(struct player* p) {
	p->jumping = true;
}

void player_down(struct player* p) {
	p->down = true;
}
void player_stop(struct player* p) {
	p->left  = false;
	p->right = false;
	p->down = false;
	p->jumping = false;
	p->dx = PLAYER_MIN_DX;
}

static bool player_is_colliding(struct player* p, float newx, float newy) {
	assert(p->map != NULL);

	// Get the tile coordinate of the top-left corner of the player's bounds.
	int tilex1 = floorf(newx / TILE_SIZE);
	int tiley1 = floorf(newy / TILE_SIZE);

	// Get the tile coordinate of the bottom-right corner of the player's bounds.
	int tilex2 = floorf((newx + p->w) / TILE_SIZE);
	int tiley2 = floorf((newy + p->h) / TILE_SIZE);

#if 0
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

void player_update(struct player* p, float delta_time) {
	// First we have to have to possible new positions, so declare
	// those, starting with our current x and y positions.
	float newx = p->x;
	float newy = p->y;

	if (p->left || p->right) {
		// increase the velocity of the player when moving, until
		// the maximum has been reached. This allows us to move
		// tiny amounts over the x-axis (for better precision).
		p->dx += ((PLAYER_MAX_DX + PLAYER_MIN_DX) / 2) * delta_time;
		p->dx = fminf(p->dx, PLAYER_MAX_DX);
	}

	// If we're moving left, calculate our possible new x position.
	if (p->left) {
		newx -= p->dx * delta_time;
	}

	// If we're moving right, calculate our possible new y position.
	if (p->right) {
		newx += p->dx * delta_time;
	}

	if (p->jumping && !p->falling) {
		printf("jmpz0r?\n");
		p->dy = -290.0f;
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
	p->dy += 1.0f;
	newy += p->dy * delta_time;
	printf("newy: %d\n", (int)newy);

	// Check if we are colliding over the y axis, using the
	// current x position, and the newly calculated y position.
	if (player_is_colliding(p, p->x, newy)) {
		p->falling = false;
		p->jumping = false;
		p->dy = 1.0f;
	} else {
		p->falling = true;
		p->y = newy;
	}
}

void player_handle_event(struct player* p, const SDL_Event* event) {
	if (event->type == SDL_KEYDOWN && event->key.repeat == 0) {
		switch (event->key.keysym.sym) {
		case SDLK_UP    : player_up(p)    ; break ;
		case SDLK_RIGHT : player_right(p) ; break ;
		case SDLK_DOWN  : player_down(p)  ; break ;
		case SDLK_LEFT  : player_left(p)  ; break ;
		}
	} else if (event->type == SDL_KEYUP && event->key.repeat == 0) {
		switch (event->key.keysym.sym) {
		case SDLK_UP    : // fallthrough
		case SDLK_RIGHT : //     v
		case SDLK_DOWN  : //     v
		case SDLK_LEFT  : player_stop(p); break;
		}
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
