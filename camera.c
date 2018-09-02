#include "camera.h"
#include "player.h"
#include "tilemap.h"

void camera_init(struct camera* cam, int winwidth, int winheight) {
	cam->winwidth = winwidth;
	cam->winheight = winheight;
}

void camera_update(struct camera* cam, const struct player* p, const struct tilemap* map) {
	// TODO: smooth lerping

	// XXX: somehow subtracting with -20 takes care of the extra blackness
	uint32_t mapwidth  = (map->map->width)  * map->tilewidth - 20;
	uint32_t mapheight = (map->map->height) * map->tileheight - 20;

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
