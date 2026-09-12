#include "engine.h"
#include "camera.h"
#include <string.h>
#include <stdlib.h>

extern camera_t cameras[MAX_VIEWPORTS];

// Group the stage filename and time limit together into a single structure
typedef struct {
    const char *filename;
    float time_limit;
} stage_config_t;

// Define your static level playlist mapping parameters
static const stage_config_t level_playlist[MAX_LEVELS] = {
    { "/level1.bin", 60.0f },
    { "/level1.bin", 60.0f },
    { "/level1.bin", 60.0f }
};

void engine_init(game_state_t *state) {
    memset(state, 0, sizeof(*state));

    state->match_state = STATE_WAITING_TO_START;

    joypad_init();

    for (int i = 0; i < MAX_PLAYERS; i++) {
        input_init(&state->input[i]);
    }
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
     
        character_update(&state->players[i], state->players, &state->input[i], state->level.map_data, dt);
    }

    // 2. Update your enemies using simulated AI inputs
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!(state->enemies[i].meta.state & ACTIVE)) continue;

      // Create a local, lightweight input instance on the stack for this loop iteration
        input_state simulated_input;
        simulated_input.active_actions = 0; // Clear it to zero clean slate

        simulate_enemy_ai(&state->enemies[i], state, &simulated_input, dt);

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

        character_init(self, type, false);
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
        if (target_x + (float)self->meta.width > (float)(MAP_WIDTH * TILE_SIZE)) {
            target_x = (float)(MAP_WIDTH * TILE_SIZE) - (float)self->meta.width;
        }

        // --- TILE OVERLAP PREVENTER ---
        // Sample the tiles where the player's torso would spawn
        int test_tile_x = (int)(target_x + ((float)self->meta.width / 2.0f)) / TILE_SIZE;
        int test_tile_y = (int)(target_y + ((float)self->meta.height / 2.0f)) / TILE_SIZE;

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

void simulate_enemy_ai(character *enemy, const game_state_t *state, input_state *dummy_input, float dt) {
    dummy_input->active_actions = 0;
    if (!(enemy->meta.state & ACTIVE) || !enemy->meta.is_enemy) return;

    // =========================================================================
    // 1. DYNAMIC MULTI-VIEWPORT CHECK (Enforced across all active cameras)
    // =========================================================================
    // First, count how many players/viewports are currently active in the match
    int active_viewports = 0;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].meta.state & ACTIVE) active_viewports++;
    }
    
    // Default fallback check safety gate
    if (active_viewports == 0) active_viewports = 1;

    bool visible_in_any_viewport = false;
    float buffer = 32.0f; 

    // Scan every active camera footprint to look for this enemy
    for (int v = 0; v < active_viewports; v++) {
        // Read directly out of your global cameras configuration array
        float cam_left   = (float)cameras[v].x;
        float cam_right  = (float)(cameras[v].x + cameras[v].width);
        float cam_top    = (float)cameras[v].y;
        float cam_bottom = (float)(cameras[v].y + cameras[v].height);

        // Check if the enemy overlaps this specific viewport window boundary layout
        if (enemy->x >= (cam_left - buffer)  && enemy->x <= (cam_right + buffer) &&
            enemy->y >= (cam_top - buffer)   && enemy->y <= (cam_bottom + buffer)) {
            visible_in_any_viewport = true;
            break; // Found it! Exit early to save tracking processing cycles
        }
    }

    // If completely hidden across all active player windows, drop simulation tasks
    if (!visible_in_any_viewport) {
        enemy->meta.state &= ~SPAWNED; 
        return; 
    }

    // =========================================================================
    // 2. TIMING LAZY INITIALIZATION: Lock home row upon hitting screen view
    // =========================================================================
    if (!(enemy->meta.state & SPAWNED)) {
        enemy->meta.ai_home_row = (int)floorf((enemy->y + (float)enemy->meta.height + 4.0f) / (float)TILE_SIZE);
        enemy->meta.state |= SPAWNED; 
    }

    // =========================================================================
    // 3. --- SELF-HEALING HOME ROW SAFETY TRACKER ---
    // =========================================================================
    int enemy_home_row = enemy->meta.ai_home_row;
    int enemy_actual_current_row = (int)floorf((enemy->y + (float)enemy->meta.height + 4.0f) / (float)TILE_SIZE);

    if (enemy_actual_current_row != enemy_home_row) {
        if (enemy->physics.state & GROUNDED) {
            enemy->meta.ai_home_row = enemy_actual_current_row;
            enemy_home_row = enemy_actual_current_row; 
        } else {
            return; 
        }
    }

    // =========================================================================
    // 4. TARGET TRACKING: Find nearest player on the matching platform row
    // =========================================================================
    character *closest_player = NULL;
    float min_distance = 999999.0f;

    for (int p = 0; p < MAX_PLAYERS; p++) {
        const character *player = &state->players[p];
        if (!(player->meta.state & ACTIVE)) continue;

        int player_current_row = (int)floorf((player->y + (float)player->meta.height + 4.0f) / (float)TILE_SIZE);

        if (player_current_row != enemy_home_row) {
            continue; 
        }

        float dist_x = fabsf(player->x - enemy->x);
        if (dist_x < min_distance) {
            min_distance = dist_x;
            closest_player = (character *)player;
        }
    }

    // =========================================================================
    // 5. BASE DIRECTIONAL INTENT LOGIC WITH IDLE PATROL FALLBACK
    // =========================================================================
    bool wants_move_left = false;
    bool wants_move_right = false;

    if (closest_player == NULL) {
        if (enemy->physics.vx > 0.1f) {
            wants_move_right = true;
        } else if (enemy->physics.vx < -0.1f) {
            wants_move_left = true;
        } else {
            wants_move_right = (enemy->meta.frame % 2 == 0); 
            wants_move_left  = !wants_move_right;
        }
    } 
    else {
        if (enemy->meta.type == GOOMBA) {
            ai_behavior_goomba(enemy, closest_player, &wants_move_left, &wants_move_right);
        }
        else if (enemy->meta.type == SKELETON) {
            if (enemy->x < closest_player->x - 4.0f) {
                wants_move_right = true;
            } else if (enemy->x > closest_player->x + 4.0f) {
                wants_move_left = true;
            }
        }
    }

    // =========================================================================
    // 6. RADAR EDGE SCANNER: Verify floor stability ahead (DYNAMIC VELOCITY LOGIC)
    // =========================================================================
    bool hit_cliff_edge = false;
    int ground_tile_y = (int)floorf((enemy->y + (float)enemy->meta.height + 4.0f) / (float)TILE_SIZE); 

    float dynamic_forward_look = 4.0f + (fabsf(enemy->physics.vx) * dt);

    if (ground_tile_y < MAP_HEIGHT) {
        if (wants_move_right) {
            int check_x = (int)floorf((enemy->x + (float)enemy->meta.width + dynamic_forward_look) / (float)TILE_SIZE);
            if (check_x < MAP_WIDTH) {
                if (state->level.map_data[ground_tile_y * MAP_WIDTH + check_x] == 0) { 
                    wants_move_right = false;
                    hit_cliff_edge = true;
                }
            }
        }
        else if (wants_move_left) {
            int check_x = (int)floorf((enemy->x - dynamic_forward_look) / (float)TILE_SIZE);
            if (check_x >= 0) {
                if (state->level.map_data[ground_tile_y * MAP_WIDTH + check_x] == 0) { 
                    wants_move_left = false;
                    hit_cliff_edge = true;
                }
            }
        }
    }

    // Turn patrolling units around smoothly if they strike an unmapped edge
    if (hit_cliff_edge && closest_player == NULL) {
        if (enemy->physics.vx > 0.0f) {
            wants_move_left = true;
            wants_move_right = false;
        } else {
            wants_move_right = true;
            wants_move_left = false;
        }
    }

    // =========================================================================
    // 7. DELEGATE COMPLEX SUB-BEHAVIORS (Passing down locked radar flags)
    // =========================================================================
    if (enemy->meta.type == SKELETON && closest_player != NULL) {
        ai_behavior_skeleton(
            enemy, 
            closest_player, 
            min_distance, 
            hit_cliff_edge, 
            dummy_input, 
            &wants_move_left, 
            &wants_move_right
        );
    }

    // =========================================================================
    // 8. ANTI-STACKING CROWD CONTROL: Keep spacing between entities clean
    // =========================================================================
    float personal_space_buffer = 24.0f; 
    for (int e = 0; e < MAX_ENEMIES; e++) {
        const character *other = &state->enemies[e];
        if (other == enemy || !(other->meta.state & ACTIVE) || !(other->meta.state & SPAWNED)) continue;

        if (fabsf(other->y - enemy->y) < 16.0f) {
            float dx = other->x - enemy->x;
            if (wants_move_right && dx > 0.0f && dx < personal_space_buffer) wants_move_right = false;
            if (wants_move_left && dx < 0.0f && dx > -personal_space_buffer)  wants_move_left = false;
        }
    }

    // =========================================================================
    // 9. COMMIT FINAL ACTIONS TO DUMMY INPUT REGISTER
    // =========================================================================
    if (wants_move_right) dummy_input->active_actions |= ACTION_MOVE_RIGHT;
    if (wants_move_left)  dummy_input->active_actions |= ACTION_MOVE_LEFT;
}

void ai_behavior_skeleton(character *enemy, const character *target, float distance, bool hit_cliff_edge, input_state *dummy_input, bool *move_left, bool *move_right) {
    // Only engage if player target falls inside its active horizontal vision radius
    if (distance >= 200.0f) return;

    // Decrement jump interval cooldown frame timer properties natively
    if (enemy->meta.ai_jump_cooldown > 0) {
        enemy->meta.ai_jump_cooldown--;
    }

    // Scenario A: Hit a cliff edge but want to pursue the target over the open gap
    if (hit_cliff_edge) {
        if (enemy->meta.ai_jump_cooldown == 0 && (enemy->physics.state & GROUNDED)) {
            // Restore movement directions to launch forward through the air cleanly
            if (enemy->x < target->x) *move_right = true;
            else *move_left = true;

            dummy_input->active_actions |= ACTION_JUMP;
            enemy->meta.ai_jump_cooldown = 60; // 1-second leap cooldown window
        }
    }
    // Scenario B: Terrain is safe, navigate or scale vertical geometry blocks normal pathing
    else {
        if (enemy->x < target->x - 4.0f) {
            *move_right = true;
        } else if (enemy->x > target->x + 4.0f) {
            *move_left = true;
        }

        // Standard wall collision jumping: Hop up if running into vertical tile boundaries
        if ((enemy->physics.state & GROUNDED) && enemy->meta.ai_jump_cooldown == 0) {
            bool blocked_moving_right = (*move_right && enemy->physics.vx < 0.1f);
            bool blocked_moving_left  = (*move_left && enemy->physics.vx > -0.1f);

            if (blocked_moving_right || blocked_moving_left) {
                dummy_input->active_actions |= ACTION_JUMP;
                enemy->meta.ai_jump_cooldown = 90; // 1.5-second standard jump cooldown
            }
        }
    }
}

void ai_behavior_goomba(const character *enemy, const character *target, bool *move_left, bool *move_right) {
    if (enemy->x < target->x) {
        *move_right = true;
    } else {
        *move_left = true;
    }
}

void check_pve_combat(game_state_t *state) {
    for (int p = 0; p < MAX_PLAYERS; p++) {
        character *player = &state->players[p];
        if (!(player->meta.state & ACTIVE)) continue;

        if (player->meta.invincibility_frames > 0) {
            player->meta.invincibility_frames--;
        }

        // --- CLUSTER SHIELD CONFIGURATION ---
        // Cache initial downward speed before evaluation loop alters it dynamically
        float initial_frame_vy = player->physics.vy;
        bool registered_stomp_this_frame = false;

        for (int e = 0; e < MAX_ENEMIES; e++) {
            character *enemy = &state->enemies[e];
            if (!(enemy->meta.state & ACTIVE)) continue;

            // Round float coordinates to integer bounding boxes for precision checking
            int p_x = (int)(player->x + 0.5f);
            int p_y = (int)(player->y + 0.5f);
            int e_x = (int)(enemy->x + 0.5f);
            int e_y = (int)(enemy->y + 0.5f);

            // FIXED: Bounding box overlap calculation uses unique local meta metrics
            if (p_x < e_x + enemy->meta.width &&
                p_x + player->meta.width > e_x &&
                p_y < e_y + enemy->meta.height &&
                p_y + player->meta.height > e_y) {

                // =========================================================================
                // --- DESIGN CRITERIA: CRISP STOMP WINDOW DETECTOR (DYNAMIC SIZES) ---
                // =========================================================================
                // FIXED: Calculates player bottom edge relative to their specific height
                float player_bottom = player->y + (float)player->meta.height;
                
                // FIXED: Calculates enemy stomp threshold relative to the target's specific height
                float enemy_stomp_threshold = enemy->y + ((float)enemy->meta.height * 0.25f);

                // STOMP CHECK: Uses our cached vertical frame vector to protect cluster overlaps
                if (initial_frame_vy > 0.0f && player_bottom <= enemy_stomp_threshold + 8.0f) {
                    enemy->meta.health--;
                    if (enemy->meta.health <= 0) {
                        enemy->meta.state &= ~ACTIVE;
                    }                        

                    // CRITICAL DIRECTION FIX: Screen-space coordinates require a NEGATIVE 
                    // Y velocity vector to bounce UPWARDS away from the ground plane.
                    player->physics.vy = -fabsf(player->physics.jump_force) * 0.75f;
                    
                    player->physics.state &= ~GROUNDED;
                    player->physics.state |= JUMPING;
                    player->meta.coyote_frames = 0;

                    registered_stomp_this_frame = true;
                    continue; // Safe! Skips out to process the next entity slot
                }               
                // HURT CHECK: Enforces cluster protection so a successful stomp shield won't poison a teammate frame
                else if (!registered_stomp_this_frame && player->meta.invincibility_frames == 0) { 
                    player->meta.health--;
                    if (player->meta.health <= 0) {
                        player->meta.health = 0; // Absolute clamp
                        player->meta.state &= ~ACTIVE;
                        break; // Safely breaks enemy scanning loop for this dead player slot
                    }

                    // FIXED: Calculates midpoint displacement offsets dynamically via local parameters
                    float p_center_x = player->x + ((float)player->meta.width / 2.0f);
                    float e_center_x = enemy->x + ((float)enemy->meta.width / 2.0f);

                    if (p_center_x < e_center_x) {
                        player->physics.vx = -120.0f; 
                    } else {
                        player->physics.vx = 120.0f;  
                    }
                    
                    // Matches screen-space directional damage bounce pop up
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

    // =========================================================================
    // --- MULTI-VIEWPORT CAMERA ARRAY INITIALIZATION ---
    // =========================================================================
    for (int v = 0; v < MAX_VIEWPORTS; v++) {
        camera_init(
            &cameras[v],                  // Pass the address of this specific camera element
            MAP_WIDTH * TILE_SIZE,        // World map bounds metrics
            MAP_HEIGHT * TILE_SIZE, 
            SCREEN_WIDTH,                 // Full screen window dimensions as baseline seed
            SCREEN_HEIGHT, 
            state->level.spawn_x,         // Safe spawn origin values
            state->level.spawn_y
        );
    }
}