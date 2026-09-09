#ifndef RENDERER_H
#define RENDERER_H

#include "engine.h"

void renderer_init(void);
void renderer_draw(const game_state_t *state);

void draw_background();

void draw_character(const game_state_t *state);

void draw_level();

#endif // RENDERER_H
