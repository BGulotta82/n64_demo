#ifndef RENDERER_H
#define RENDERER_H

#include "engine.h"

void renderer_init(void);
void renderer_draw(surface_t *disp, const game_state_t *state);
void draw_single_character(const character *chr, const camera_t *active_cam, int off_x, int off_y, int view_w, int view_h);
void draw_map_tiles(const level_t *level, const camera_t *active_cam, int off_x, int off_y, int view_w, int view_h);
void draw_hud(const game_state_t *state);
void draw_dynamic_split_screen(const game_state_t *state);
void debug_draw_character_hitbox(const character *chr, const camera_t *active_cam, int off_x, int off_y, uint32_t color_rgba);
void debug_render_character_telemetry(const character *c, float x, float y);
#endif // RENDERER_H
