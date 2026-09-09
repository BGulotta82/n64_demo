#include "engine.h"
#include <string.h>

extern camera_t camera;

void engine_init(game_state_t *state) {
    memset(state, 0, sizeof(*state));

    joypad_init();

    for (int i = 0; i < MAX_PLAYERS; i++) {
        input_init(&state->input[i]);
        character_init(&state->players[i]);
    }
}

static bool group_would_fit_horizontally(const game_state_t *state, int player_index, int proposed_x) {
    int min_x = 999999;
    int max_x = -999999;
    int active_count = 0;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!state->players[i].active) {
            continue;
        }

        active_count++;

        int px = state->players[i].x;
        if (i == player_index) {
            px = proposed_x;
        }

        int left = px;
        int right = px + PLAYER_WIDTH;

        if (left < min_x) min_x = left;
        if (right > max_x) max_x = right;
    }

    if (active_count == 0) {
        return true;
    }

    // only enforce the horizontal screen bounds
    if (min_x < camera.x || max_x > camera.x + camera.width) {
        return false;
    }

    return true;
}

void engine_update(game_state_t *state, float dt) {
    joypad_poll();

    for (int i = 0; i < MAX_PLAYERS; i++) {
        bool active = input_update(&state->input[i], i);

        if (active && !state->players[i].active) {
            state->players[i].active = true;
            if (i > 0) {
                state->players[i].x = state->players[0].x + (i * 20);
                state->players[i].y = state->players[0].y;
            }
        }

        if (!state->players[i].active) {
            continue;
        }

        int old_x = state->players[i].x;
        int old_y = state->players[i].y;

        character_update(&state->players[i], state->players, &state->input[i], dt);

        // Only constrain horizontal movement for same-screen multiplayer.
        if (!group_would_fit_horizontally(state, i, state->players[i].x)) {
            state->players[i].x = old_x;
        }

        // vertical movement is intentionally allowed to continue normally
        // so falling / jumping is not blocked by the screen edge rule
    }

    state->frame++;
}
