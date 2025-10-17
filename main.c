#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "darr.h"

#define Grav_Const 6.67430e-11
#define Light_Speed 299792458.0
#define Body_Mass 1

// Our simulation's width and height
#define CANVAS_WIDTH 1000
#define CANVAS_HEIGHT 1000
#define TARGET_FPS 60

typedef struct {
	int x, y;
} pos2_t;

typedef struct {
	double x, y;
} dir2_t;

typedef struct {
	pos2_t pos;
	double mass;
	double r_s; // event horizon
} black_hole_t;

typedef struct {
	double x, y;
	dir2_t dir;
} ray_state_t;

typedef struct {
	double x, y;
	dir2_t dir;
	void *trail; // ptr to previous positions & directions; contiguous array of ray states
	size_t trail_cap, trail_len; // length and capacity
} ray_t;

static void black_hole_init(black_hole_t *black_hole);
static void ray_init(ray_t *ray,
	int x, int y,
	const dir2_t dir,
	void *trail_mem,
	const size_t trail_mem_offset,
	const size_t trail_cap);
static void ray_update_pos(ray_t *ray);

void black_hole_init(black_hole_t *black_hole) {
	black_hole->mass = Body_Mass;
	black_hole->r_s = (2.0 * Grav_Const * Body_Mass) / (Light_Speed * Light_Speed);
	black_hole->pos.x = 0;
	black_hole->pos.y = 0;
}

void ray_init(
	ray_t *ray,
	int x, int y,
	const dir2_t dir,
	void *trail_mem,
	const size_t trail_mem_offset,
	const size_t trail_cap
) {
	assert(trail_mem != NULL);
	ray->x = x;
	ray->y = y;
	ray->dir.x = dir.x;
	ray->dir.y = dir.y;
	ray->trail = trail_mem + trail_mem_offset * sizeof(ray_t);
	ray->trail_len = 0;
	ray->trail_cap = trail_cap;
}

void ray_update_pos(ray_t *ray) {
	if (ray->trail_cap == ray->trail_len) {
		ray->x += ray->dir.x * Light_Speed;
		ray->y += ray->dir.y * Light_Speed;
		return;
	}

	const uint8_t *trail = ray->trail;
	const size_t filled = ray->trail_len;
	ray_state_t *ray_state = (ray_state_t *)((uint8_t *)trail + filled * sizeof(ray_state_t));
	ray_state->x = ray->x;
	ray_state->y = ray->y;
	ray_state->dir = ray->dir;

	ray->x += ray->dir.x * Light_Speed;
	ray->y += ray->dir.y * Light_Speed;

	ray->trail_len += 1;
}

#define NUMBER_OF_RAYS 10

int main() {

	// For ray trails, create huge pool
	const size_t trail_max_len = 300;
	da_t trail_pool = da_new(NUMBER_OF_RAYS * trail_max_len, sizeof(ray_state_t));

	ray_t rays[NUMBER_OF_RAYS] = {0};
	const int ray_y_step = CANVAS_HEIGHT / (NUMBER_OF_RAYS + 1);
	int ray_ypos = 0;
	for (int i = 0; i < NUMBER_OF_RAYS; i++) {
		ray_ypos += ray_y_step;
		const int ray_xpos = 0;
		const dir2_t ray_dir = {.x = 1e-8, .y = 0};
		const size_t trail_offset = i * trail_max_len;
		ray_init(&rays[i], ray_xpos, ray_ypos, ray_dir, trail_pool.data, trail_offset, trail_max_len);
	}

	// black_hole_t black_hole;
	// black_hole_init(&black_hole);

	SetTargetFPS(TARGET_FPS);

	InitWindow(CANVAS_WIDTH, CANVAS_HEIGHT, "Black Hole");

	const int circle_radius = 80;

	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(BLACK);
		DrawCircle(CANVAS_WIDTH - circle_radius, 0.5 * CANVAS_HEIGHT, circle_radius, RED);
		for (int i = 0; i < NUMBER_OF_RAYS; i++) {
			ray_t *ray = &rays[i];
			for (int i = 0; i < ray->trail_len; i++) {
				ray_state_t *ray_state = (ray_state_t *)((uint8_t *)ray->trail + i * sizeof(ray_state_t));
				const int alpha = 255 * ((float)i / (float)(ray->trail_len - 1));
				const Color color = {255, 255, 255, alpha};
				DrawRectangle(ray_state->x, ray_state->y, 2, 2, color);
			}
			DrawRectangle(ray->x, ray->y, 2, 2, WHITE);
			ray_update_pos(ray);
		}
		EndDrawing();
	}

	da_terminate(&trail_pool);
	CloseWindow();
	return 0;
}
