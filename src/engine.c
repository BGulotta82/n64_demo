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

typedef struct {
    int frame_count;   // Abstract number of frames in this action
    int frame_duration;// How many game ticks to hold each frame
} anim_config_t;

// Define the static constant table mapping parameters directly to the character type index
const animation_profile_t character_animation_profiles[CHAR_TYPE_MAX] = {
    [KNIGHT]   = { -4.0f, -0.5f, 0.5f, 4.0f, 0.1f },
    [ELF]      = { -5.0f, -0.7f, 0.7f, 5.0f, 0.08f }, // Fast/light archetype adjustments
    [WIZARD]   = { -3.5f, -0.4f, 0.4f, 3.5f, 0.12f },
    [DWARF]    = { -2.5f, -0.3f, 0.3f, 2.5f, 0.15f },
    [GOOMBA]   = { -2.0f, -0.2f, 0.2f, 2.0f, 0.05f },
    [SKELETON] = { -3.0f, -0.5f, 0.5f, 3.0f, 0.1f }
};

// Add your complete mapping table to cover all 4 types safely
static const anim_config_t character_anims[CHAR_TYPE_MAX][NUMBER_OF_ANIMATION_STATES] = {
    [KNIGHT] = {
        [ANIM_IDLE]   = { .frame_count = 4,  .frame_duration = 8 },
        [ANIM_WALK]   = { .frame_count = 7,  .frame_duration = 6 },
        [ANIM_ATTACK] = { .frame_count = 12, .frame_duration = 4 },
        [ANIM_JUMP]   = { .frame_count = 5,  .frame_duration = 0 } // Duration 0: Velocity handles this explicitly
    },
    [ELF] = {
        [ANIM_IDLE]   = { .frame_count = 4,  .frame_duration = 8 },
        [ANIM_WALK]   = { .frame_count = 7,  .frame_duration = 6 },
        [ANIM_ATTACK] = { .frame_count = 12, .frame_duration = 4 },
        [ANIM_JUMP]   = { .frame_count = 5,  .frame_duration = 0 } // Duration 0: Velocity handles this explicitly
    },
    [WIZARD] = {
        [ANIM_IDLE]   = { .frame_count = 4,  .frame_duration = 8 },
        [ANIM_WALK]   = { .frame_count = 7,  .frame_duration = 6 },
        [ANIM_ATTACK] = { .frame_count = 12, .frame_duration = 4 },
        [ANIM_JUMP]   = { .frame_count = 5,  .frame_duration = 0 } // Duration 0: Velocity handles this explicitly
    },
    [DWARF] = {
        [ANIM_IDLE]   = { .frame_count = 4,  .frame_duration = 8 },
        [ANIM_WALK]   = { .frame_count = 7,  .frame_duration = 6 },
        [ANIM_ATTACK] = { .frame_count = 12, .frame_duration = 4 },
        [ANIM_JUMP]   = { .frame_count = 5,  .frame_duration = 0 } // Duration 0: Velocity handles this explicitly
    },
    [GOOMBA] = {
        [ANIM_IDLE]   = { .frame_count = 4,  .frame_duration = 8 },
        [ANIM_WALK]   = { .frame_count = 7,  .frame_duration = 6 },
        [ANIM_ATTACK] = { .frame_count = 12, .frame_duration = 4 },
        [ANIM_JUMP]   = { .frame_count = 5,  .frame_duration = 0 } // Duration 0: Velocity handles this explicitly
    },
    [SKELETON] = {
        [ANIM_IDLE]   = { .frame_count = 4,  .frame_duration = 8 },
        [ANIM_WALK]   = { .frame_count = 7,  .frame_duration = 6 },
        [ANIM_ATTACK] = { .frame_count = 12, .frame_duration = 4 },
        [ANIM_JUMP]   = { .frame_count = 5,  .frame_duration = 0 } // Duration 0: Velocity handles this explicitly
    }
};

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
    check_pve_combat(state, dt);

    if (state->match_state == STATE_PLAYING) {
        
        // --- TIMEOUT PRIORITY GATE (LIFTED OUTSIDE OF SPAWN CHECKS) ---
        // If the clock drops to zero or below, freeze the clock and force a hard DEFEAT state immediately.
        if (state->level_timer <= 0.001f) {
            state->level_timer = 0.0f; // Clamp clock visual for your draw_hud string
            state->match_state = STATE_GAME_OVER;
        }        
        // --- SURVIVOR OR SPONTANEOUS DROP-IN CONDITION TRACKING ---
        // Only evaluate standard field tracking flags if a player isn't in mid-spawn transition
        else if (!player_spawned) {
            // Trigger a victory if all enemies are dead and active players are present on screen
            if (state->total_enemies_left == 0 && active_players > 0) {
                state->match_state = STATE_LEVEL_CLEARED;
            }
            // Trigger a defeat if all drop-in players have completely run out of lives and died
            else if (active_players == 0) {
                state->match_state = STATE_GAME_OVER;
            }
        }
        // Fallback: If players died while someone was spawning, but the timer is safe, 
        // the drop-in player preserves the match lifecycle cleanly.
        else if (active_players == 0 && player_spawned) {
            // Keep state playing so the new player drops down from the sky smoothly!
        }
    }

        // =========================================================================
    // 4. --- UPDATE ANIMATION STATES (PLACE HERE) ---
    // =========================================================================
    // At this exact point, all movement, collisions, gravity adjustments, and 
    // combat knockbacks have locked down. We can now safely read the final positions.
    
    // Process all active players
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].meta.state & ACTIVE) {
            update_character_animation_state(&state->players[i]);
        }
    }

    // Process all active enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (state->enemies[i].meta.state & ACTIVE) {
            update_character_animation_state(&state->enemies[i]);
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
        //character_type type = (rand() % 4) + 1; 
        character_type type = KNIGHT; 

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
    // 1. FIXED MULTI-VIEWPORT CHECK (Decoupled from live player life status)
    // =========================================================================
    bool visible_in_any_viewport = false;
    float buffer = 32.0f; 

    // ALWAYS scan through all physical cameras up to MAX_PLAYERS.
    // Do NOT stop scanning a camera viewport just because its player died!
    for (int v = 0; v < MAX_PLAYERS; v++) {
        // If your camera system has a structural flag for active screens (e.g. split screen active)
        // check it here. Otherwise, let it read the persistent layout data.
        
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

    // If completely hidden across all active layout windows, drop simulation tasks
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
            wants_move_right = (enemy->meta.current_frame % 2 == 0); 
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
    // 9. FIXED TYPO & COMMIT FINAL ACTIONS TO DUMMY INPUT REGISTER
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

void check_pve_combat(game_state_t *state, float dt) {
    for (int p = 0; p < MAX_PLAYERS; p++) {
        character *player = &state->players[p];
        if (!(player->meta.state & ACTIVE)) continue;

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

            // 1. STANDARD BOX OVERLAP CHECK
            if (p_x < e_x + enemy->meta.width && 
                p_x + player->meta.width > e_x && 
                p_y < e_y + enemy->meta.height && 
                p_y + player->meta.height > e_y) {

                // Calculate current bottom of the player
                float player_bottom = player->y + (float)player->meta.height;
                
                // TUNNELING FIX: Calculate where the player's feet were BEFORE physics moved them this frame
                float player_bottom_previous = player_bottom - (initial_frame_vy * dt);
                
                // Define the top zone of the enemy (upper 25%)
                float enemy_stomp_threshold = enemy->y + ((float)enemy->meta.height * 0.25f);

                // CORNER-SNAG FIX: Check if player's horizontal center is actually landing over the enemy body
                float p_center_x = player->x + ((float)player->meta.width / 2.0f);
                bool is_over_enemy_horizontally = (p_center_x >= (float)e_x - 4.0f) && 
                                                  (p_center_x <= (float)(e_x + enemy->meta.width) + 4.0f);

                // 2. STOMP CONDITION
                // True if: Falling down AND horizontally aligned AND (was above threshold last frame OR is within current tolerance)
                if (initial_frame_vy >= 0.0f && is_over_enemy_horizontally &&
                    (player_bottom_previous <= enemy_stomp_threshold || player_bottom <= enemy_stomp_threshold + 4.0f)) {
                    
                    enemy->meta.health--;
                    if (enemy->meta.health <= 0) {
                        enemy->meta.state &= ~ACTIVE;
                    }

                    // CRITICAL DIRECTION FIX: Screen-space bounce up
                    player->physics.vy = -fabsf(player->physics.jump_force) * 0.75f;
                    player->physics.state &= ~GROUNDED;
                    player->physics.state |= JUMPING;
                    player->meta.coyote_frames = 0;
                    
                    // DOUBLE-FRAMING FIX: Give the player a tiny window of safety (10 frames) so 
                    // they don't get hurt by the same enemy hitbox on the next frame while moving upwards.
                    player->meta.invincibility_frames = 10; 
                    
                    registered_stomp_this_frame = true;
                    continue; // Successfully stomped; bypass damage check for this enemy slot
                }

                // 3. HURT CHECK
                // Only processes if no stomp was registered anywhere during this loop iteration
                if (!registered_stomp_this_frame && player->meta.invincibility_frames == 0) {
                    player->meta.health--;
                    if (player->meta.health <= 0) {
                        player->meta.health = 0; // Absolute clamp
                        player->meta.state &= ~ACTIVE;
                        break; // Exit enemy loop completely; player is dead
                    }

                    // Midpoint displacement horizontal knockback
                    float e_center_x = enemy->x + ((float)enemy->meta.width / 2.0f);
                    if (p_center_x < e_center_x) {
                        player->physics.vx = -120.0f;
                    } else {
                        player->physics.vx = 120.0f;
                    }

                    // Screen-space directional damage bounce pop up
                    player->physics.vy = -100.0f;
                    player->physics.state &= ~GROUNDED;
                    player->meta.invincibility_frames = 60;
                }
            }
        }
    }
}

void update_character_animation_state(character *self) {
    const animation_profile_t *prof = self->meta.anim_profile;
    if (!prof) prof = &character_animation_profiles[self->meta.type];

    anim_state_t previous_anim = self->meta.current_anim;
    bool process_time_based_ticker = true;

    // Is the character truly resting on something solid? (World tile OR a teammate)
    bool is_on_solid_surface = (self->physics.state & GROUNDED) || (self->meta.state & SUPPORTED_BY_PLAYER);

    // =========================================================================
    // PHASE 1: EVALUATE & CHOOSE STATE
    // =========================================================================
    
    // 1. Attack override takes ultimate priority
    if (self->meta.current_anim == ANIM_ATTACK) {
        const anim_config_t *atk_cfg = &character_anims[self->meta.type][ANIM_ATTACK];
        if (self->meta.current_frame_index < atk_cfg->frame_count - 1) {
            process_time_based_ticker = true; 
        } else {
            // Attack loop finished, switch to appropriate rest state
            self->meta.current_anim = is_on_solid_surface ? ANIM_IDLE : ANIM_JUMP;
        }
    }
    
    // 2. Air states run ONLY if we are floating completely in mid-air
    if (self->meta.current_anim != ANIM_ATTACK && !is_on_solid_surface) {
        self->meta.current_anim = ANIM_JUMP;
        float vy = self->physics.vy;

        if (vy < prof->jump_fast_up_threshold) {
            self->meta.current_frame_index = 0;
        } 
        else if (vy < prof->jump_slow_up_threshold) {
            self->meta.current_frame_index = 1;
        } 
        else if (vy >= -prof->apex_threshold && vy <= prof->apex_threshold) {
            self->meta.current_frame_index = 2;
        } 
        else if (vy <= prof->fall_slow_down_threshold) {
            self->meta.current_frame_index = 3;
        } 
        else {
            self->meta.current_frame_index = 4;
        }
        
        self->meta.anim_timer = 0;
        process_time_based_ticker = false;
    }
    
    // 3. Ground states (Idle or Walk) run if resting safely on world tiles OR on a teammate
    else if (self->meta.current_anim != ANIM_ATTACK) {
        if (fabsf(self->physics.vx) > prof->walk_deadzone) {
            self->meta.current_anim = ANIM_WALK;
        } else {
            self->meta.current_anim = ANIM_IDLE;
        }
    }

    // State Transition Reset
    if (self->meta.current_anim != previous_anim) {
        self->meta.anim_timer = 0;
        self->meta.current_frame_index = 0;
    }

    // =========================================================================
    // PHASE 2: PROGRESS GROUND ANIMATION TICKERS
    // =========================================================================
    if (process_time_based_ticker) {
        const anim_config_t *cfg = &character_anims[self->meta.type][self->meta.current_anim];
        
        if (cfg->frame_duration > 0) {
            self->meta.anim_timer++;
            if (self->meta.anim_timer >= cfg->frame_duration) {
                self->meta.anim_timer = 0;
                self->meta.current_frame_index = (self->meta.current_frame_index + 1) % cfg->frame_count;
            }
        }
    }

    self->meta.current_frame++;
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
            (float)(MAP_WIDTH * TILE_SIZE),        // World map bounds metrics
            (float)(MAP_HEIGHT * TILE_SIZE), 
            (float)SCREEN_WIDTH,                 // Full screen window dimensions as baseline seed
            (float)SCREEN_HEIGHT, 
            state->level.spawn_x,         // Safe spawn origin values
            state->level.spawn_y
        );
    }
}