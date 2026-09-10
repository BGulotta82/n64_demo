#include "camera.h"
#include "constants.h"

void camera_init(camera_t *cam, int world_width, int world_height, int screen_width, int screen_height) {
    cam->x = 0;
    cam->y = 0;
    cam->width = screen_width;
    cam->height = screen_height;
    cam->world_width = world_width;
    cam->world_height = world_height;
}

void camera_update(camera_t *cam, int *player_x, int *player_y, bool *player_active, int player_count) {
    int active_count = 0;
    
    // Bounds tracking variables for both axes
    int min_x = 999999;
    int max_x = -999999;
    int min_y = 999999;
    int max_y = -999999;

    for (int i = 0; i < player_count; i++) {
        if (!player_active[i]) {
            continue;
        }

        active_count++;
        
        // Horizontal bounds tracking
        if (player_x[i] < min_x) min_x = player_x[i];
        if (player_x[i] + PLAYER_WIDTH > max_x) max_x = player_x[i] + PLAYER_WIDTH;

        // Vertical bounds tracking
        if (player_y[i] < min_y) min_y = player_y[i];
        if (player_y[i] + PLAYER_HEIGHT > max_y) max_y = player_y[i] + PLAYER_HEIGHT;
    }

    if (active_count == 0) {
        return;
    }

    // --- HORIZONTAL TRACKING & CLAMPING ---
    int desired_x = (min_x + max_x) / 2 - (cam->width / 2);

    if (desired_x < 0) desired_x = 0;
    if (desired_x + cam->width > cam->world_width) {
        desired_x = cam->world_width - cam->width;
    }
    cam->x = desired_x;

    // --- VERTICAL TRACKING & CLAMPING (THE FIX) ---
    int desired_y = (min_y + max_y) / 2 - (cam->height / 2);

    if (desired_y < 0) desired_y = 0;
    if (desired_y + cam->height > cam->world_height) {
        desired_y = cam->world_height - cam->height;
    }
    cam->y = desired_y;
}
