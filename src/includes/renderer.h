#ifndef RENDERER_H
#define RENDERER_H

#include "engine.h"

void renderer_init(void);
void renderer_draw(surface_t *disp, const game_state_t *state);
void draw_characters(const game_state_t *state);
void draw_level();

#endif // RENDERER_H
