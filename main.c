#include "tilemap.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <time.h>

#include <SDL.h>
#include <SDL_image.h>

#define debug(s); fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, s);

SDL_Renderer* gRenderer = NULL;

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
	int tw = 64;
	for(int y = 0; y < m->h; y++) {
		for (int x = 0; x < m->w; x++) {
			int bleh = tilemap_get(m, x, y);
			if (bleh == TILE_BRICK) {
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

	SDL_Window* window = SDL_CreateWindow(
		"Titania",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		800, 600,
		SDL_WINDOW_SHOWN);

	if (window == NULL) {
		fprintf(stderr, "Cannot open window: %s\n", SDL_GetError());
		exit(1);
	}

	gRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);

	struct tilemap tm;
	if (!tilemap_read(&tm, "tilemap.txt")) {
		tilemap_free(&tm);
		exit(1);
	}


	SDL_Event e;
	bool quit = false;
	bool pause = false;

	while (!quit) {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = true;
			}

			if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym) {
				case SDLK_f:
					SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
					break;
				case SDLK_ESCAPE:
					quit = true;
					break;
				case SDLK_SPACE:
					if (e.key.type == SDL_KEYDOWN) {
						pause = !pause;
					}
					break;
				}
			}
		}

		if (pause) {
			continue;
		}

		SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);
		SDL_RenderClear(gRenderer);

		rendertilemap(&tm, gRenderer);

		SDL_RenderPresent(gRenderer);
	}

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
	return 0;
}
