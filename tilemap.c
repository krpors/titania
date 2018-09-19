#include "camera.h"
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

static void draw_layer(struct tilemap* tm, struct camera* cam, SDL_Renderer* r, const tmx_layer* layer) {
	tmx_map* map = tm->map;
	tmx_tileset* tileset = NULL;

	SDL_Texture* tileset_texture = NULL;
	SDL_Rect src_rect; // source rectangle for the texture
	SDL_Rect dst_rect; // target rectangle, where the place the src_rect.


	uint8_t opacity = layer->opacity * 255;
	uintmax_t gid;
	for (uintmax_t i = 0; i < map->height; i++) {
		for (uintmax_t j = 0; j < map->width; j++) {
			SDL_RendererFlip flip = SDL_FLIP_NONE;
			double rotate = 0;

			int idx = i * map->width + j;
			gid = layer->content.gids[idx];

			bool flipped_horizontally = (gid & TMX_FLIPPED_HORIZONTALLY);
			bool flipped_vertically   = (gid & TMX_FLIPPED_VERTICALLY);
			bool flipped_diagonally   = (gid & TMX_FLIPPED_DIAGONALLY);

			// XXX: what a clusterfuck this is. What am I missing here?
			// This part just screams "I don't get these flags" and begs
			// for a bit of refactoring.
			if (flipped_diagonally) {
				if (flipped_horizontally && flipped_vertically) {
					rotate = 90;
					flip |= SDL_FLIP_HORIZONTAL;
				} else if (flipped_horizontally) {
					rotate = 90;
				} else if (flipped_vertically) {
					rotate = -90;
				} else {
					rotate = 90;
					flip |= SDL_FLIP_VERTICAL;
				}
			} else {
				if (flipped_horizontally) {
					flip |= SDL_FLIP_HORIZONTAL;
				}
				if (flipped_vertically) {
					flip |= SDL_FLIP_VERTICAL;
				}
			}

			// Always clear the motherflippin' bits :)
			gid &= TMX_FLIP_BITS_REMOVAL;
			if (map->tiles[gid] == NULL) {
				continue;
			}

			tileset = map->tiles[gid]->tileset;
			tileset_texture = tileset->image->resource_image;

			src_rect.x = map->tiles[gid]->ul_x;
			src_rect.y = map->tiles[gid]->ul_y;
			src_rect.w = tileset->tile_width;
			src_rect.h = tileset->tile_height;

			dst_rect.x = j * tm->tilewidth - cam->x;
			dst_rect.y = i * tm->tileheight - cam->y;
			dst_rect.w = tm->tilewidth;
			dst_rect.h = tm->tileheight;


			SDL_SetTextureAlphaMod(tileset_texture, opacity);
			SDL_RenderCopyEx(r, tileset_texture, &src_rect, &dst_rect, rotate, NULL, flip);

			if (j == 5 && i == 5) {
				SDL_SetRenderDrawColor(r, 0xff, 0xff, 0xff, 0xff);
				SDL_RenderDrawRect(r, &dst_rect);
			}
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

struct tilemap* tilemap_create(const char* path) {
	struct tilemap* tm = malloc(sizeof(struct tilemap));

	tm->map = tmx_load(path);
	if (tm->map == NULL) {
		tmx_perror("tmx_load");
		return NULL;
	}

	debug_print("Tilemap is loaded: width = %d, height = %d\n", tm->map->width, tm->map->height);

	tm->collision_layer = find_collision_layer(tm->map);
	if (tm->collision_layer == NULL) {
		fprintf(stderr, "Could not find collision layer!?\n");
		tilemap_free(tm);
		return NULL;
	}

	tm->tilewidth = 0;
	tm->tilewidth = 0;

	return tm;
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

struct tile tilemap_gettile(struct tilemap* tm, float x, float y) {
	// Convert the x and y variadic coordinates to tile indexes.
	int tilex = floorf(x / tm->tilewidth);
	int tiley = floorf(y / tm->tileheight);

	struct tile t = {
		.r = {
			.x = tilex * tm->tilewidth,
			.y = tiley * tm->tileheight,
			.w = tm->tilewidth,
			.h = tm->tileheight
		},
		.gid = 1,
	};

	if (tilex < 0 || tilex >= (int)tm->map->width || tiley < 0 || tiley >= (int)tm->map->height) {
		return t;
	}

	tmx_map* map = tm->map;

	int idx = y * map->width + x;
	t.gid = tm->collision_layer->content.gids[idx];
	return t;
}

void tilemap_draw_foreground(struct tilemap* tm, struct camera* cam, SDL_Renderer* r) {
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
			draw_layer(tm, cam, r, layer);
		}
	}
}

void tilemap_draw_background(struct tilemap* tm, struct camera* cam, SDL_Renderer* r) {
	tmx_map* map = tm->map;
	// Iterate over every layer until we hit the 'Main' layer. The background
	// is everything up until that 'Main' layer (but not inclusive).
	for(tmx_layer* layer = map->ly_head; layer != NULL; layer = layer->next) {
		if (strcmp(LAYER_COLLISION, layer->name) == 0) {
			// Do not draw our collision layer, that would be stupid.
			continue;
		}

		draw_layer(tm, cam, r, layer);
		// We draw up until the 'Main' layer. That are our background layers.
		if (strcmp(LAYER_MAIN, layer->name) == 0) {
			break;
		}
	}
}

void tilemap_handle_event(struct tilemap* tm, const SDL_Event* event) {
	// for some quick debugging.
	if (event->type == SDL_KEYDOWN) {
		switch (event->key.keysym.sym) {
		case SDLK_KP_PLUS:
			tm->tilewidth++;
			tm->tileheight++;
			break;
		case SDLK_KP_MINUS:
			tm->tilewidth--;
			tm->tileheight--;
			break;
		}
	}
}

void tilemap_getsize(const struct tilemap* tm, int* w, int* h) {
	*w = tm->tilewidth  * tm->map->width;
	*h = tm->tileheight * tm->map->height;
}
