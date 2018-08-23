#include "tilemap.h"
#include "player.h"
#include "bitmapfont.h"

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
static bool drawgrid = false;
static struct player p;

int tilewidth = 64;
int tileheight = 64;

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
	int tw = tilewidth;
	int th = tileheight;
	for(int y = 0; y < m->h; y++) {
		for (int x = 0; x < m->w; x++) {
			int bleh = tilemap_get(m, x, y);
			SDL_Rect src;
			SDL_SetTextureAlphaMod(m->sheet->texture, 255);
			if (bleh == TILE_MAP_EDGE) {
				src.x = 32;
				src.y = 0;
				src.w = 32;
				src.h = 32;
			} else if (bleh == 0xaa) {
				src.x = 64;
				src.y = 0;
				src.w = 32;
				src.h = 32;
			} else if (bleh == TILE_NONE) {
				SDL_SetTextureAlphaMod(m->sheet->texture, 90);
				src.x = 160;
				src.y = 64;
				src.w = 32;
				src.h = 32;
			}

			SDL_Rect rect = { x * tw, y * th, th, th };
			SDL_RenderCopy(r, m->sheet->texture, &src, &rect);

		}
	}
}

void draw_grid(SDL_Renderer* r) {
	if (drawgrid) {
		int w = tilewidth;
		int h = tileheight;
		SDL_SetRenderDrawColor(r, 0, 255, 0, 55);
		for (int x = 0; x < 800; x += w) {
			SDL_RenderDrawLine(r, x + w, 0, x + w, 600);
		}
		for (int y = 0; y < 600; y += h) {
			SDL_RenderDrawLine(r, 0, y + h, 800, y + h);
		}
	}
}

void handle_keypress(const SDL_Event* event) {
	if (event->type != SDL_KEYDOWN && event->type != SDL_KEYUP) {
		return;
	}

	if (event->type == SDL_KEYDOWN) {
		switch (event->key.keysym.sym) {
		case SDLK_d: drawgrid = !drawgrid; break;
		case SDLK_f: SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_FULLSCREEN); break;
		case SDLK_ESCAPE: quit = true; break;
		case SDLK_SPACE:
			if (event->key.type == SDL_KEYDOWN) {
				pause = !pause;
			}
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

	tilewidth = ceilf(800.0 / 12.0);
	tileheight = ceilf(600.0 / 9.0);

	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
	SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);

	struct spritesheet sheet;
	if (!spritesheet_init(&sheet, "/home/krpors/Development/pixelart.png")) {
		exit(1);
	}

	struct tilemap tm;
	if (!tilemap_read(&tm, "tilemap.txt")) {
		tilemap_free(&tm);
		exit(1);
	}
	tm.sheet = &sheet;

	player_init(&p);
	p.map = &tm;

	const char* glyphs = " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,!?-+/():;%&`'*#=[]\"";
	struct bitmapfont bmf;
	if (!bitmapfont_init(&bmf, gRenderer, "font.png", glyphs)) {
		exit(1);
	}

	p.font = &bmf;

	SDL_Event e;

	float deltaTime = 0.0f;

	while (!quit) {

		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = true;
			}
			handle_keypress(&e);
			player_handle_event(&p, &e);
		}

		if (pause) {
			// TODO: quick hack. When pausing, the timestep would
			// increase to very large amounts, resulting in real fast
			// movement/behaviour. This has to be done better I guess.
			deltaTime = SDL_GetTicks();
			continue;
		}

		float timeStep = (SDL_GetTicks() - deltaTime) / 1000.0f;

		SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);
		SDL_RenderClear(gRenderer);

		player_update(&p, timeStep);

		rendertilemap(&tm, gRenderer);
		player_draw(&p, gRenderer);
		draw_grid(gRenderer);

		bitmapfont_renderf(&bmf, 0, 0 * 14, "P(%3.0f, %3.0f), vx: %f, dy: %f", p.x, p.y, p.dx, p.dy);
		bitmapfont_renderf(&bmf, 0, 1 * 14, "  jumping: %d", p.jumping);
		bitmapfont_renderf(&bmf, 0, 2 * 14, "  can jump: %d", p.can_jump);
		bitmapfont_renderf(&bmf, 0, 3 * 14, "  boop_life: %-3d", p.boop_life);

		deltaTime = SDL_GetTicks();

		SDL_RenderPresent(gRenderer);

	}

	bitmapfont_free(&bmf);
	tilemap_free(&tm);

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	IMG_Quit();
	SDL_Quit();
	return 0;
}
