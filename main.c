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
	assert(ss != null);
	assert(path != null);
	assert(gRenderer != null);

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

int main(int argc, char* argv[]) {
	(void)(argc);
	(void)(argv);

	uint8_t* zoit = calloc(20 * 10, sizeof(uint8_t));

	struct map m;
	map_init(&m);
	map_set_tiles(&m, zoit);
	map_free(&m);

	return 0;
}

#if 0
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
		"Platformer",
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

	struct spritesheet sheet;
	spritesheet_init(&sheet, "/home/krpors/Development/pixelart.png");

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

		rendersprite(&sheet, gRenderer);

		SDL_RenderPresent(gRenderer);
	}

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
	return 0;
}
#endif
