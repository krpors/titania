#include "util.h"

#include <assert.h>
#include <stdbool.h>

#include <SDL.h>
#include <SDL_image.h>

extern SDL_Renderer* gRenderer;

SDL_Rect rect_to_SDLRect(const struct rect* r) {
	SDL_Rect ret = {
		.x = r->x,
		.y = r->y,
		.w = r->w,
		.h = r->h,
	};

	return ret;
}

bool is_color_equal(SDL_Color* one, SDL_Color* two) {
	return one->r == two->r
		&& one->g == two->g
		&& one->b == two->b
		&& one->a == two->a;
}

float random_float(float min, float max) {
	float scale = rand() / (float) RAND_MAX;
	return min + scale * (max - min);
}

//#############################################################################
// Circular list implementation.
//#############################################################################

struct circular_list* circular_list_create() {
	struct circular_list* cl = malloc(sizeof(struct circular_list));
	cl->head = NULL;
	cl->curr = NULL;
	cl->tail = NULL;
	return cl;
}

void circular_list_add(struct circular_list* cl, void* data) {
	struct circular_list_node* newnode = malloc(sizeof(struct circular_list_node));
	newnode->data = data;
	debug_print("Adding new node: %p, size = %lu\n", (void*)newnode, sizeof(newnode));

	// First element inserted.
	if (cl->head == NULL) {
		// this is the very first node, and the next node of the current node points to
		// itself (it's circular).
		cl->head = newnode;
		cl->curr = newnode;
		cl->tail = newnode;
		newnode->next = newnode;
	} else {
		// Update the tail of the current tail to the new node:
		cl->tail->next = newnode;
		// The new node is now the new tail:
		cl->tail = newnode;
		// The next node of the new node is the head of the list.
		newnode->next = cl->head;
	}
}

void* circular_list_next(struct circular_list* cl) {
	void* data = cl->curr->data;
	cl->curr = cl->curr->next;
	return data;
}

void circular_list_free(struct circular_list* cl) {
	debug_print("Head is %p\n", (void*)cl->head);
	debug_print("Tail is %p\n", (void*)cl->tail);

	struct circular_list_node* node = cl->head;
	while (node != cl->tail) {
		struct circular_list_node* next = node->next;
		debug_print("Freeing %p\n", (void*)node);
		free(node);
		node = next;
	}

	debug_print("Freeing %p\n", (void*)cl->tail);
	free(cl->tail);
	cl->tail = NULL;
	free(cl);
	cl = NULL;
}
