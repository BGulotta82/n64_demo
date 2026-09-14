#ifndef CAMERA_H
#define CAMERA_H

#include "structs.h"
#include "constants.h"
#include <stdbool.h>

extern viewport_layout_t viewport_configs[4][4]; // Declare its shape, but do not assign values here!

// Your other prototypes
extern camera_t cameras[4];

void camera_init(camera_t *cam, float world_width, float world_height, float screen_width, float screen_height, float spawn_x, float spawn_y);
void camera_update_split(camera_t *cam, float p_x, float p_y, float p_w, float p_h, float view_w, float view_h, facing_dir direction, float dt);

#endif