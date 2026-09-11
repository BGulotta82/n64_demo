#include "engine.h"
#include "camera.h"
#include <string.h>

extern camera_t camera;

// Group the stage filename and time limit together into a single structure
typedef struct {
    const char *filename;
    float time_limit;
} stage_config_t;

// Define your static level playlist mapping parameters
static const stage_config_t level_playlist[MAX_LEVELS] = {
    { "/level1.bin", 99.0f },
    { "/level1.bin", 30.0f },
    { "/level1.bin", 20.0f }
};

void engine_init(game_state_t *state) {
    memset(state, 0, sizeof(*state));

    state->match_state = STATE_WAITING_TO_START;

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

    if (state->match_state != STATE_PLAYING && 
        state->match_state != STATE_WAITING_TO_START) {
        input_update(&state->input[0], 0);
        
        if (state->input[0].active_actions & ACTION_START) {
            if (state->match_state == STATE_GAME_OVER) {
                load_stage_by_index(state, state->level_index); 
            } else {
                load_stage_by_index(state, state->level_index++); 
            }
            return;
        }
        return; 
    }

    int live_players = 0;
    bool player_spawned;

    for (int i = 0; i < MAX_PLAYERS; i++) {

        player_spawned = check_new_player_spawn(state, i);
        if (player_spawned && i == 0){
            state->match_state = STATE_PLAYING;
        }

        if (!state->players[i].active) {
            continue;
        }

        live_players++;

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
        character_update(&state->enemies[i], NULL, &simulated_input, state->level.map_data, dt);
    }

     // 2. Count down your level match timer
    if (state->level_timer > 0.0f && state->match_state == STATE_PLAYING) {
        state->level_timer -= dt;
    }

    // 3. Count remaining active enemies
    state->total_enemies_left = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (state->enemies[i].active) {
            state->total_enemies_left++;
        }
    }

    // 3. RESOLVE COMBAT OUTCOMES LAST
    // This evaluates modifications over clean, locked positions
    check_pve_combat(state);

    if (state->match_state == STATE_PLAYING && !player_spawned) {
        
        // Only trigger a victory if enemies were counted as 0 AND players actually exist on screen
        if (state->total_enemies_left == 0 && live_players > 0) {
            state->match_state = STATE_LEVEL_CLEARED;
        }
        // Trigger a defeat if time runs out OR if all drop-in players have completely died out
        else if (state->level_timer <= 0.0f || live_players == 0) {
            state->level_timer = 0.0f; // Clamp clock visual
            state->match_state = STATE_GAME_OVER;
        }
    }

    state->frame++;
}

bool check_new_player_spawn(game_state_t *state, int i)
{
    input_update(&state->input[i], i);

    if (state->input[i].active_actions & ACTION_START && !state->players[i].active)
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

        character_init(&state->players[i], type);
        state->players[i].active = true;
        spawn_new_player(&state->players[i], state->players, &state->level);
        return true;
    }

    return false;
}

void spawn_new_player(character *self, character *players, level_t *level)
{
    // --- Player 1 (The Host) Spawns at Level Point ---
    if (self == &players[0])
    {
        self->x = level->spawn_x;
        self->y = level->spawn_y;
    } 
    // --- Players 2, 3, and 4 Drop In Dynamically ---
    else 
    {
        // Fallback safety check: If Player 1 somehow died or is inactive, use level default
        if (!players[0].active) {
            self->x = level->spawn_x;
            self->y = level->spawn_y;
            return;
        }

        bool p1_moving_right = players[0].physics.state & MOVING_RGHT;
        float desired_offset = p1_moving_right ? -20.0f : 20.0f; // Tucked slightly closer than 30px
        
        float target_x = players[0].x + desired_offset;
        float target_y = players[0].y;

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

        uint8_t target_tile_block = get_tile_at(level->map_data, test_tile_x, test_tile_y);

        if (target_tile_block == 2) {
            // If the desired offset is a solid block, bypass the offset completely.
            // This spawns Player 2 EXACTLY inside Player 1's space, safely leveraging 
            // your player-to-player collision code to gently push them apart!
            self->x = players[0].x;
            self->y = players[0].y;
        } else {
            self->x = target_x;
            self->y = target_y;
        }
    }
}        

void simulate_enemy_ai(character *enemy, const game_state_t *state, input_state *dummy_input) {
    dummy_input->active_actions = 0;
    if (!enemy->active || !enemy->is_enemy) return;

    // =========================================================================
    // 1. NEAREST TARGET TRACKING: Find the closest active player
    // =========================================================================
    character *closest_player = NULL;
    float min_distance = 999999.0f;

    for (int p = 0; p < MAX_PLAYERS; p++) {
        const character *player = &state->players[p];
        if (!player->active) continue;

        float dist_x = fabsf(player->x - enemy->x);
        if (dist_x < min_distance) {
            min_distance = dist_x;
            closest_player = (character *)player;
        }
    }

    // If no active players exist in the entire game world, enemies stand completely idle
    if (closest_player == NULL) return;

    // =========================================================================
    // 2. FLAG-FREE BEHAVIOR LOGIC (Modifying dummy_input ONLY)
    // =========================================================================
    
    // --- GOOMBA: Relentless Zombie/Chaser AI ---
    if (enemy->type == GOOMBA) {
        // Simply press Left or Right depending on which side of the enemy the player is on
        if (enemy->x < closest_player->x) {
            dummy_input->active_actions |= ACTION_MOVE_RIGHT;
        } else {
            dummy_input->active_actions |= ACTION_MOVE_LEFT;
        }
        
        // Note: No jump inputs or wall-turning states are tracked. If a Goomba hits a wall,
        // it will continuously push against it until the player jumps over it or walks away!
    } 
    
    // --- SKELETON: Aggressive Agility Chaser AI ---
    else if (enemy->type == SKELETON) {
        // Only engage if the closest player is within its vision radius (e.g., 200 pixels)
        if (min_distance < 200.0f) {
            
            // Advance horizontally toward the target
            if (enemy->x < closest_player->x - 4.0f) {
                dummy_input->active_actions |= ACTION_MOVE_RIGHT;
            } else if (enemy->x > closest_player->x + 4.0f) {
                dummy_input->active_actions |= ACTION_MOVE_LEFT;
            }

            // Only jump if horizontally blocked by a wall OR if the player is noticeably higher up
            bool blocked_by_wall = fabsf(enemy->physics.vx) < 0.1f;
            bool player_is_above = (enemy->y > closest_player->y + 24.0f);

            if ((enemy->physics.state & GROUNDED) && (blocked_by_wall || player_is_above)) {
                dummy_input->active_actions |= ACTION_JUMP;
            }
        }
    }
}

void check_pve_combat(game_state_t *state) {
    for (int p = 0; p < MAX_PLAYERS; p++) {
        character *player = &state->players[p];
        if (!player->active) continue;

        if (player->invincibility_frames > 0) {
            player->invincibility_frames--;
        }

        for (int e = 0; e < MAX_ENEMIES; e++) {
            character *enemy = &state->enemies[e];
            if (!enemy->active) continue;

            // Round float coordinates to integer bounding boxes for precision checking
            int p_x = (int)(player->x + 0.5f);
            int p_y = (int)(player->y + 0.5f);
            int e_x = (int)(enemy->x + 0.5f);
            int e_y = (int)(enemy->y + 0.5f);

            if (p_x < e_x + (int)PLAYER_WIDTH &&
                p_x + (int)PLAYER_WIDTH > e_x &&
                p_y < e_y + (int)PLAYER_HEIGHT &&
                p_y + (int)PLAYER_HEIGHT > e_y) {

                float player_bottom = player->y + PLAYER_HEIGHT;
                float enemy_midpoint = enemy->y + (PLAYER_HEIGHT / 2.0f);

                    // Stomp Check
                    if (player->physics.vy > 0.0f && player_bottom <= enemy_midpoint + 4.0f) {
                        enemy->health--;
                        if (enemy->health <= 0) {
                            enemy->active = false;
                        }                        

                        // Satisfying bounce upward
                        player->physics.vy = player->physics.jump_force * 0.75f;
                        player->physics.state &= ~GROUNDED;
                        player->physics.state |= JUMPING;
                        player->coyote_frames = 0;

                        continue; // ◄ Safe here! Skips the rest of this cell's checks and checks the next enemy asset slot
                    }               
                    else if (player->invincibility_frames == 0) { // Hurt Check
                    player->health--;
                    if (player->health == 0) {
                        player->active = false;
                        break;
                    }

                    if (player->x + (PLAYER_WIDTH / 2.0f) < enemy->x + (PLAYER_WIDTH / 2.0f)) {
                        player->physics.vx = -120.0f; 
                    } else {
                        player->physics.vx = 120.0f;  
                    }
                    player->physics.vy = -100.0f; 
                    player->physics.state &= ~GROUNDED;
                    player->invincibility_frames = 60; 
                }
            }
        }
    }
}

void load_stage_by_index(game_state_t *state, int index) {
    if (index < 0 || index >= MAX_LEVELS) {
        index = 0; 
    }
    
    state->level_index  = index;

    // 1. Clear out active player flags for the level transition
    for (int i = 0; i < MAX_PLAYERS; i++) {
        state->players[i].active = false;
    }

    // 2. Fetch configurations straight from the playlist struct table
    const char *target_file = level_playlist[index].filename;
    float target_time       = level_playlist[index].time_limit;

    // 3. Load map data
    load_level_binary(target_file, &state->level, state->enemies);

    // 4. THE CLEAN SLATE: Assign the time directly to your active countdown clock!
    state->level_timer = target_time; 
    state->total_enemies_left = state->level.number_of_enemies;

    // 5. Initialize camera positions over the parsed world dimensions
    camera_init(
        &camera, 
        MAP_WIDTH * TILE_SIZE, 
        MAP_HEIGHT * TILE_SIZE, 
        SCREEN_WIDTH, 
        SCREEN_HEIGHT, 
        state->level.spawn_x, 
        state->level.spawn_y
    );

    state->match_state = STATE_WAITING_TO_START;
}
