#ifndef ENGINE_H
#define ENGINE_H

#include "character.h"
#include "input.h"

typedef struct {
    int frame;
    input_state input;
    character player1;
} game_state_t;

void engine_init(game_state_t *state);
void engine_update(game_state_t *state, float dt);

#endif // ENGINE_H
