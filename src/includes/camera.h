#ifndef CAMERA_H
#define CAMERA_H

#include <stdint.h>

typedef struct {
    int x;
    int y;
    int width;
    int height;
    int world_width;
    int world_height;
} camera_t;

void camera_init(camera_t *cam, int world_width, int world_height, int screen_width, int screen_height);
void camera_update(camera_t *cam, int *player_x, int *player_y, int player_count);

#endif