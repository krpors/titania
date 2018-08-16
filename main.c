#include "tilemap.h"
#include "player.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <time.h>

#include <SDL.h>
#include <SDL_image.h>

#define debug(s, ...); fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, s);

static SDL_Renderer* gRenderer = NULL;
static SDL_Window* gWindow = NULL;
static bool quit = false;
static bool pause = false;
static struct player p;

struct spritesheet {
	int width;  // width of the spritesheet
	int height; // height of the spritesheet
	int sz;     // size of each sprite in the sheet.

	SDL_Texture* texture;
};

bool spritesheet_init(struct spritesheet* ss, const char* path) {
	assert(ss != NULL);
	assert(path != NULL);
	assert(gRenderer != NULL);

	SDL_Surface* surface = IMG_Load(path);
	if (surface == NULL) {
		fprintf(stderr, "Unable to load surface from path '%s': %s\n", path, SDL_GetError());
		return false;
	}

	ss->sz     = 32;
	ss->width  = surface->w;
	ss->height = surface->h;

	ss->texture = SDL_CreateTextureFromSurface(gRenderer, surface);
	if (ss->texture == NULL) {
		fprintf(stderr, "Unable to create texture from path '%s': %s\n", path, SDL_GetError());
		SDL_FreeSurface(surface);
		return false;
	}

	SDL_FreeSurface(surface);

	return true;
}

void spritesheet_free(struct spritesheet* ss) {
	SDL_DestroyTexture(ss->texture);
}

void rendersprite(const struct spritesheet* ss, SDL_Renderer* renderer) {
	SDL_Rect src = { .x = 32, .y = 0,  .w = ss->sz, .h = ss->sz };
	SDL_Rect dst = { .x = 82, .y = 80, .w = ss->sz, .h = ss->sz };
	SDL_RenderCopy(renderer, ss->texture,  &src, &dst);
}

void rendertilemap(const struct tilemap* m, SDL_Renderer* r) {
	int tw = TILE_SIZE;
	for(int y = 0; y < m->h; y++) {
		for (int x = 0; x < m->w; x++) {
			int bleh = tilemap_get(m, x, y);
			if (bleh == TILE_MAP_EDGE) {
				SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 255);
			} else if (bleh == 0xaa) {
				SDL_SetRenderDrawColor(gRenderer, 255, 255, 0, 255);
			} else if (bleh == TILE_NONE) {
				SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);
			}

			SDL_Rect rect = { x * tw, y * tw, tw, tw };
			SDL_RenderFillRect(r, &rect);

		}
	}
}

void draw_grid(SDL_Renderer* r) {
	int w = 64;
	SDL_SetRenderDrawColor(r, 0, 255, 0, 55);
	for (int x = 0; x < 800; x += w) {
		SDL_RenderDrawLine(r, x + w, 0, x + w, 600);
	}
	for (int y = 0; y < 600; y += w) {
		SDL_RenderDrawLine(r, 0, y + w, 800, y + w);
	}
}

void handle_keypress(const SDL_Event* event) {
	if (event->type != SDL_KEYDOWN && event->type != SDL_KEYUP) {
		return;
	}

	switch (event->key.keysym.sym) {
	case SDLK_f:
		SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_FULLSCREEN);
		break;
	case SDLK_ESCAPE:
		quit = true;
		break;
	case SDLK_SPACE:
		if (event->key.type == SDL_KEYDOWN) {
			pause = !pause;
		}
		break;
	}
}

/**
 * Handles keypresses for the player.
 */
void handle_keypress_player(const SDL_Event* event, struct player* p) {
	if (event->type != SDL_KEYDOWN && event->type != SDL_KEYUP) {
		return;
	}

	if (event->type == SDL_KEYDOWN) {
		switch (event->key.keysym.sym) {
		case SDLK_LEFT:
			player_left(p);
			break;
		case SDLK_RIGHT:
			player_right(p);
			break;
		case SDLK_UP:
			player_up(p);
			break;
		case SDLK_DOWN:
			player_down(p);
			break;
		}
	} else if (event->type == SDL_KEYUP) {
		switch (event->key.keysym.sym) {
		case SDLK_LEFT:
		case SDLK_RIGHT:
		case SDLK_UP:
		case SDLK_DOWN:
			player_stop(p);
			break;
		}
	}
}

int main(int argc, char* argv[]) {
	(void)(argc);
	(void)(argv);

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Cannot init SDL: %s\n", SDL_GetError());
		exit(1);
	}

	int flags = IMG_INIT_PNG;
	if ((IMG_Init(flags) & flags) != flags) {
		fprintf(stderr, "Failed to init SDL_image :%s\n", IMG_GetError());
		exit(1);
	}

	gWindow = SDL_CreateWindow(
		"Titania",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		800, 600,
		SDL_WINDOW_SHOWN);

	if (gWindow == NULL) {
		fprintf(stderr, "Cannot open window: %s\n", SDL_GetError());
		exit(1);
	}

	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
	SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);

	struct tilemap tm;
	if (!tilemap_read(&tm, "tilemap.txt")) {
		tilemap_free(&tm);
		exit(1);
	}

	player_init(&p);
	p.map = &tm;

	SDL_Event e;

	while (!quit) {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = true;
			}
			handle_keypress(&e);
			handle_keypress_player(&e, &p);
		}

		if (pause) {
			continue;
		}

		SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);
		SDL_RenderClear(gRenderer);

		player_update(&p);

		rendertilemap(&tm, gRenderer);
		player_draw(&p, gRenderer);
		draw_grid(gRenderer);

		SDL_RenderPresent(gRenderer);
	}

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	IMG_Quit();
	SDL_Quit();
	return 0;
}
