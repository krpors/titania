#ifndef TILEMAP_H
#define TILEMAP_H

#include <stdbool.h>

enum tiletype {
	TILE_NONE = 0x00,
	TILE_MAP_EDGE = 0xff,
};

static const int TILE_SIZE = 64;

/*
 * The tilemap.
 */
struct tilemap {
	int* tiles;
	int len;
	int w;
	int h;

	struct spritesheet* sheet;
};

void tilemap_init(struct tilemap* m);
bool tilemap_read(struct tilemap* m, const char* path);
int  tilemap_get(const struct tilemap* m, int x, int y);
void tilemap_add(struct tilemap* m, int tileval);
void tilemap_free(struct tilemap* m);

#endif // TILEMAP_H
