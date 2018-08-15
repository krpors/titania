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
void tilemap_add(struct tilemap* m, int tileval);

void tilemap_init(struct tilemap* m) {
	m->tiles = calloc(1, sizeof(int));
	m->len = 0;
	m->w = 0;
	m->h = 0;
}

bool tilemap_read(struct tilemap* map, const char* path) {
	map->tiles = calloc(1, sizeof(int));
	map->len = 0;
	map->w = 0;
	map->h = 0;

	FILE* f = fopen(path, "r");
	if (f == NULL) {
		perror(path);
	}

	char buf[3] = { 0 }; // This buffer will be filled with each read hex digit.
	int  bi = 0;         // The buffer index (to appoint char in buf)

	int maxcols = -1; // Maximum columns detected. -1 is the base, default.
	int cols = 0;     // The amount of columns detected.
	int rows = 0;     // The amount of rows.

	int r;
	while (r = fgetc(f), r != EOF) {
		// When we discover a newline, we hit the end of the row.
		// Reset some indicators and make us ready to parse a new row.
		if (r == '\n') {
			// First iteration of columns. This row contains the
			// expected amount of columns. If the following amount
			// do not match up, write an error and bail out.
			if (maxcols == -1) {
				maxcols = cols;
			} else if (maxcols != cols) {
				fprintf(stderr,
					"warning: inconsistent colums detected "
					"(expected %d, got %d) at (%d,%d)\n", maxcols, cols, rows, cols);
				return false;
			}

			rows++;
			bi = 0;
			cols = 0;
		}

		// If we hit a space, it's a separator of a column.
		if (r == ' ') {
			bi = 0;
		}

		// Check if the read char is a hexadecimal digit, and
		// if so, append it to the buffer.
		if (isxdigit(r)) {
			buf[bi++] = r;
		} else {
			// TODO: this
			//fprintf(stderr, "error: %c is not 
		}

		if (bi == 2) {
			int cruft = strtol(buf, NULL, 16);
			tilemap_add(map, cruft);
			//printf("%3d ", cruft);
			cols++;
		}

		if (bi > 2) {
			fprintf(stderr, "warning: expecting single byte\n");
			return false;
		}
	}

	map->w = maxcols;
	map->h = rows;

	return true;

}

void tilemap_add(struct tilemap* m, int tileval) {
	assert(m != NULL);
	m->tiles[m->len++] = tileval;
	m->tiles = realloc(m->tiles, (m->len + 1) * sizeof(int));
}



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
