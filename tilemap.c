#include "tilemap.h"
#include "tmx/tmx.h"

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL_image.h>

extern int tilewidth;
extern int tileheight;

static void draw_all_layers(SDL_Renderer* r, const tmx_map* map) {
	tmx_layer* layer = map->ly_head;
	int layer_num = 0;

	tmx_tileset* tileset = NULL;

	SDL_Texture* tileset_texture = NULL;
	SDL_Rect src_rect; // source rectangle for the texture
	SDL_Rect dst_rect; // target rectangle, where the place the src_rect.

	while (layer) {
		tmx_property* property = tmx_get_property(layer->properties, "collision");
		if (property == NULL) {
			// if non-null, it contains our collision property, marking it as a
			// collidable layer. Do not draw, but use it for the physics part.

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

		layer = layer->next;
		layer_num++;
	}
}

static tmx_layer* find_collision_layer(const tmx_map* map) {
	tmx_layer* layer = map->ly_head;

	while (layer) {
		tmx_property* property = tmx_get_property(layer->properties, "collision");
		if (property != NULL) {
			return layer;
		}
		layer = layer->next;
	}

	return NULL;
}


bool tilemap_load(struct tilemap* tm, const char* path) {
	tm->map = tmx_load(path);
	if (tm->map == NULL) {
		tmx_perror("tmx_load");
		return false;
	}

	printf("Map is loaded. w = %d, h = %d\n", tm->map->width, tm->map->height);

	tm->collision_layer = find_collision_layer(tm->map);
	if (tm->collision_layer == NULL) {
		fprintf(stderr, "Could not find collision layer!?\n");
		return false;
	}

	return true;
}

void tilemap_free(struct tilemap* tm) {
	tmx_map_free(tm->map);
	tm->map = NULL;

	// TODO: free resource_image from tmx_tileset/tmx_image.
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

void tilemap_draw(struct tilemap* tm, SDL_Renderer* r) {
	(void)tm;
	(void)r;
	draw_all_layers(r, tm->map);
}
