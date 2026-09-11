#include "camera.h"
#include "constants.h"

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

void camera_update(camera_t *cam, int *player_x, int *player_y, int *player_w, int *player_h, bool *player_active, int player_count, float dt) {
    int active_count = 0;
    
    int min_x = 999999;
    int max_x = -999999;
    int min_y = 999999;
    int max_y = -999999;

    for (int i = 0; i < player_count; i++) {
        if (!player_active[i]) {
            continue;
        }

        active_count++;
        
        // FIXED: Track boundaries using the passed-in dimension arrays instead of globals!
        if (player_x[i] < min_x) min_x = player_x[i];
        if (player_x[i] + player_w[i] > max_x) max_x = player_x[i] + player_w[i];

        if (player_y[i] < min_y) min_y = player_y[i];
        if (player_y[i] + player_h[i] > max_y) max_y = player_y[i] + player_h[i];
    }

    if (active_count == 0) {
        return;
    }

    // 1. Calculate the raw TARGET coordinates using your bounding boxes
    int desired_x = (min_x + max_x) / 2 - (cam->width / 2);
    int desired_y = (min_y + max_y) / 2 - (cam->height / 2);

    // 2. Clamp the targets inside the level matrix boundaries
    if (desired_x < 0) desired_x = 0;
    if (desired_x + cam->width > cam->world_width) {
        desired_x = cam->world_width - cam->width;
    }

    if (desired_y < 0) desired_y = 0;
    if (desired_y + cam->height > cam->world_height) {
        desired_y = cam->world_height - cam->height;
    }

    // =========================================================================
    // --- THE SNAP FIX: SMOOTH INTEGER INTERPOLATION ---
    // =========================================================================
    int error_x = desired_x - cam->x;
    int error_y = desired_y - cam->y;

    float tracking_speed = 6.0f;

    cam->x += (int)((float)error_x * tracking_speed * dt);
    cam->y += (int)((float)error_y * tracking_speed * dt);
}
