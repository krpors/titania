#include "tilemap.h"
#include "tmx/tmx.h"
#include "util.h"

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL_image.h>

static const char* LAYER_MAIN      = "Main";
static const char* LAYER_COLLISION = "Collision";

extern int tilewidth;
extern int tileheight;

static void draw_layer(SDL_Renderer* r, const tmx_map* map, const tmx_layer* layer) {
	tmx_tileset* tileset = NULL;

	SDL_Texture* tileset_texture = NULL;
	SDL_Rect src_rect; // source rectangle for the texture
	SDL_Rect dst_rect; // target rectangle, where the place the src_rect.

	uint8_t opacity = layer->opacity * 255;
	uintmax_t gid;
	for (uintmax_t i = 0; i < map->height; i++) {
		for (uintmax_t j = 0; j < map->width; j++) {
			int idx = i * map->width + j;
			gid = layer->content.gids[idx];
			if (map->tiles[gid] == NULL) {
				continue;
			}

			tileset = map->tiles[gid]->tileset;
			tileset_texture = tileset->image->resource_image;

			src_rect.x = map->tiles[gid]->ul_x;
			src_rect.y = map->tiles[gid]->ul_y;
			src_rect.w = tileset->tile_width;
			src_rect.h = tileset->tile_height;

			dst_rect.x = j * tilewidth;
			dst_rect.y = i * tileheight;
			dst_rect.w = tilewidth;
			dst_rect.h = tileheight;

			SDL_SetTextureAlphaMod(tileset_texture, opacity);
			SDL_RenderCopy(r, tileset_texture, &src_rect, &dst_rect);
		}
	}
}

/**
 * Finds the layer namd "Collision". Every set gid in that map will
 * count as a collidable tile for the player to check with.
 */
static tmx_layer* find_collision_layer(const tmx_map* map) {
	for (tmx_layer* layer = map->ly_head; layer != NULL; layer = layer->next) {
		if (strcmp(LAYER_COLLISION, layer->name) == 0) {
			return layer;
		}
	}

	return NULL;
}


bool tilemap_load(struct tilemap* tm, const char* path) {
	tm->map = tmx_load(path);
	if (tm->map == NULL) {
		tmx_perror("tmx_load");
		return false;
	}

	debug_print("Tilemap is loaded: width = %d, height = %d\n", tm->map->width, tm->map->height);

	tm->collision_layer = find_collision_layer(tm->map);
	if (tm->collision_layer == NULL) {
		fprintf(stderr, "Could not find collision layer!?\n");
		tilemap_free(tm);
		return false;
	}

	return true;
}

void tilemap_free(struct tilemap* tm) {
	tmx_map_free(tm->map);
	tm->map = NULL;
}

int tilemap_tileat(struct tilemap* tm, int x, int y) {
	if (x < 0 || x >= (int)tm->map->width || y < 0 || y >= (int)tm->map->height) {
		return 1;
	}

	tmx_map* map = tm->map;

	uintmax_t gid;
	int idx = y * map->width + x;
	gid = tm->collision_layer->content.gids[idx];
	return gid;
}

void tilemap_draw_foreground(struct tilemap* tm, SDL_Renderer* r) {
	tmx_map* map = tm->map;
	bool start_drawing = false;
	// Iterate over every layer until we hit the 'Main' layer. The foreground
	// is everything including 'Main', and all layers after (on top of it).
	for(tmx_layer* layer = map->ly_head; layer != NULL; layer = layer->next) {
		if (strcmp(LAYER_MAIN, layer->name) == 0) {
			// So, we found our 'Main' layer. We can start drawing layers.
			start_drawing = true;
		}

		if (start_drawing) {
			draw_layer(r, map, layer);
		}
	}
}

void tilemap_draw_background(struct tilemap* tm, SDL_Renderer* r) {
	tmx_map* map = tm->map;
	// Iterate over every layer until we hit the 'Main' layer. The background
	// is everything up until that 'Main' layer (but not inclusive).
	for(tmx_layer* layer = map->ly_head; layer != NULL; layer = layer->next) {
		if (strcmp(LAYER_COLLISION, layer->name) == 0) {
			// Do not draw our collision layer, that would be stupid.
			continue;
		}

		draw_layer(r, map, layer);
		// We draw up until the 'Main' layer. That are our background layers.
		if (strcmp(LAYER_MAIN, layer->name) == 0) {
			break;
		}
	}
}
