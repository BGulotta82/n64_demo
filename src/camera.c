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

void camera_init(camera_t *cam, int world_width, int world_height, int screen_width, int screen_height, float spawn_x, float spawn_y) {
    cam->width = screen_width;
    cam->height = screen_height;
    cam->world_width = world_width;
    cam->world_height = world_height;

    // --- CENTERING CALCULATIONS ---
    // Subtract half the screen dimensions from the player spawn coordinates
    int desired_x = (int)spawn_x - (screen_width / 2);
    int desired_y = (int)spawn_y - (screen_height / 2);

    // --- MAP BOUNDARY CLAMPING ---
    // Prevent the camera from scrolling past the left/top edges
    if (desired_x < 0) desired_x = 0;
    if (desired_y < 0) desired_y = 0;

    // Prevent the camera from scrolling past the right/bottom edges
    if (desired_x + cam->width > cam->world_width) {
        desired_x = cam->world_width - cam->width;
    }
    if (desired_y + cam->height > cam->world_height) {
        desired_y = cam->world_height - cam->height;
    }

    // Set the finalized, safely-bounded starting positions
    cam->x = desired_x;
    cam->y = desired_y;
}

void camera_update_split(camera_t *cam, int p_x, int p_y, int p_w, int p_h, int view_w, int view_h, float dt) {
    // 1. Assign current frame width/height to camera metrics
    cam->width = view_w;
    cam->height = view_h;

    // 2. Calculate the target destination centered exactly over this specific player
    int desired_x = p_x + (p_w / 2) - (view_w / 2);
    int desired_y = p_y + (p_h / 2) - (view_h / 2);

    // 3. Enforce absolute level boundaries
    if (desired_x < 0) desired_x = 0;
    if (desired_x + view_w > cam->world_width)  desired_x = cam->world_width - view_w;
    if (desired_y < 0) desired_y = 0;
    if (desired_y + view_h > cam->world_height) desired_y = cam->world_height - view_h;

    // 4. Smoothly glide this camera toward its target
    int error_x = desired_x - cam->x;
    int error_y = desired_y - cam->y;
    float tracking_speed = 6.0f;

    cam->x += (int)((float)error_x * tracking_speed * dt);
    cam->y += (int)((float)error_y * tracking_speed * dt);
}
