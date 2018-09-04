#ifndef TILEMAP_H
#define TILEMAP_H

#include "camera.h"
#include "tmx/tmx.h"

#include <stdbool.h>

#include <SDL.h>

static const int TILE_SIZE = 64;

/*
 * The tilemap.
 */
struct tilemap {
	tmx_map* map;
	tmx_layer* collision_layer;

	float tilewidth;
	float tileheight;
};


bool tilemap_load(struct tilemap* tm, const char* path);
void tilemap_free(struct tilemap* tm);
int tilemap_tileat(struct tilemap* tm, int x, int y);
void tilemap_draw_foreground(struct tilemap* tm, struct camera* cam, SDL_Renderer* r);
void tilemap_draw_background(struct tilemap* tm, struct camera* cam, SDL_Renderer* r);
void tilemap_handle_event(struct tilemap* tm, const SDL_Event* event);
void tilemap_getsize(const struct tilemap* tm, int* w, int* h);

#endif // TILEMAP_H
