#include "camera.h"
#include "player.h"
#include "tilemap.h"

struct camera* camera_create(int winwidth, int winheight) {
	debug_print("Camera initializing with window width: %d, height: %d\n", winwidth, winheight);
	struct camera* cam = malloc(sizeof(struct camera));
	cam->winwidth = winwidth;
	cam->winheight = winheight;
	return cam;
}

void camera_update(struct camera* cam, const struct player* p, const struct tilemap* map) {
	// TODO: smooth lerping

	uint32_t mapwidth  = (map->map->width)  * map->tilewidth - p->w;
	uint32_t mapheight = (map->map->height) * map->tileheight - p->h;

	uint32_t xmin = 0;
	uint32_t xmax = mapwidth - cam->winwidth + p->w;

	uint32_t ymin = 0;
	uint32_t ymax = mapheight - cam->winheight + p->h;

	// Increase the x viewport based on the player position, but keep
	// within bounds of the map. We do a float division to get a better
	// result for the max values.
	cam->x = p->x - ((float)cam->winwidth / 2.0);
	cam->x = fmin(fmax(cam->x, xmin), xmax);

	cam->y = p->y - ((float)cam->winheight / 2.0);
	cam->y = fmin(fmax(cam->y, ymin), ymax);

}
