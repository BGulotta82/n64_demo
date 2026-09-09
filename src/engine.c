#include "engine.h"
#include <string.h>

void engine_init(game_state_t *state) {
    memset(state, 0, sizeof(*state));

    for (int i = 0; i < MAX_PLAYERS; i++) {
        input_init(&state->input[i]);
        character_init(&state->players[i]);
    }
}

void engine_update(game_state_t *state, float dt) {
    (void)dt;

    for (int i = 0; i < MAX_PLAYERS; i++) {        
        bool active = input_update(&state->input[i], i);        
        if (active) {
            state->players[i].active = true; // Activate player if input is detected
        }

        if (!state->players[i].active) {
            continue; // Skip inactive players
        }

        character_update(&state->players[i], &state->input[i], dt);
    }

    state->frame++;
}
