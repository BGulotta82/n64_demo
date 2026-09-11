#ifndef ENGINE_H
#define ENGINE_H

#include "level.h"
#include "input.h"
#include "character.h"

typedef struct {
    int frame;
    input_state input[MAX_PLAYERS];
    character players[MAX_PLAYERS];
    character enemies[MAX_ENEMIES];
    level_t level; 
} game_state_t;

void engine_init(game_state_t *state);
void engine_update(game_state_t *state, float dt);
void check_new_player_spawn(game_state_t *state, int i);
void spawn_new_player(game_state_t *state, character_type type, int i);
void simulate_enemy_ai(character *enemy, const game_state_t *state, input_state *dummy_input);
void check_pve_combat(game_state_t *state);

#endif // ENGINE_H
