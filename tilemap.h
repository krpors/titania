#ifndef TILEMAP_H
#define TILEMAP_H

#include <stdbool.h>

static const int TILE_NONE = 0x00;
static const int TILE_BRICK = 0xff;

enum tiletype {
	NONE = 0x00,
	BRICK = 0xff,
};

/*
 * The tilemap.
 */
struct tilemap {
	int* tiles;
	int len;
	int w;
	int h;
};

void tilemap_init(struct tilemap* m);
bool tilemap_read(struct tilemap* m, const char* path);
int  tilemap_get(const struct tilemap* m, int x, int y);
void tilemap_add(struct tilemap* m, int tileval);
void tilemap_free(struct tilemap* m);

#endif // TILEMAP_H
