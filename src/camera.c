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
    int min_x = 999999;
    int max_x = -999999;

    for (int i = 0; i < player_count; i++) {
        if (!player_active[i]) {
            continue;
        }

        active_count++;
        if (player_x[i] < min_x) min_x = player_x[i];
        if (player_x[i] + PLAYER_WIDTH > max_x) max_x = player_x[i] + PLAYER_WIDTH;
    }

    if (active_count == 0) {
        return;
    }

    int desired_x = (min_x + max_x) / 2 - (cam->width / 2);

    if (desired_x < 0) desired_x = 0;
    if (desired_x + cam->width > cam->world_width) {
        desired_x = cam->world_width - cam->width;
    }

    cam->x = desired_x;

    // Y is left alone for vertical play / falling behavior
    // or can be updated separately later if desired
}