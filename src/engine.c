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
        if (!(state->players[i].meta.state && ACTIVE)) {
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
    
    if (state->match_state == STATE_GAME_OVER ||
        state->match_state == STATE_LEVEL_CLEARED) {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            input_update(&state->input[i], i);
            if ((state->match_state == STATE_GAME_OVER || 
                 state->players[i].meta.state & ACTIVE) && 
                 state->input[i].active_actions & ACTION_START) {
                int next_level_index = state->level_index;
                if (state->match_state == STATE_LEVEL_CLEARED) {
                    state->level_index++;
                    next_level_index = state->level_index;
                }
                load_stage_by_index(state, next_level_index);
                return;
            }
        }

        return;
    }

    if (state->match_state == STATE_STAGE_INTRO) {
        // Advance to active gameplay so the next frame runs normally
        state->match_state = STATE_PLAYING;    
        state->frame++;
        return;
    }

    int active_players = 0;
    bool player_spawned = false;

    for (int i = 0; i < MAX_PLAYERS; i++) {

        input_update(&state->input[i], i);

        bool player_active = (state->players[i].meta.state & ACTIVE);

        if (!player_active){
            check_new_player_spawn(&state->players[i], state->players, &state->level, &state->input[i]);
            player_spawned = state->players[i].meta.state & SPAWNED; 
        }

        if (!(state->players[i].meta.state & ACTIVE)) {
            continue;
        }

        if (state->match_state == STATE_WAITING_TO_START) {
            state->match_state =  STATE_PLAYING;
        }

        active_players++;

        int old_x = state->players[i].x;
     
        character_update(&state->players[i], state->players, &state->input[i], state->level.map_data, dt);

        // Only constrain horizontal movement for same-screen multiplayer.
        if (!group_would_fit_horizontally(state, i, state->players[i].x)) {
            state->players[i].x = old_x;
        }
    }

    // 2. Update your enemies using simulated AI inputs
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!(state->enemies[i].meta.state & ACTIVE)) continue;

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
        if (state->enemies[i].meta.state & ACTIVE) {
            state->total_enemies_left++;
        }
    }

    // 3. RESOLVE COMBAT OUTCOMES LAST
    // This evaluates modifications over clean, locked positions
    check_pve_combat(state);

    if (state->match_state == STATE_PLAYING && !player_spawned) {
        
        // Only trigger a victory if enemies were counted as 0 AND players actually exist on screen
        if (state->total_enemies_left == 0 && active_players > 0) {
            state->match_state = STATE_LEVEL_CLEARED;
        }
        // Trigger a defeat if time runs out OR if all drop-in players have completely died out
        else if (state->level_timer <= 0.0f || active_players == 0) {
            state->level_timer = 0.0f; // Clamp clock visual
            state->match_state = STATE_GAME_OVER;
        }
    }

    state->frame++;
}

void check_new_player_spawn(character *self, character *players, level_t *level, input_state *input)
{

    if (input->active_actions & ACTION_START && 
      !(self->meta.state & ACTIVE) && 
      !(self->meta.state & SPAWNED))
    {
        character_type type = (rand() % 4) + 1; 

        character_init(self, type);
        self->meta.state |= ACTIVE;
        self->meta.state |= SPAWNED;
        spawn_new_player(self, players, level);
    }
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
        if (!(players[0].meta.state & ACTIVE)) {
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
    if (!(enemy->meta.state & ACTIVE) || !enemy->meta.is_enemy) return;

    if (!(enemy->meta.state & SPAWNED)) {
        // =========================================================================
        // 0. VIEWPORT CHECK: Only simulate AI if within the camera bounds
        // =========================================================================
        // Note: Adjust 'state->camera' properties here if your struct fields differ.
        float cam_left   = camera.x;
        float cam_right  = camera.x + camera.width;
        float cam_top    = camera.y;
        float cam_bottom = camera.y + camera.height;

        // 32-pixel outer padding so enemies activate seamlessly right before scrolling into view
        float buffer = 32.0f; 

        if (enemy->x < (cam_left - buffer)  || enemy->x > (cam_right + buffer) ||
            enemy->y < (cam_top - buffer)   || enemy->y > (cam_bottom + buffer)) {
            return; // Outside the camera view; leave dummy_input cleared so they stand idle
        }

        enemy->meta.state |= SPAWNED;
    }


    // =========================================================================
    // 1. NEAREST TARGET TRACKING: Find the closest active player
    // =========================================================================
    character *closest_player = NULL;
    float min_distance = 999999.0f;

    for (int p = 0; p < MAX_PLAYERS; p++) {
        const character *player = &state->players[p];
        if (!(player->meta.state & ACTIVE)) continue;

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
    if (enemy->meta.type == GOOMBA) {
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
    else if (enemy->meta.type == SKELETON) {
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
        if (!(player->meta.state & ACTIVE)) continue;

        if (player->meta.invincibility_frames > 0) {
            player->meta.invincibility_frames--;
        }

        // --- CLUSTER FIX STEP 1 ---
        // Cache the player's vertical velocity BEFORE entering the enemy loop.
        // Once the player bounces, their actual .vy becomes negative, but this 
        // cached value keeps the stomp window open for the rest of the cluster on this frame.
        float initial_frame_vy = player->physics.vy;
        bool registered_stomp_this_frame = false;

        for (int e = 0; e < MAX_ENEMIES; e++) {
            character *enemy = &state->enemies[e];
            if (!(enemy->meta.state & ACTIVE)) continue;

            int p_x = (int)(player->x + 0.5f);
            int p_y = (int)(player->y + 0.5f);
            int e_x = (int)(enemy->x + 0.5f);
            int e_y = (int)(enemy->y + 0.5f);

            if (p_x < e_x + (int)PLAYER_WIDTH &&
                p_x + (int)PLAYER_WIDTH > e_x &&
                p_y < e_y + (int)PLAYER_HEIGHT &&
                p_y + (int)PLAYER_HEIGHT > e_y) {

                float player_bottom = player->y + PLAYER_HEIGHT;
                
                // Set threshold to top 25% of the enemy
                float enemy_stomp_threshold = enemy->y + (PLAYER_HEIGHT * 0.25f);

                // --- CLUSTER FIX STEP 2 ---
                // Evaluate the stomp using our CACHED initial vertical velocity
                if (initial_frame_vy > 0.0f && player_bottom <= enemy_stomp_threshold + 8.0f) {
                    enemy->meta.health--;
                    if (enemy->meta.health <= 0) {
                        enemy->meta.state &= ~ACTIVE;
                    }                        

                    // Force the upward bounce vector cleanly
                    player->physics.vy = -fabsf(player->physics.jump_force) * 0.75f;
                    
                    player->physics.state &= ~GROUNDED;
                    player->physics.state |= JUMPING;
                    player->meta.coyote_frames = 0;

                    // Tag that a stomp happened so the player is immune to the rest of the loop
                    registered_stomp_this_frame = true;

                    continue; 
                }               
                // --- CLUSTER FIX STEP 3 ---
                // Only evaluate the hurt check if the player hasn't successfully stomped something on this exact frame
                else if (!registered_stomp_this_frame && player->meta.invincibility_frames == 0) { 
                    player->meta.health--;
                    if (player->meta.health == 0) {
                        player->meta.state &= ~ACTIVE;
                        break; // Break the enemy loop since this player just died
                    }

                    if (player->x + (PLAYER_WIDTH / 2.0f) < enemy->x + (PLAYER_WIDTH / 2.0f)) {
                        player->physics.vx = -120.0f; 
                    } else {
                        player->physics.vx = 120.0f;  
                    }
                    
                    player->physics.vy = -100.0f; 
                    player->physics.state &= ~GROUNDED;
                    player->meta.invincibility_frames = 60; 
                }
            }
        }
    }
}

void load_stage_by_index(game_state_t *state, int index) {
    // Cache the previous match state before we overwrite anything
    match_state_t previous_state = state->match_state;

    if (index < 0 || index >= MAX_LEVELS) {
        index = 0; // Safe fallback boundary clamp
    }
    
    state->level_index = index;
    
    const char *target_file = level_playlist[index].filename;
    float target_time       = level_playlist[index].time_limit;
    load_level_binary(target_file, &state->level, state->enemies);

    state->level_timer = target_time; 
    
    // Initialize camera tracking window properties securely over the fresh geometry
    camera_init(
        &camera, 
        MAP_WIDTH * TILE_SIZE, 
        MAP_HEIGHT * TILE_SIZE, 
        SCREEN_WIDTH, 
        SCREEN_HEIGHT, 
        state->level.spawn_x, 
        state->level.spawn_y
    );

    for (int i = 0; i < MAX_PLAYERS; i++) {
        // If we just cleared a level and this specific player survived (is active), 
        // DO NOT kill them. Keep their active state and health intact!
        if (previous_state == STATE_LEVEL_CLEARED && state->players[i].meta.state & ACTIVE) {
            // Stop horizontal speeds so they don't slide into the new stage uncontrollably
            state->players[i].physics.vx = 0.0f;
            state->players[i].physics.vy = 0.0f;
            state->players[i].physics.state = PHYSICS_NONE;
            state->players[i].meta.coyote_frames = 0;
            state->players[i].meta.jump_buffer_frames = 0;
            state->players[i].meta.invincibility_frames = 0;
            state->players[i].meta.health = 3;
        } 
        else {
            // If it was a Game Over, initial boot, or if the player was dead, 
            // completely deactivate the slot so they must press START to drop back in.
            state->players[i].meta.state &= ~ACTIVE;
            state->players[i].meta.state &= ~SPAWNED;
        }
    }

    // =========================================================================
    // --- REPOSITION LIVING SURVIVORS SECURELY ---
    // =========================================================================
    // Now that the level's fresh state->level.spawn_x and spawn_y are loaded,
    // explicitly position your surviving heroes using your existing safety logic.
    bool survivor = false;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!(state->players[i].meta.state & ACTIVE)) continue;

        survivor = true;
        // Force a clean positioning refresh using your existing multi-player logic
        spawn_new_player(&state->players[i], state->players, &state->level);
    }

    if (!survivor) 
    {
        state->match_state = STATE_WAITING_TO_START;
    } 
    else 
    {
        state->match_state = STATE_STAGE_INTRO;
    }
}
