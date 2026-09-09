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

static bool group_would_fit_in_camera(const game_state_t *state, int player_index, int proposed_x, int proposed_y) {
    int active_count = 0;
    int min_x = 999999;
    int min_y = 999999;
    int max_x = -999999;
    int max_y = -999999;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!state->players[i].active) {
            continue;
        }

        active_count++;

        int px = state->players[i].x;
        int py = state->players[i].y;

        if (i == player_index) {
            px = proposed_x;
            py = proposed_y;
        }

        int left = px;
        int right = px + PLAYER_WIDTH;
        int top = py;
        int bottom = py + PLAYER_HEIGHT;

        if (left < min_x) min_x = left;
        if (top < min_y) min_y = top;
        if (right > max_x) max_x = right;
        if (bottom > max_y) max_y = bottom;
    }

    if (active_count == 0) {
        return true;
    }

    // Candidate camera centered on the active group.
    int candidate_x = ((min_x + max_x) / 2) - (camera.width / 2);
    int candidate_y = ((min_y + max_y) / 2) - (camera.height / 2);

    if (candidate_x < 0) candidate_x = 0;
    if (candidate_y < 0) candidate_y = 0;
    if (candidate_x + camera.width > camera.world_width) {
        candidate_x = camera.world_width - camera.width;
    }
    if (candidate_y + camera.height > camera.world_height) {
        candidate_y = camera.world_height - camera.height;
    }

    // Reject if any active player would be outside that camera.
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!state->players[i].active) {
            continue;
        }

        int px = state->players[i].x;
        int py = state->players[i].y;

        if (i == player_index) {
            px = proposed_x;
            py = proposed_y;
        }

        int left = px;
        int right = px + PLAYER_WIDTH;
        int top = py;
        int bottom = py + PLAYER_HEIGHT;

        if (left < candidate_x || right > candidate_x + camera.width ||
            top < candidate_y || bottom > candidate_y + camera.height) {
            return false;
        }
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

        if (!group_would_fit_in_camera(state, i, state->players[i].x, state->players[i].y)) {
            state->players[i].x = old_x;
            state->players[i].y = old_y;
        }
    }

    state->frame++;
}
