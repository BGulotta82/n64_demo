#include "engine.h"
#include "camera.h"
#include <string.h>

extern camera_t camera;

void engine_init(game_state_t *state) {
    memset(state, 0, sizeof(*state));

    joypad_init();

    for (int i = 0; i < MAX_PLAYERS; i++) {
        input_init(&state->input[i]);
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

        check_new_player_spawn(state, i);

        if (!state->players[i].active) {
            continue;
        }

        int old_x = state->players[i].x;
     
        character_update(&state->players[i], state->players, &state->input[i], state->level.map_data, dt);

        // Only constrain horizontal movement for same-screen multiplayer.
        if (!group_would_fit_horizontally(state, i, state->players[i].x)) {
            state->players[i].x = old_x;
        }
    }

    // 2. Update your enemies using simulated AI inputs
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!state->enemies[i].active) continue;

      // Create a local, lightweight input instance on the stack for this loop iteration
        input_state simulated_input;
        simulated_input.active_actions = 0; // Clear it to zero clean slate

        simulate_enemy_ai(&state->enemies[i], state, &simulated_input);

        //Run them through the exact same update system!
        //Pass the player array down so enemies can physically interact with players
        character_update(&state->enemies[i], state->players, &simulated_input,state->level.map_data, dt);
    }

    state->frame++;
}

void check_new_player_spawn(game_state_t *state, int i)
{
    bool active = input_update(&state->input[i], i);

    if (active)
    {
        character_type type;
        switch (i)
        {
        case 0:
            type = KNIGHT;            
            break;
        case 1:
            type = ELF;
            break;
        case 2:
            type = WIZARD;
            break;
        case 3:
            type = DWARF;
            break;
        }

        spawn_new_player(state, type, i);
    }
}

void spawn_new_player(game_state_t *state, character_type type, int i)
{
    if (i < 0 || i >= MAX_PLAYERS) return;

    if (!state->players[i].active)
    {
        character_init(&state->players[i], type);
        state->players[i].active = true;

        // --- Player 1 (The Host) Spawns at Level Point ---
        if (i == 0)
        {
            state->players[i].x = state->level.spawn_x;
            state->players[i].y = state->level.spawn_y;
        } 
        // --- Players 2, 3, and 4 Drop In Dynamically ---
        else 
        {
            // Fallback safety check: If Player 1 somehow died or is inactive, use level default
            if (!state->players[0].active) {
                state->players[i].x = state->level.spawn_x;
                state->players[i].y = state->level.spawn_y;
                return;
            }

            bool p1_moving_right = state->players[0].physics.state & MOVING_RGHT;
            float desired_offset = p1_moving_right ? -20.0f : 20.0f; // Tucked slightly closer than 30px
            
            float target_x = state->players[0].x + desired_offset;
            float target_y = state->players[0].y;

            // --- LEVEL BOUNDARY SAFETY WALLS ---
            // Keep late spawns within the map dimensions so they don't spawn off-screen
            if (target_x < 0.0f) target_x = 0.0f;
            if (target_x + PLAYER_WIDTH > (float)(MAP_WIDTH * TILE_SIZE)) {
                target_x = (float)(MAP_WIDTH * TILE_SIZE) - PLAYER_WIDTH;
            }

            // --- TILE OVERLAP PREVENTER ---
            // Sample the tiles where the player's torso would spawn
            int test_tile_x = (int)(target_x + (PLAYER_WIDTH / 2.0f)) / TILE_SIZE;
            int test_tile_y = (int)(target_y + (PLAYER_HEIGHT / 2.0f)) / TILE_SIZE;

            uint8_t target_tile_block = get_tile_at(state->level.map_data, test_tile_x, test_tile_y);

            if (target_tile_block == 2) {
                // If the desired offset is a solid block, bypass the offset completely.
                // This spawns Player 2 EXACTLY inside Player 1's space, safely leveraging 
                // your player-to-player collision code to gently push them apart!
                state->players[i].x = state->players[0].x;
                state->players[i].y = state->players[0].y;
            } else {
                state->players[i].x = target_x;
                state->players[i].y = target_y;
            }

            // Give the new player temporary invincibility frames here if your structural model has them!
            // state->players[i].invincibility_frames = 60;
        }
    }
}

void simulate_enemy_ai(character *enemy, const game_state_t *state, input_state *dummy_input) {
    dummy_input->active_actions = 0;
    if (!enemy->active || !enemy->is_enemy) return;

    // Direct behavior mapping via character_type enum
    if (enemy->type == GOOMBA) {
        // Goomba-style pacing logic
        if (enemy->physics.state & MOVING_LEFT) {
            dummy_input->active_actions |= ACTION_MOVE_LEFT;
        } else {
            dummy_input->active_actions |= ACTION_MOVE_RIGHT;
        }

        // Turn around if walking into a tile block
        if (fabsf(enemy->physics.vx) < 0.01f && (enemy->physics.state & GROUNDED)) {
            if (enemy->physics.state & MOVING_LEFT) {
                enemy->physics.state &= ~MOVING_LEFT;
                enemy->physics.state |= MOVING_RGHT;
            } else {
                enemy->physics.state &= ~MOVING_RGHT;
                enemy->physics.state |= MOVING_LEFT;
            }
        }
    } 
    else if (enemy->type == SKELETON) {
        // Aggressive chasing logic targeting Player 0
        character *target = (character*)&state->players[0];
        if (target->active) {
            if (enemy->x < target->x) dummy_input->active_actions |= ACTION_MOVE_RIGHT;
            else dummy_input->active_actions |= ACTION_MOVE_LEFT;

            // Jump if trying to get over obstacles or close a vertical gap
            if (enemy->y > target->y + 16.0f && (enemy->physics.state & GROUNDED)) {
                dummy_input->active_actions |= ACTION_JUMP;
            }
        }
    }
}
