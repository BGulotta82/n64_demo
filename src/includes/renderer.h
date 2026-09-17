#ifndef RENDERER_H
#define RENDERER_H

#include "structs.h"
#include "engine.h"
#include <libdragon.h>

void renderer_init(void);
void renderer_draw(surface_t *disp, const game_state_t *state);
void draw_game_state(const game_state_t *state);
void draw_single_character(const character *chr, int cam_x_floor, int cam_y_floor, int off_x, int off_y, int view_w, int view_h, int character_index);
void draw_map_tiles(const level_t *level, int cam_x_floor, int cam_y_floor, int off_x, int off_y, int view_w, int view_h);
void draw_hud(const game_state_t *state);
void draw_dynamic_split_screen(const game_state_t *state);
void debug_render_character_telemetry(const character *c, float x, float y);
void debug_draw_projectiles_hitbox(int cam_x_floor, int cam_y_floor, int off_x, int off_y);
void debug_draw_character_hitbox(const character *chr, int cam_x, int cam_y, int off_x, int off_y, color_t color_rgba);
#endif // RENDERER_H
