#ifndef CAMERA_H
#define CAMERA_H

// Forward declaration to satisfy the dependency of player.
// player.h includes camera.h, meaning camera.h cannot include
// player.h for usage since it would be a circular dependency.
struct player;
struct tilemap;

struct camera {
	float x;
	float y;
	int winwidth;
	int winheight;
};

void camera_init(struct camera* cam, int winwidth, int winheight);

void camera_update(struct camera* cam, const struct player* p, const struct tilemap* map);

const char* camera_str(struct camera* cam);

#endif // CAMERA_H
