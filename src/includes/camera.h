#ifndef CAMERA_H
#define CAMERA_H

#include <stdbool.h>

typedef struct {
    int x;
    int y;
    int width;
    int height;
    int world_width;
    int world_height;
} camera_t;

void camera_init(camera_t *cam, int world_width, int world_height, int screen_width, int screen_height, float spawn_x, float spawn_y);
void camera_update(camera_t *cam, int *player_x, int *player_y, bool *player_active, int player_count);

#endif