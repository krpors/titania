#include "camera.h"
#include "tilemap.h"
#include "player.h"
#include "bitmapfont.h"
#include "tmx/tmx.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <time.h>

#include <SDL.h>
#include <SDL_image.h>

static SDL_Renderer* gRenderer = NULL;
static SDL_Window* gWindow = NULL;
static bool quit = false;
static bool pause = false;
static bool drawgrid = false;
static bool drawdebug = false;
static struct player p;

float tilewidth = 64;
float tileheight = 64;

struct background {
	SDL_Texture* tex;
	int w;
	int h;
};

bool background_init(struct background* bg, SDL_Renderer* r, const char* path) {
	bg->tex = IMG_LoadTexture(r, path);
	if (bg->tex == NULL) {
		return false;
	}

	SDL_QueryTexture(bg->tex, NULL, NULL, &bg->w, &bg->h);

	return true;
}

void background_free(struct background* bg) {
	SDL_DestroyTexture(bg->tex);
	bg->tex = NULL;
}

void background_draw(struct background* bg, SDL_Renderer* r, struct camera* cam) {
	SDL_SetRenderDrawColor(r, 0xff, 0xff, 0xff, 0xff);
	SDL_Rect src = {0, 0, bg->w, bg->h};
	SDL_Rect dst = {-cam->x / 6 - 1800, -cam->y / 6 - 200, bg->w * 2, bg->h * 2};
	SDL_RenderCopy(r, bg->tex, &src, &dst);
}




void draw_grid(const struct camera* cam, const struct tilemap* tm, SDL_Renderer* r) {
	(void)cam;
	if (drawgrid) {
		int mapwidth, mapheight;
		tilemap_getsize(tm, &mapwidth, &mapheight);
		float tw = tm->tilewidth;
		float th = tm->tileheight;
		SDL_SetRenderDrawColor(r, 0, 255, 0, 55);
		for (int x = 0; x < mapwidth; x += tw) {
			SDL_RenderDrawLine(r, x + tw - cam->x, 0, x + tw - cam->x, 600);
		}
		for (int y = 0; y < mapheight; y += th) {
			SDL_RenderDrawLine(r, 0, y + th - cam->y, 800, y + th - cam->y);
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
		case SDLK_p: drawdebug = !drawdebug; break;
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

void* sdl_img_loader(const char *path) {
	return IMG_LoadTexture(gRenderer, path);
}

int main(int argc, char* argv[]) {
	(void)(argc);
	(void)(argv);

	tmx_map* map = tmx_load("map01.tmx");
	if (map == NULL) {
		tmx_perror("tmx_load");
	}
	tmx_map_free(map);

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

	float ratio = 14.0;
	tilewidth = ceilf(800.0 / ratio);
	tileheight = ceilf(600.0 / (ratio / 1.3333));

	debug_print("Tile width(%.0f) and height(%.0f)\n", tilewidth, tileheight);

	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);

	tmx_img_load_func = (void* (*)(const char*))sdl_img_loader;
	tmx_img_free_func = (void (*)(void*)) SDL_DestroyTexture;

#if 1
	struct background bg;
	if (!background_init(&bg, gRenderer, "background.png")) {
		fprintf(stderr, "Unable to load background!\n");
		exit(1);
	}
#endif

	struct tilemap tm;
	if (!tilemap_load(&tm, "map01.tmx")) {
		tilemap_free(&tm);
		exit(1);
	}
	tm.tilewidth = tilewidth;
	tm.tileheight = tileheight;

	player_init(&p);
	if (!player_load_texture(&p, gRenderer, "player.png")) {
		exit(1);
	}
	p.map = &tm;

	struct camera cam;
	camera_init(&cam, 800.0, 600.0);

	const char* glyphs = " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,!?-+/():;%&`'*#=[]\"";
	struct bitmapfont bmf;
	if (!bitmapfont_init(&bmf, gRenderer, "font.png", glyphs)) {
		exit(1);
	}

	p.font = &bmf;

	SDL_Event e;

	float deltaTime = 0.0f;

	uint32_t fps_timer = SDL_GetTicks();
	long total_frames = 0;
	uint32_t timeBefore = 0;
	uint32_t timeAfter = 0;
	while (!quit) {

		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = true;
			}
			handle_keypress(&e);
			tilemap_handle_event(&tm, &e);
			player_handle_event(&p, &e);
		}

		if (pause) {
			// TODO: quick hack. When pausing, the timestep would
			// increase to very large amounts, resulting in real fast
			// movement/behaviour. This has to be done better I guess.
			deltaTime = SDL_GetTicks();
			continue;
		}

		float secondspassed = (SDL_GetTicks() - fps_timer) / 1000.0f;
		float fps = total_frames / secondspassed;

		// Update logic
		deltaTime = (timeAfter - timeBefore) / 1000.0f;
		timeBefore = SDL_GetTicks();

		player_update(&p, deltaTime);

		camera_update(&cam, &p, &tm);

		// Render logic
		SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);
		SDL_RenderClear(gRenderer);

		background_draw(&bg, gRenderer, &cam);
		tilemap_draw_background(&tm, &cam, gRenderer);
		player_draw(&p, &cam, gRenderer);
		tilemap_draw_foreground(&tm, &cam, gRenderer);

		draw_grid(&cam, &tm, gRenderer);

		if (drawdebug) {
			bitmapfont_renderf(&bmf, 0, 0 * 14, "P(%3.0f, %3.0f), vx: %f, dy: %f", p.x, p.y, p.dx, p.dy);
			bitmapfont_renderf(&bmf, 0, 1 * 14, "  jumping: %d", p.jumping);
			bitmapfont_renderf(&bmf, 0, 2 * 14, "  can jump: %d", p.can_jump);
			bitmapfont_renderf(&bmf, 0, 3 * 14, "  boop_life: %-3d", p.boop_life);
			// bitmapfont_renderf(&bmf, 0, 4 * 14, "  anim: %d", p.anim);
			// spacing
			bitmapfont_renderf(&bmf, 0, 6 * 14, "Delta time: %-3f", deltaTime);
			bitmapfont_renderf(&bmf, 0, 7 * 14, "FPS: %-3f", fps);
			bitmapfont_renderf(&bmf, 0, 8 * 14, "Cam: %1.0f, %1.0f", cam.x, cam.y);
			bitmapfont_renderf(&bmf, 0, 9 * 14, "Tile size: %1.0f x %1.0f", tm.tilewidth, tm.tileheight);
		}

		SDL_RenderPresent(gRenderer);

		timeAfter = SDL_GetTicks();

		total_frames++;
	}

	bitmapfont_free(&bmf);
	tilemap_free(&tm);

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	IMG_Quit();
	SDL_Quit();
	return 0;
}
