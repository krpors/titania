#include "player.h"
#include "bitmapfont.h"
#include "tilemap.h"

#include <assert.h>
#include <math.h>

#include <SDL.h>

void player_init(struct player* p) {
	p->map = NULL;
	p->x  = 120;
	p->y  = 70;
	p->dx = PLAYER_MIN_DX;
	p->dy = 0.0f;
	p->w = 18;
	p->h = 18;

	p->scale = 1.0f;
	p->boop_life = 0;
	p->jumping = false;
	p->can_jump = false;
}

void player_left(struct player* p) {
	p->left = true;
}

void player_right(struct player* p) {
	p->right = true;
}

void player_jump(struct player* p) {
	if (p->can_jump) {
		p->jumping = true;
	}
}

void player_down(struct player* p) {
	p->down = true;
}
void player_stop(struct player* p) {
	p->left  = false;
	p->right = false;
	p->down = false;
	p->dx = PLAYER_MIN_DX;
}

// TODO: stop using globals... ?
extern int tilewidth;
extern int tileheight;

static bool player_is_colliding(struct player* p, float newx, float newy) {
	assert(p->map != NULL);

	// Get the tile coordinate of the top-left corner of the player's bounds.
	int tilex1 = floorf(newx / tilewidth);
	int tiley1 = floorf(newy / tileheight);

	// Get the tile coordinate of the bottom-right corner of the player's bounds.
	int tilex2 = floorf((newx + p->w) / tilewidth);
	int tiley2 = floorf((newy + p->h) / tilewidth);

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

	if (p->jumping && p->can_jump && p->dy <= 0.0f) {
		// If we pressed the jump button, and we can actually jump (i.e.
		// we are touching the ground), set our velocity to negative so
		// we actually up. We also check our dy to see if we are not falling.
		p->dy += -PLAYER_JUMP_VEL;
		p->can_jump = false;
	}

	if (!p->jumping && p->dy < -200.0f) {
		// If we prematurely released the jump button, stop
		// accelerating upwards so we can control the height
		// of our jumps.
		p->dy = 0.0f;
	}


	// If we are moving left, or right, and if we are NOT colliding
	// based on the new x position (but the same y position - this is
	// important!), update our x position with the new x position.
	if (p->left || p->right) {
		if (!player_is_colliding(p, newx, p->y)) {
			p->x = newx;
		}
	}

	// Always apply some force downwards. Does not matter when we are
	// jumping, or falling, or standing still...
	p->dy += GRAVITY * delta_time;
	newy += p->dy * delta_time;

	//printf("Probable new y: %f\n", newy);

	if (p->boop_life > 0) {
		p->boop_life -= (delta_time * 1000.0f);
		p->scale += 1.0f * delta_time;
	} else {
		p->scale = 1.0f;
		p->boop_life = 0;
	}

	bool colliding = player_is_colliding(p, p->x, newy);
	if (colliding) {
		if (p->dy < 0.0f) {
			// Collision with the ceiling, since our dy is negative.
			printf("Collided with the ceiling\n");
			p->bx = p->x + (p->w / 4);
			p->by = p->y - (p->h / 4);
			p->jumping = false;
			p->boop_life = 255;
		} else if (p->dy >= 0.0f) {
			// Collision with the ground, we can jump again.
			p->jumping = false;
			p->can_jump = true;
			if (p->dy > 1000.0f) {
				printf("Ouch!\n");
			}
		}
		// Whether we collided with the ceiling or the bottom,
		// reset our y velocity to 0.
		p->dy = 0.0f;
	} else {
		// We are not colliding with anything over the y-axis,
		// so update our actual position to the new y coordinate.
		p->y = newy;
	}
}

void player_handle_event(struct player* p, const SDL_Event* event) {
	if (event->type == SDL_KEYDOWN && event->key.repeat == 0) {
		switch (event->key.keysym.sym) {
		case SDLK_UP    : player_jump(p)  ; break ;
		case SDLK_RIGHT : player_right(p) ; break ;
		case SDLK_DOWN  : player_down(p)  ; break ;
		case SDLK_LEFT  : player_left(p)  ; break ;
		}
	} else if (event->type == SDL_KEYUP && event->key.repeat == 0) {
		switch (event->key.keysym.sym) {
		case SDLK_UP    : p->jumping = false; break;
		case SDLK_RIGHT : // fallthrough
		case SDLK_DOWN  : // fallthrough
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

	if (p->boop_life > 0) {
		float scale = p->scale;
		SDL_RenderSetScale(r, scale, scale);
		SDL_SetTextureAlphaMod(p->font->texture, p->boop_life);
		bitmapfont_renderf(p->font, p->bx / scale, p->by / scale, "Boop!!!");
		SDL_SetTextureAlphaMod(p->font->texture, 0xff);
		SDL_RenderSetScale(r, 1.0f, 1.0f);
	}

	SDL_SetRenderDrawColor(r, 200, 200, 200, 155);
	SDL_RenderFillRect(r, &rekt);
}
