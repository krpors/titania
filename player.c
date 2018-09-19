#include "camera.h"
#include "player.h"
#include "bitmapfont.h"
#include "tilemap.h"

#include <assert.h>
#include <math.h>

#include <SDL.h>
#include <SDL_image.h>

//#############################################################################
// Private functions.
//#############################################################################


/*
 * Particle related functions. Currently defined static since they are
 * only applicable for the player currently. We'll see where this should go.
 */

static struct particle_list* particle_list_create(void) {
	struct particle_list* l = calloc(1, sizeof(struct particle_list));
	l->len = 20;
	l->p = calloc(l->len, sizeof(struct particle));
	for (size_t i = 0; i < l->len; i++) {
		l->p[i].w = 3;
		l->p[i].h = 3;
		l->p[i].a = 255;
		l->p[i].life = 100;
		l->p[i].max_life = 50;
	}
	return l;
}

static void particle_list_free(struct particle_list* list) {
	free(list->p);
	free(list);
}

/*
 * This function can be called repeatedly to check if "it's time" to place yet
 * another particle on the player's position. If it's the case, a particle with
 * the index of particle_num will be initialized to the player's position.
 */
static void particle_list_calc_frame(struct particle_list* list, const struct player* p) {
	int current_time = SDL_GetTicks();
	if (current_time > list->particle_time + 25) {
		struct particle* part = &list->p[list->particle_num];
		part->x = random_float(p->x - 2, p->x + 2);
		part->y = p->y + 5;
		part->w = part->h = random_float(2, 5);
		part->dy = random_float(-110, -100);
		part->life = 20;
		part->a = 255;

		list->particle_num++;
		if (list->particle_num >= (int)list->len) {
			list->particle_num = 0;
		}


		list->particle_time = current_time;
	}
}

/*
 * Updates the particle list every frame.
 */
static void particle_list_update(struct particle_list* list, float delta_time) {
	for (size_t i = 0; i < list->len; i++) {
		struct particle* part = &list->p[i];
		// Decrease the life of the particle and change the alpha.
		part->life--;
		// part->a = (float)part->life / (float)part->max_life * 255;
		part->y += part->dy * delta_time;
		part->dy += 1000.0f * delta_time;
	}
}

static void particle_list_draw(const struct particle_list* list, const struct camera* cam, SDL_Renderer* r) {
	for (size_t i = 0; i < list->len; i++) {
		const struct particle* current_particle = &list->p[i];
		SDL_SetRenderDrawColor(r, 255, 255, 255, current_particle->a);
		if (current_particle->life < 0) {
			continue;
		}
		// TODO: randomize sizes, directions etc.
		SDL_Rect rector = {
			.x = current_particle->x - cam->x,
			.y = current_particle->y - cam->y + 30,
			.w = current_particle->w,
			.h = current_particle->h,
		};

		SDL_RenderFillRect(r, &rector);
	}
}

//#############################################################################
// Public functions.
//#############################################################################

/*
 * All player struct related functions.
 */

struct player* player_create() {
	struct player* p = calloc(1, sizeof(struct player));

	p->map = NULL;

	p->x = 120;
	p->y = 70;
	p->w = 22;
	p->h = 36;

	p->dx = PLAYER_MIN_DX;
	p->dy = 0.0f;

	p->facing_direction = 1; // right

	p->scale = 1.0f;
	p->boop_life = 0;
	p->jumping = false;
	p->can_jump = false;

	p->move_animation = anim_create(30);
	p->rest_animation = anim_create(80);

	p->rect_jump.x = 0;
	p->rect_jump.y = 32;
	p->rect_jump.w = 16;
	p->rect_jump.h = 16;

	p->rect_fall.x = 16;
	p->rect_fall.y = 32;
	p->rect_fall.w = 16;
	p->rect_fall.h = 16;

	p->particles = particle_list_create();

	return p;
}

void player_free(struct player* p) {
	anim_free(p->move_animation);
	anim_free(p->rest_animation);
	particle_list_free(p->particles);
	free(p);
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

bool player_load_texture(struct player* p, SDL_Renderer* r, const char* path) {
	p->texture = IMG_LoadTexture(r, path);
	if (p->texture == NULL) {
		fprintf(stderr, "Failed to load player texture!");
		return false;
	}

	anim_add(p->move_animation, 16 * 0, 16, 16, 16);
	anim_add(p->move_animation, 16 * 1, 16, 16, 16);
	anim_add(p->move_animation, 16 * 2, 16, 16, 16);
	anim_add(p->move_animation, 16 * 3, 16, 16, 16);
	anim_add(p->move_animation, 16 * 4, 16, 16, 16);
	anim_add(p->move_animation, 16 * 5, 16, 16, 16);

	anim_add(p->rest_animation, 16 * 0, 0, 16, 16);
	anim_add(p->rest_animation, 16 * 1, 0, 16, 16);
	anim_add(p->rest_animation, 16 * 2, 0, 16, 16);
	anim_add(p->rest_animation, 16 * 3, 0, 16, 16);

	p->rest.x = 0;
	p->rest.y = 0;
	p->rest.w = 16;
	p->rest.h = 16;

	return true;
}

static bool player_is_colliding(const struct player* p, float newx, float newy) {
	assert(p->map != NULL);

	// Get the tile coordinate of the top-left corner of the player's bounds.
	int tilex1 = floorf(newx / p->map->tilewidth);
	int tiley1 = floorf(newy / p->map->tileheight);

	// Get the tile coordinate of the bottom-right corner of the player's bounds.
	int tilex2 = floorf((newx + p->w) / p->map->tilewidth);
	int tiley2 = floorf((newy + p->h) / p->map->tileheight);

	// Now iterate over (tilex1, tiley1) - (tilex2, tiley2), and check
	// whether the tile is collidable.
	for (int x = tilex1; x <= tilex2; x++) {
		for (int y = tiley1; y <= tiley2; y++) {
			int tile = tilemap_tileat(p->map, x, y);
			if (tile >= 1) {
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

		anim_next(p->move_animation);

		if (p->can_jump)
			particle_list_calc_frame(p->particles, p);
	}

	particle_list_update(p->particles, delta_time);

	// If we're moving left, calculate our possible new x position.
	// Decrease the life of the particle and change the alpha.
	if (p->left) {
		newx -= p->dx * delta_time;
		p->facing_direction = -1;
	}

	// If we're moving right, calculate our possible new y position.
	if (p->right) {
		newx += p->dx * delta_time;
		p->facing_direction = 1;
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

	if (p->boop_life > 0) {
		p->boop_life -= (delta_time * 500.0f);
		p->scale += 1.0f * delta_time;
	} else {
		p->scale = 1.0f;
		p->boop_life = 0;
	}

	bool colliding = player_is_colliding(p, p->x, newy);
	if (colliding) {
		if (p->dy < 0.0f) {
			// Collision with the ceiling, since our dy is negative.
			debug_print("Collided with the ceiling at %.1f\n", newy);
			p->bx = p->x + (p->w / 4);
			p->by = p->y - (p->h / 4);
			p->jumping = false;
			p->boop_life = 255;
		} else if (p->dy >= 0.0f) {
			// Collision with the ground, we can jump again.
			p->jumping = false;
			p->can_jump = true;
			// Align ourselves to the top of the tile we bumped into. If we don't
			// do something like this, we get weird animation due to the gravity
			// constantly wanting to pull us down. Explain this better for future
			// self, I suppose...
			struct tile tilehit = tilemap_gettile(p->map, p->x, newy + p->h);
			// FIXME: NASTY HACK! Subtract with 0.001!
			p->y = tilehit.r.y - p->h - 0.001;

			if (p->dy > 1000.0f) {
				debug_print("Hit the ground with a force of %.1f\n", p->dy);
			}
		}
		// Whether we collided with the ceiling or the bottom,
		// reset our y velocity to 0.
		p->dy = 0.0f;
	} else {
		// We are not colliding with anything over the y-axis,j
		// so update our actual position to the new y coordinate.
		p->y = newy;
	}

	if (!p->left && !p->right && p->dy == 0.0f) {
		anim_next(p->rest_animation);
	}

	// Update the collision rectangle to the new player position.
	p->rect_collision.x = newx + 12;
	p->rect_collision.y = newy + 5;
	p->rect_collision.w = 25;
	p->rect_collision.h = 38;
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

void player_draw(const struct player* p, const struct camera* cam, SDL_Renderer* r) {
	// The position of the sprite differs from the actual x,y,w,h position
	// from the player, since that defines our hitbox (with the world and other
	// entities such as items, enemies, etc.).
	const SDL_Rect rect_sprite = {
		.x = p->x - 15 - cam->x,
		.y = p->y - 10 - cam->y,
		.w = PLAYER_SPRITE_WIDTH,
		.h = PLAYER_SPRITE_HEIGHT,
	};

	if (p->boop_life > 0) {
		float scale = p->scale;
		SDL_RenderSetScale(r, scale, scale);
		SDL_SetTextureAlphaMod(p->font->texture, p->boop_life);
		bitmapfont_renderf(p->font, (p->bx - cam->x) / scale, (p->by - cam->y) / scale, "Boop!!!");
		SDL_SetTextureAlphaMod(p->font->texture, 0xff);
		SDL_RenderSetScale(r, 1.0f, 1.0f);
	}

	particle_list_draw(p->particles, cam, r);

	SDL_SetRenderDrawColor(r, 200, 200, 200, 255);

	SDL_RendererFlip flip = SDL_FLIP_NONE;
	if (p->facing_direction == -1) {
		flip = SDL_FLIP_HORIZONTAL;
	}

	const SDL_Rect* rect = NULL;

	if (p->dy < 0.0f) {
		// jumping animation.
		rect = &p->rect_jump;
	} else if (p->dy > 0.0f) {
		// falling
		rect = &p->rect_fall;
	} else if ((p->left || p->right) && p->dy == 0.0f) {
		rect = anim_current(p->move_animation);
	} else {
		rect = anim_current(p->rest_animation);
	}

	SDL_RenderCopyEx(r, p->texture, rect, &rect_sprite, 0, NULL, flip);

/* 	const SDL_Rect rect_hitbox = {
		.x = p->x - cam->x,
		.y = p->y - cam->y,
		.w = p->w,
		.h = p->h,
	};
	SDL_SetRenderDrawColor(r, 0, 255, 0, 100);
	SDL_RenderFillRect(r, &rect_hitbox); */

}
