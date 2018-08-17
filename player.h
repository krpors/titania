#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include <SDL.h>

struct player {
	struct tilemap* map;

	float x;
	float y;
	float dx;
	float dy;
	int w;
	int h;

	float g;
	bool falling;

	bool left;
	bool right;
	bool up;
	bool down;
};

void player_init(struct player* p);
void player_left(struct player* p);
void player_right(struct player* p);
void player_up(struct player* p);
void player_down(struct player* p);
void player_stop(struct player* p);
void player_update(struct player* p);
void player_draw(struct player* p, SDL_Renderer* r);

#endif // PLAYER_H
