#include "camera.h"
#include "constants.h"

camera_t cameras[MAX_VIEWPORTS]; 

// Dedicated layout configurations supporting 1, 2, or 4-way screen grids
viewport_layout_t viewport_configs[4][4] = {
    // --- 1 PLAYER ACTIVE: Full Screen (320 x 240) ---
    { {0, 0, 320, 240}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} },

    // --- 2 PLAYERS ACTIVE: Horizontal Dual Split (320 x 120 each) ---
    { {0, 0, 320, 120}, {0, 120, 320, 120}, {0, 0, 0, 0}, {0, 0, 0, 0} },

    // --- 3 PLAYERS ACTIVE: Quad Grid Setup (P4 slot sits black or holds a mini-map) ---
    { {0, 0, 160, 120}, {160, 0, 160, 120}, {0, 120, 160, 120}, {160, 120, 160, 120} },

    // --- 4 PLAYERS ACTIVE: Full Quad Grid Display (160 x 120 each) ---
    { {0, 0, 160, 120}, {160, 0, 160, 120}, {0, 120, 160, 120}, {160, 120, 160, 120} }
};

void camera_init(camera_t *cam, float world_width, float world_height, float screen_width, float screen_height, float spawn_x, float spawn_y) {
    cam->width = screen_width;
    cam->height = screen_height;
    cam->world_width = world_width;
    cam->world_height = world_height;

    // --- CENTERING CALCULATIONS (Using Floats) ---
    // Subtract half the screen dimensions from the player spawn coordinates safely
    float desired_x = spawn_x - (screen_width / 2.0f);
    float desired_y = spawn_y - (screen_height / 2.0f);

    // --- MAP BOUNDARY CLAMPING ---
    // Prevent the camera from scrolling past the left/top edges
    if (desired_x < 0.0f) desired_x = 0.0f;
    if (desired_y < 0.0f) desired_y = 0.0f;

    // Prevent the camera from scrolling past the right/bottom edges
    if (desired_x + cam->width > cam->world_width) {
        desired_x = cam->world_width - cam->width;
    }
    if (desired_y + cam->height > cam->world_height) {
        desired_y = cam->world_height - cam->height;
    }

    // Set the finalized, safely-bounded starting positions as floats
    cam->x = desired_x;
    cam->y = desired_y;
}


void camera_update_split(camera_t *cam, float p_x, float p_y, float p_w, float p_h, float view_w, float view_h, facing_dir direction, float dt) {
    cam->width = view_w;
    cam->height = view_h;

    // 1. Calculate the standard dead-center baseline focal point
    float desired_x = p_x + (p_w / 2.0f) - (view_w / 2.0f);
    float desired_y = p_y + (p_h / 2.0f) - (view_h / 2.0f);

    // Forward-facing focus offset
    float dynamic_bias = (view_w <= 160.0f) ? 24.0f : 48.0f;

    if (direction == FACING_LEFT) {
        desired_x -= dynamic_bias;
    } else if (direction == FACING_RIGHT) {
        desired_x += dynamic_bias;
    }

    // 2. Enforce absolute level boundaries safely without integer truncations
    if (desired_x < 0.0f) desired_x = 0.0f;
    if (desired_x + view_w > cam->world_width)  desired_x = cam->world_width - view_w;
    if (desired_y < 0.0f) desired_y = 0.0f;
    if (desired_y + view_h > cam->world_height) desired_y = cam->world_height - view_h;

    // 3. Smoothly glide using pure floating-point math
    float error_x = desired_x - cam->x;
    float error_y = desired_y - cam->y;
    
    float tracking_speed = 4.5f;

    cam->x += error_x * tracking_speed * dt;
    cam->y += error_y * tracking_speed * dt;
}
