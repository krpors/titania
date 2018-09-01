#include "camera.h"
#include "player.h"
#include "tilemap.h"

void camera_init(struct camera* cam, int winwidth, int winheight) {
	cam->winwidth = winwidth;
	cam->winheight = winheight;
}

void camera_update(struct camera* cam, const struct player* p, const struct tilemap* map) {
	// TODO: smooth lerping

	// Map dimensions? We do a +1 to the width and height for whatever reason.
	// We also multiply by 2 since the tilesize isn't 32, but 64.
	uint32_t mapwidth  = (map->map->width  + 1) * map->map->tile_width  * 2;
	uint32_t mapheight = (map->map->height + 1) * map->map->tile_height * 2;

	uint32_t xmin = 0;
	uint32_t xmax = mapwidth - cam->winwidth + p->w;

	uint32_t ymin = 0;
	uint32_t ymax = mapheight - cam->winheight + p->h;

	// Increase the x viewport based on the player position, but keep
	// within bounds of the map. We do a float division to get a better
	// result for the max values.
	cam->x = p->x - ((float)cam->winwidth / 2.0);
	if (cam->x < xmin) {
		cam->x = xmin;
	}
	if (cam->x > xmax) {
		cam->x = xmax;
	}

	// Same, but this time over the y position.
	cam->y = p->y - ((float)cam->winheight / 2.0);
	if (cam->y < ymin) {
		cam->y = ymin;
	}
	if (cam->y > ymax) {
		cam->y = ymax;
	}
}
