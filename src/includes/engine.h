#ifndef ENGINE_H
#define ENGINE_H

#include "character.h"
#include "input.h"
#include "camera.h"
#include "constants.h"

typedef struct {
    int frame;
    input_state input[MAX_PLAYERS];
    character players[MAX_PLAYERS];
} game_state_t;

void engine_init(game_state_t *state);
void engine_update(game_state_t *state, float dt);

#endif // ENGINE_H
