#ifndef TILEMAP_H
#define TILEMAP_H

#include "camera.h"
#include "tmx/tmx.h"

#include <stdbool.h>

#include <SDL.h>

/*
 * Contains data for a single tile.
 */
struct tile {
	int gid;    // The tile gid from the TMX file.
	SDL_Rect r; // The tile dimensions.
};

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

/*
 * Gets the tile data for the tile which is occupied at the given x and y position.
 */
struct tile tilemap_gettile(struct tilemap* tm, float x, float y);
void tilemap_draw_foreground(struct tilemap* tm, struct camera* cam, SDL_Renderer* r);
void tilemap_draw_background(struct tilemap* tm, struct camera* cam, SDL_Renderer* r);
void tilemap_handle_event(struct tilemap* tm, const SDL_Event* event);
void tilemap_getsize(const struct tilemap* tm, int* w, int* h);

#endif // TILEMAP_H
