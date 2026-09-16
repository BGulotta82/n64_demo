#ifndef ENGINE_H
#define ENGINE_H

#include "structs.h"
#include "level.h"
#include "camera.h"
#include "input.h"
#include "character.h"
#include "projectile.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>


void engine_init(game_state_t *state);
void engine_update(game_state_t *state, float dt);
void spawn_new_enemies(game_state_t *state, int num_enemies_to_spawn, character *self);
void load_stage_by_index(game_state_t *state, int index);
void check_new_player_spawn(character *self, character *players, level_t *level, input_state *input, int id);
void spawn_new_player(character *self, character *players, level_t *level);
void check_pve_combat(game_state_t *state, float dt);
void check_melee_collisions(game_state_t *state);
void check_projectile_collisions(game_state_t *state);
void simulate_enemy_ai(character *enemy, const game_state_t *state, input_state *dummy_input, float dt);
void ai_behavior_skeleton(character *enemy, const character *target, float distance, bool hit_cliff_edge, input_state *dummy_input, bool *move_left, bool *move_right);
void ai_behavior_goomba(const character *enemy, const character *target, bool *move_left, bool *move_right);
void update_character_animation_state(character *self);
bool get_character_secondary_hitbox(const character *chr, rect_t *out_rect);

#endif // ENGINE_H
