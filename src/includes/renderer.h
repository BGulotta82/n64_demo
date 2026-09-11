#ifndef RENDERER_H
#define RENDERER_H

#include "engine.h"

void renderer_init(void);
void renderer_draw(surface_t *disp, const game_state_t *state);
void draw_characters(const game_state_t *state);
void draw_single_character(const character *chr);
void draw_level();
void draw_hud(const game_state_t *state);
#endif // RENDERER_H
