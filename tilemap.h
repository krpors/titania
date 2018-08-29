#ifndef TILEMAP_H
#define TILEMAP_H

#include <stdbool.h>

#include <SDL.h>

#include "tmx/tmx.h"

static const int TILE_SIZE = 64;

/*
 * The tilemap.
 */
struct tilemap {
	tmx_map* map;
	tmx_layer* collision_layer;
	SDL_Renderer* renderer;
	SDL_Texture* image;
};


bool tilemap_load(struct tilemap* tm, const char* path);
void tilemap_free(struct tilemap* tm);
int tilemap_tileat(struct tilemap* tm, int x, int y);
void tilemap_draw(struct tilemap* tm, SDL_Renderer* r);

#endif // TILEMAP_H
