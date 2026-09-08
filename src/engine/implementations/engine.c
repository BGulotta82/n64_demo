#include "engine.h"
#include <string.h>

void engine_init(game_state_t *state) {
    memset(state, 0, sizeof(*state));
    input_init(&state->input);
    character_init(&state->player1);
}

void engine_update(game_state_t *state, float dt) {
    (void)dt;
    input_update(&state->input);
    character_update(&state->player1, &state->input);
    state->frame++;
}
    