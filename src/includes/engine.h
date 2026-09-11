#ifndef ENGINE_H
#define ENGINE_H

#include "level.h"
#include "input.h"
#include "character.h"

typedef enum {
    STATE_WAITING_TO_START,
    STATE_PLAYING,
    STATE_LEVEL_CLEARED,
    STATE_GAME_OVER
} match_state_t;

typedef struct {
    int frame;
    input_state input[MAX_PLAYERS];
    character players[MAX_PLAYERS];
    character enemies[MAX_ENEMIES];
    level_t level; 
    float level_timer;
    int total_enemies_left;
    match_state_t match_state; 
} game_state_t;

void engine_init(game_state_t *state);
void engine_update(game_state_t *state, float dt);
void check_new_player_spawn(game_state_t *state, int i);
void spawn_new_player(game_state_t *state, character_type type, int i);
void simulate_enemy_ai(character *enemy, const game_state_t *state, input_state *dummy_input);
void check_pve_combat(game_state_t *state);

#endif // ENGINE_H
