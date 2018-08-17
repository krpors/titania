#include "tilemap.h"

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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
				fclose(f);
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

		// if we read two bytes, (e.g. aa, fa, 09, 18), convert the buffer
		// contents to an integer, and add it to the map.
		if (bi == 2) {
			int cruft = strtol(buf, NULL, 16);
			tilemap_add(map, cruft);
			cols++;
		}

		if (bi > 2) {
			fprintf(stderr, "warning: expecting single byte\n");
			return false;
		}
	}
	fclose(f);

	map->w = maxcols;
	map->h = rows;

	return true;

}

int tilemap_get(const struct tilemap* m, int x, int y) {
	// Protect from going out of bounds. If that happens, return
	// the tile type TILE_MAP_EDGE so we don't get weird behaviour
	// by indexing impossible values on the array.
	if (x < 0 || x > m->w || y < 0 || y > m->h) {
		return TILE_MAP_EDGE;
	}
	// We're having a one dimensional array. We know the width and
	// height, so we can index that with (x,y) coords by using the
	// next formula.
	return m->tiles[y * m->w + x];
}

void tilemap_add(struct tilemap* m, int tileval) {
	assert(m != NULL);
	m->tiles[m->len++] = tileval;
	m->tiles = realloc(m->tiles, (m->len + 1) * sizeof(int));
}

void tilemap_free(struct tilemap* m) {
	assert(m != NULL);
	free(m->tiles);
}
