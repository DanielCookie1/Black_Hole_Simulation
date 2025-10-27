#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>

#include "darr.h"

#define CANVAS_WIDTH 1000
#define CANVAS_HEIGHT 1000
#define TARGET_FPS 60

#define Grav_Const 6.67430e-11
#define Light_Speed 2.99792458
#define Body_Mass 1.0
#define Sch_Radius 150
#define Sch_Radius_Squared (Sch_Radius * Sch_Radius)
#define Body_XPos (CANVAS_WIDTH - 2.2 * Sch_Radius)
#define Body_YPos (0.5 * CANVAS_HEIGHT)

#define NUMBER_OF_RAYS 20

// for rays: Polar coordinates have origin at black hole center
#define get_radius(x, y) sqrt(((x) - Body_XPos) * ((x) - Body_XPos) + ((y) - Body_YPos) * ((y) - Body_YPos))
#define get_phase(x, y) atan2((y) - Body_YPos, (x) - Body_XPos)
#define get_xpos(radius, phase) ((cos(phase) * (radius)) + Body_XPos)
#define get_ypos(radius, phase) ((sin(phase) * (radius)) + Body_YPos)

// Schwarzschild metric
#define geodesic_ddrad(rad, dphi) ((rad) * (dphi) * (dphi) - (Light_Speed * Light_Speed * Sch_Radius) / (2.0 * (rad) * (rad)))
#define geodesic_ddphi(rad, drad, dphi) (-(2.0 / (rad)) * (drad) * (dphi))

typedef struct {
	int x, y;
} pos2_t;

typedef struct {
	double x, y;
} dir2_t;

// typedef struct {
// 	pos2_t pos;
// 	double mass;
// 	double schrad; // event horizon
// } black_hole_t;

typedef struct {
	double x, y;
	// double rad, phi;
	// double drad, dphi; // speeds
	// dir2_t dir;
} ray_state_t;

typedef struct {
	double x, y;
	double rad, phi;
	double drad, dphi; // speeds
	dir2_t dir;
	void *trail; // ptr to previous positions & directions; contiguous array of ray states
	size_t trail_cap, trail_len; // length and capacity
} ray_t;

// static void black_hole_init(black_hole_t *black_hole);
static void ray_init(ray_t *ray,
	int x, int y,
	const dir2_t dir,
	void *trail_mem,
	const size_t trail_mem_offset,
	const size_t trail_cap);
static void ray_update_pos(ray_t *ray, double lambda_step);

// void black_hole_init(black_hole_t *black_hole) {
// 	black_hole->mass = Body_Mass;
// 	black_hole->schrad = (2.0 * Grav_Const * Body_Mass) / (Light_Speed * Light_Speed);
// 	black_hole->pos.x = 0;
// 	black_hole->pos.y = 0;
// }

// Global state; mischievous
// double ddrad;
// double ddphi;

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
	ray->rad = get_radius(x, y);
	ray->phi = get_phase(x, y);
	ray->drad = 0.0000;
	ray->dphi = 0.0008;
	ray->dir.x = dir.x;
	ray->dir.y = dir.y;
	ray->trail = trail_mem + trail_mem_offset * sizeof(ray_state_t);
	ray->trail_len = 0;
	ray->trail_cap = trail_cap;
}

void ray_update_pos(ray_t *ray, double lambda_step) {
	if (ray->rad < Sch_Radius) {
		return;
	}
	// @DAN: we should not have a limitation on trail size: FIX
	if (ray->trail_cap == ray->trail_len) {
		ray->x += ray->dir.x * Light_Speed;
		ray->y += ray->dir.y * Light_Speed;
		ray->rad = get_radius(ray->x, ray->y);
		ray->phi = get_phase(ray->x, ray->y);
		return;
	}

	const uint8_t *trail = ray->trail;
	const size_t filled = ray->trail_len;
	ray_state_t *ray_state = (ray_state_t *)((uint8_t *)trail + filled * sizeof(ray_state_t));

	ray_state->x = ray->x;
	ray_state->y = ray->y;

	ray->trail_len += 1;

	// RK4
	// @DAN: Switch to vector state based implementation would clean up things?
	{
		// ddrad and ddphi found through geodesic equations
		const double rad = ray->rad;
		// const double phi = ray->phi;
		const double drad = ray->drad;
		const double dphi = ray->dphi;

		const double one_sixth = 1.0/6.0;
		const double half_lambda_step = 0.5 * lambda_step;

		double rad_temp, drad_temp,
			// phi_temp,
			dphi_temp;

		const double k1_rad  = drad;
		const double k1_phi  = dphi;
		const double k1_drad = geodesic_ddrad(rad, dphi);
		const double k1_dphi = geodesic_ddphi(rad, drad, dphi);

		rad_temp  = rad  + half_lambda_step * k1_rad;
		// phi_temp  = phi  + half_lambda_step * k1_phi;
		drad_temp = drad + half_lambda_step * k1_drad;
		dphi_temp = dphi + half_lambda_step * k1_dphi;

		const double k2_rad  = drad_temp;
		const double k2_phi  = dphi_temp;
		const double k2_drad = geodesic_ddrad(rad_temp, dphi_temp);
		const double k2_dphi = geodesic_ddphi(rad_temp, drad_temp, dphi_temp);

		rad_temp  = rad  + half_lambda_step * k2_rad;
		// phi_temp  = phi  + half_lambda_step * k2_phi;
		drad_temp = drad + half_lambda_step * k2_drad;
		dphi_temp = dphi + half_lambda_step * k2_dphi;

		const double k3_rad  = drad_temp;
		const double k3_phi  = dphi_temp;
		const double k3_drad = geodesic_ddrad(rad_temp, dphi_temp);
		const double k3_dphi = geodesic_ddphi(rad_temp, drad_temp, dphi_temp);

		rad_temp  = rad  + lambda_step * k3_rad;
		// phi_temp  = phi  + lambda_step * k3_phi;
		drad_temp = drad + lambda_step * k3_drad;
		dphi_temp = dphi + lambda_step * k3_dphi;

		const double k4_rad  = drad_temp;
		const double k4_phi  = dphi_temp;
		const double k4_drad = geodesic_ddrad(rad_temp, dphi_temp);
		const double k4_dphi = geodesic_ddphi(rad_temp, drad_temp, dphi_temp);

		ray->rad  += one_sixth * lambda_step * ((k1_rad  + k4_rad)  + 2 * (k2_rad  + k3_rad));
		ray->phi  += one_sixth * lambda_step * ((k1_phi  + k4_phi)  + 2 * (k2_phi  + k3_phi));
		ray->drad += one_sixth * lambda_step * ((k1_drad + k4_drad) + 2 * (k2_drad + k3_drad));
		ray->dphi += one_sixth * lambda_step * ((k1_dphi + k4_dphi) + 2 * (k2_dphi + k3_dphi));
	}

	ray->x = get_xpos(ray->rad, ray->phi);
	ray->y = get_ypos(ray->rad, ray->phi);
}

int main() {
	// For ray trails, create huge pool
	const size_t trail_max_len = 1000;
	da_t trail_pool = da_new(NUMBER_OF_RAYS * trail_max_len, sizeof(ray_state_t));

	ray_t rays[NUMBER_OF_RAYS] = {0};
	const int ray_y_step = CANVAS_HEIGHT / (NUMBER_OF_RAYS + 1);
	int ray_ypos = 0;
	for (int i = 0; i < NUMBER_OF_RAYS; i++) {
		ray_ypos += ray_y_step;
		const int ray_xpos = 0;
		const dir2_t ray_dir = {.x = 1e-1, .y = 0};
		const size_t trail_offset = i * trail_max_len;
		ray_init(&rays[i], ray_xpos, ray_ypos, ray_dir, trail_pool.data, trail_offset, trail_max_len);
	}

	// black_hole_t black_hole;
	// black_hole_init(&black_hole);

	SetTargetFPS(TARGET_FPS);

	InitWindow(CANVAS_WIDTH, CANVAS_HEIGHT, "Black Hole");

	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(BLACK);
		DrawCircle(Body_XPos, Body_YPos, Sch_Radius, RED);
		for (int i = 0; i < NUMBER_OF_RAYS; i++) {
			ray_t *ray = &rays[i];
			for (int i = 0; i < ray->trail_len; i++) {
				ray_state_t *ray_state = (ray_state_t *)((uint8_t *)ray->trail + i * sizeof(ray_state_t)); // @DAN: kinda dangerous?
				const int alpha = 255 * ((float)i / (float)(ray->trail_len - 1));
				const Color color = {255, 255, 255, alpha};
				DrawRectangle(ray_state->x, ray_state->y, 2, 2, color);
			}
			// geodesic(ray);
			DrawRectangle(ray->x, ray->y, 2, 2, WHITE);
			const double lambda_step = 5e0;
			ray_update_pos(ray, lambda_step);
		}
		EndDrawing();
	}

	da_terminate(&trail_pool);
	CloseWindow();
	return 0;
}
