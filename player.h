#ifndef PLAYER_H
#define PLAYER_H

#include "camera.h"
#include "util.h"

#include <stdbool.h>
#include <SDL.h>

static const float PLAYER_SPRITE_WIDTH  = 48;
static const float PLAYER_SPRITE_HEIGHT = 48;

static const float PLAYER_MIN_DX = 300.0f;
static const float PLAYER_MAX_DX = 400.0f;

static const float PLAYER_JUMP_VEL = 1000.0f;
static const float PLAYER_MAX_DY = 600.0f;

static const float GRAVITY = 3000.0f;

struct particle {
	float x;
	float y;
	float w;
	float h;

	float dy;

	int a;
	int life;
	int max_life;
};

/*
 * The particle list is a sort of specialized list specifically for particles
 * to calculate when the next particle should beemitted (using particle_time),
 * and which particle in the array to reset to the player's position
 * (particle_num). The implementation behaves as a sort of circular list. When
 * the end index is reached (len) the particle_num will be set to 0.
 */
struct particle_list {
	struct particle* p;
	size_t len;

	int particle_time; // to determine when to 'place' a particle.
	int particle_num;  // the particle index to use.
};


struct player {
	struct tilemap* map;
	struct bitmapfont* font;

	// The player x, y, w, h are actually the dimensions of the player's hitbox.
	// The drawing of the player sprite is done based on these values.
	float x;
	float y;
	float w;
	float h;

	// Velocities in the x and y directions.
	float dx;
	float dy;

	int facing_direction;

	float scale;
	int bx, by;
	int boop_life;

	bool jumping;
	bool can_jump;

	bool left;
	bool right;
	bool up;
	bool down;

	SDL_Texture* texture; // the player texture spritesheet

	struct anim* move_animation; // moving animation
	struct anim* rest_animation; // rest animation

	SDL_Rect rect_jump;
	SDL_Rect rect_fall;
	SDL_Rect rest; // when at rest.

	SDL_Rect rect_collision; // rectangle for collision purposes

	struct particle_list* particles;
};

struct player* player_create();
void player_free(struct player* p);
void player_left(struct player* p);
void player_right(struct player* p);
void player_jump(struct player* p);
void player_down(struct player* p);
void player_stop(struct player* p);

bool player_load_texture(struct player* p, SDL_Renderer* r, const char* path);

/*
 * Update the player position. The delta_time is the time in milliseconds
 * between frames.
 */
void player_update(struct player* p, float delta_time);

/*
 * Handle SDL events on the player.
 */
void player_handle_event(struct player* p, const SDL_Event* event);

/*
 * Draws the player on the screen using the renderer. The camera is used
 * to provide scrolling.
 */
void player_draw(const struct player* p, const struct camera* cam, SDL_Renderer* r);

#endif // PLAYER_H
