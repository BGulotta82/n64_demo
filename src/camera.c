#include "camera.h"

void camera_init(camera_t *cam, int world_width, int world_height, int screen_width, int screen_height) {
    cam->x = 0;
    cam->y = 0;
    cam->width = screen_width;
    cam->height = screen_height;
    cam->world_width = world_width;
    cam->world_height = world_height;
}

void camera_update(camera_t *cam, int *player_x, int *player_y, int player_count) {
    int min_x = 999999;
    int min_y = 999999;
    int max_x = -999999;
    int max_y = -999999;

    for (int i = 0; i < player_count; i++) {
        if (player_x[i] < min_x) min_x = player_x[i];
        if (player_y[i] < min_y) min_y = player_y[i];
        if (player_x[i] > max_x) max_x = player_x[i];
        if (player_y[i] > max_y) max_y = player_y[i];
    }

    int center_x = (min_x + max_x) / 2;
    int center_y = (min_y + max_y) / 2;

    cam->x = center_x - cam->width / 2;
    cam->y = center_y - cam->height / 2;

    if (cam->x < 0) cam->x = 0;
    if (cam->y < 0) cam->y = 0;
    if (cam->x + cam->width > cam->world_width) cam->x = cam->world_width - cam->width;
    if (cam->y + cam->height > cam->world_height) cam->y = cam->world_height - cam->height;
}