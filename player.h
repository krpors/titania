#ifndef PLAYER_H
#define PLAYER_H

#include "camera.h"
#include "util.h"

#include <stdbool.h>
#include <SDL.h>

static const float PLAYER_MIN_DX = 50.0f;
static const float PLAYER_MAX_DX = 480.0f;

static const float PLAYER_JUMP_VEL = 1000.0f;
static const float PLAYER_MAX_DY = 600.0f;

static const float GRAVITY = 3000.0f;

void anim_test();

struct player {
	struct tilemap* map;
	struct bitmapfont* font;

	float x;
	float y;
	float dx;
	float dy;
	int w;
	int h;

	float scale;
	int bx, by;
	int boop_life;

	bool jumping;
	bool can_jump;

	bool left;
	bool right;
	bool up;
	bool down;

	float counter;

	struct anim* move_animation;
	SDL_Texture* texture;
	SDL_Rect rest;
};

void player_init(struct player* p);
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

void player_draw(const struct player* p, const struct camera* cam, SDL_Renderer* r);

#endif // PLAYER_H
