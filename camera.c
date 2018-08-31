#include "camera.h"
#include "player.h"

void camera_init(struct camera* cam, int winwidth, int winheight) {
	cam->winwidth = winwidth;
	cam->winheight = winheight;
}

void camera_update(struct camera* cam, const struct player* p) {
	(void)cam;
	(void)p;
	// TODO: lerping, stay within bounds etc.
	cam->x = p->x - (800 / 2);
	cam->y = p->y - (600 / 2);
}
