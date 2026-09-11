#include "engine.h"
#include "camera.h"
#include <string.h>
#include <stdlib.h>

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
        if (!(state->players[i].meta.state & ACTIVE)) {
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

void simulate_enemy_ai(character *enemy, const game_state_t *state, input_state *dummy_input, float dt) {
    dummy_input->active_actions = 0;
    if (!(enemy->meta.state & ACTIVE) || !enemy->meta.is_enemy) return;

    // =========================================================================
    // 1. UNIVERSAL VIEWPORT CHECK (Enforced every single frame)
    // =========================================================================
    float cam_left   = camera.x;
    float cam_right  = camera.x + camera.width;
    float cam_top    = camera.y;
    float cam_bottom = camera.y + camera.height;
    float buffer     = 32.0f; 

    if (enemy->x < (cam_left - buffer)  || enemy->x > (cam_right + buffer) ||
        enemy->y < (cam_top - buffer)   || enemy->y > (cam_bottom + buffer)) {
        enemy->meta.state &= ~SPAWNED; // Force un-spawn status if scrolled off-camera
        return; 
    }

    // =========================================================================
    // 2. TIMING LAZY INITIALIZATION: Lock home row upon hitting screen view
    // =========================================================================
    if (!(enemy->meta.state & SPAWNED)) {
        enemy->meta.ai_home_row = (int)floorf((enemy->y + (float)PLAYER_HEIGHT + 4.0f) / (float)TILE_SIZE);
        enemy->meta.state |= SPAWNED; 
    }

    // =========================================================================
    // 3. --- FIXED: SELF-HEALING HOME ROW SAFETY TRACKER ---
    // =========================================================================
    int enemy_home_row = enemy->meta.ai_home_row;
    int enemy_actual_current_row = (int)floorf((enemy->y + (float)PLAYER_HEIGHT + 4.0f) / (float)TILE_SIZE);

    // If the enemy has physically left its home row (due to a fall or knockback)
    if (enemy_actual_current_row != enemy_home_row) {
        
        // Wait until the physics engine flags them as completely stable on solid ground
        if (enemy->physics.state & GROUNDED) {
            // SUCCESS: Adapt to the new platform seamlessly!
            enemy->meta.ai_home_row = enemy_actual_current_row;
            enemy_home_row = enemy_actual_current_row; // Update local tracker variable
        } else {
            // While they are actively falling through mid-air, clear their movement inputs 
            // so they drop straight down smoothly instead of drifting sideways.
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

        int player_current_row = (int)floorf((player->y + (float)PLAYER_HEIGHT + 4.0f) / (float)TILE_SIZE);

        if (player_current_row != enemy_home_row) {
            continue; // Player is on a completely different tracking plane; skip
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
        // Idle Guard Patrol routing using basic physics velocities
        if (enemy->physics.vx > 0.1f) {
            wants_move_right = true;
        } else if (enemy->physics.vx < -0.1f) {
            wants_move_left = true;
        } else {
            // Break dead-center locks rhythmically using your new metadata frame property
            wants_move_right = (enemy->meta.frame % 2 == 0); 
            wants_move_left  = !wants_move_right;
        }
    } 
    else {
        // FIXED: Run intent destination logic for BOTH types BEFORE the cliff radar ticks
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
    int ground_tile_y = (int)floorf((enemy->y + (float)PLAYER_HEIGHT + 4.0f) / (float)TILE_SIZE); 

    // Expand the scanning ray dynamically based on frame speed (vx * dt) to catch fast ticks
    float dynamic_forward_look = 4.0f + (fabsf(enemy->physics.vx) * dt); // Using frame dt parameter passing

    if (ground_tile_y < MAP_HEIGHT) {
        if (wants_move_right) {
            int check_x = (int)floorf((enemy->x + (float)PLAYER_WIDTH + dynamic_forward_look) / (float)TILE_SIZE);
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

        for (int e = 0; e < MAX_ENEMIES; e++) {
            character *enemy = &state->enemies[e];
            if (!(enemy->meta.state & ACTIVE)) continue;

            // Round float coordinates to integer bounding boxes for precision checking
            int p_x = (int)(player->x + 0.5f);
            int p_y = (int)(player->y + 0.5f);
            int e_x = (int)(enemy->x + 0.5f);
            int e_y = (int)(enemy->y + 0.5f);

            if (p_x < e_x + (int)PLAYER_WIDTH &&
                p_x + (int)PLAYER_WIDTH > e_x &&
                p_y < e_y + (int)PLAYER_HEIGHT &&
                p_y + (int)PLAYER_HEIGHT > e_y) {

                // =========================================================================
                // --- DESIGN CRITERIA: CRISP STOMP WINDOW DETECTOR ---
                // =========================================================================
                float player_bottom = player->y + PLAYER_HEIGHT;
                
                // Set the stomp threshold to the top 25% of the enemy structure
                float enemy_stomp_threshold = enemy->y + (PLAYER_HEIGHT * 0.25f);

                // STOMP CHECK: Must be moving down, and feet must be in the top portion of the target
                if (player->physics.vy > 0.0f && player_bottom <= enemy_stomp_threshold + 8.0f) {
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

                    continue; // Safe! Skips out to process the next entity slot
                }               
                else if (player->meta.invincibility_frames == 0) { // Hurt Check
                    player->meta.health--;
                    if (player->meta.health == 0) {
                        player->meta.state &= ~ACTIVE;
                        break;
                    }

                    if (player->x + (PLAYER_WIDTH / 2.0f) < enemy->x + (PLAYER_WIDTH / 2.0f)) {
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