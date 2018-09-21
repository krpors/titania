#ifndef GFX_H
#define GFX_H

#include <assert.h>

#include <SDL.h>

//#############################################################################
// Particle types.
//#############################################################################

/*
 * This struct contains information for a particle, to be used in a particle
 * generator.
 */
struct particle {
	float x;
	float y;
	float w;
	float h;

	float dx;
	float dy;

	int a;

	int life;
	int max_life;
};

//#############################################################################
// Animation types.
//#############################################################################

/*
 * Defines a structure which can be used for animation purposes.
 */
struct anim {
	int frame_time; // time for each frame in milliseconds.
	int counter; // counter;

	int curr; // the current frame index
	int n;    // the index used by anim_add

	SDL_Rect* rectangles; 	// List of rectangles.
};

struct anim* anim_create(int frame_time);

void anim_free(struct anim* a);
void anim_add(struct anim* a, int x, int y, int w, int h);

void anim_reset(struct anim* a);
void anim_next(struct anim* a);
const SDL_Rect* anim_current(struct anim* a);

#endif // GFX_H