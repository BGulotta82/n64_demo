#include "engine.h"
#include <string.h>

void engine_init(game_state_t *state) {
    memset(state, 0, sizeof(*state));
}

void engine_update(game_state_t *state, float dt) {
    (void)dt;
    state->frame++;
}
