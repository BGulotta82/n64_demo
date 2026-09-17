#include "engine.h"

const int g_enemies_per_spawn_point[MAX_PLAYERS] = { 3,6,8,10 };

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
const anim_config_t character_anims[CHAR_TYPE_MAX][NUMBER_OF_ANIMATION_STATES] = {
    [KNIGHT] = {
        [ANIM_IDLE]   = { .frame_count = 7,  .frame_duration = 8, .hitbox_start_frame = 0, .hitbox_end_frame = 0 },
        [ANIM_WALK]   = { .frame_count = 8,  .frame_duration = 6, .hitbox_start_frame = 0, .hitbox_end_frame = 0  },
        [ANIM_ATTACK] = 
        { 
            .frame_count = 6, .frame_duration = 4, 
            .hitbox_start_frame = 3, .hitbox_end_frame = 4,
            .hitbox = {
                .style = HITBOX_STYLE_MELEE_SWEEP,
                .width = 24.0f, .height = 24.0f, // Large sword swipe
                .offset_x = 18.0f, .offset_y = 8.0f
            }
        },
        [ANIM_JUMP]   = { .frame_count = 5,  .frame_duration = 0, .hitbox_start_frame = 0, .hitbox_end_frame = 0  } // Duration 0: Velocity handles this explicitly
    },
    [ELF] = {
        [ANIM_IDLE]   = { .frame_count = 4,  .frame_duration = 8, .hitbox_start_frame = 0, .hitbox_end_frame = 0  },
        [ANIM_WALK]   = { .frame_count = 7,  .frame_duration = 6, .hitbox_start_frame = 0, .hitbox_end_frame = 0  },
        [ANIM_ATTACK] = 
        {
            .frame_count = 12, .frame_duration = 2, 
            .hitbox_start_frame = 8, .hitbox_end_frame = 8, // Single frame trigger to spawn projectile_t
            .hitbox = {
                .style = HITBOX_STYLE_PROJECTILE,
                .width = 8.0f, .height = 8.0f, // Small spawning point
                .offset_x = 24.0f, .offset_y = 4.0f
            }
        },        
        [ANIM_JUMP]   = { .frame_count = 5,  .frame_duration = 0, .hitbox_start_frame = 0, .hitbox_end_frame = 0  } // Duration 0: Velocity handles this explicitly
    },
    [WIZARD] = {
        [ANIM_IDLE]   = { .frame_count = 23,  .frame_duration = 8, .hitbox_start_frame = 0, .hitbox_end_frame = 0  },
        [ANIM_WALK]   = { .frame_count = 7,  .frame_duration = 6, .hitbox_start_frame = 0, .hitbox_end_frame = 0  },
        [ANIM_ATTACK] = 
        {
            .frame_count = 12, .frame_duration = 2, 
            .hitbox_start_frame = 3, .hitbox_end_frame = 3, // Single frame trigger to spawn projectile_t
            .hitbox = {
                .style = HITBOX_STYLE_PROJECTILE,
                .width = 8.0f, .height = 8.0f, // Small spawning point
                .offset_x = 24.0f, .offset_y = 4.0f
            }
        },        
        [ANIM_JUMP]   = { .frame_count = 5,  .frame_duration = 0, .hitbox_start_frame = 0, .hitbox_end_frame = 0  } // Duration 0: Velocity handles this explicitly
    },
    [DWARF] = {
        [ANIM_IDLE]   = { .frame_count = 4,  .frame_duration = 8, .hitbox_start_frame = 0, .hitbox_end_frame = 0  },
        [ANIM_WALK]   = { .frame_count = 7,  .frame_duration = 6, .hitbox_start_frame = 0, .hitbox_end_frame = 0  },
        [ANIM_ATTACK] = 
        { 
            .frame_count = 12, .frame_duration = 3, 
            .hitbox_start_frame = 5, .hitbox_end_frame = 8,
            .hitbox = {
                .style = HITBOX_STYLE_MELEE_SWEEP,
                .width = 20.0f, .height = 10.0f, // Large sword swipe
                .offset_x = 20.0f, .offset_y = 7.0f
            }
        },
        [ANIM_JUMP]   = { .frame_count = 5,  .frame_duration = 0, .hitbox_start_frame = 0, .hitbox_end_frame = 0  } // Duration 0: Velocity handles this explicitly
    },
    [GOOMBA] = {
        [ANIM_IDLE]   = { .frame_count = 4,  .frame_duration = 8, .hitbox_start_frame = 0, .hitbox_end_frame = 0  },
        [ANIM_WALK]   = { .frame_count = 7,  .frame_duration = 6, .hitbox_start_frame = 0, .hitbox_end_frame = 0  },
        [ANIM_ATTACK] = 
        { 
            .frame_count = 12, .frame_duration = 4, 
            .hitbox_start_frame = 3, .hitbox_end_frame = 6,
            .hitbox = {
                .style = HITBOX_STYLE_BITE,
                .width = 16.0f, .height = 16.0f, // Small, tight collision box
                .offset_x = 4.0f, .offset_y = -2.0f
            }
        },
        [ANIM_JUMP]   = { .frame_count = 5,  .frame_duration = 0, .hitbox_start_frame = 0, .hitbox_end_frame = 0  } // Duration 0: Velocity handles this explicitly
    },
    [SKELETON] = {
        [ANIM_IDLE]   = { .frame_count = 4,  .frame_duration = 8, .hitbox_start_frame = 0, .hitbox_end_frame = 0  },
        [ANIM_WALK]   = { .frame_count = 7,  .frame_duration = 6, .hitbox_start_frame = 0, .hitbox_end_frame = 0  },
        [ANIM_ATTACK] = 
        { 
            .frame_count = 12, .frame_duration = 4, 
            .hitbox_start_frame = 5, .hitbox_end_frame = 8,
            .hitbox = {
                .style = HITBOX_STYLE_MELEE_SWEEP,
                .width = 48.0f, .height = 32.0f, // Large sword swipe
                .offset_x = 16.0f, .offset_y = 0.0f
            }
        },
        [ANIM_JUMP]   = { .frame_count = 5,  .frame_duration = 0, .hitbox_start_frame = 0, .hitbox_end_frame = 0  } // Duration 0: Velocity handles this explicitly
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
    init_player_registry(MAX_PLAYERS);

    state->match_state = STATE_WAITING_TO_START;

    joypad_init();

    for (int i = 0; i < MAX_PLAYERS; i++) {
        input_init(&state->input[i]);
    }
}

void engine_update(game_state_t *state, float dt) {

    joypad_poll();
    
    if (state->match_state == STATE_GAME_OVER ||
        state->match_state == STATE_LEVEL_CLEARED ||
        state->match_state == STATE_WAITING_TO_START) {
        
        input_update(&state->input[0], 0);
        if (state->input[0].active_actions & ACTION_START) {
            int next_level_index = state->level_index;
            if (state->match_state == STATE_LEVEL_CLEARED) {
                next_level_index++;
            }    

            load_stage_by_index(state, next_level_index);
            int player_count = get_player_count();
            if (player_count == 0) {
                add_new_player(0, state);
                state->joined_players++;
            }
            state->match_state = STATE_PLAYING;
        }

        state->frame++;
        return;
    }

    int player_count = get_player_count();

    // do we need to check for new players spawning in?
    int joined_players = state->joined_players;
    if (joined_players < MAX_PLAYERS) {
        int new_player_count = 0;

        for (int i = 0; i <= MAX_PLAYERS-1; i++) {
            input_state *player_input = &state->input[i];
            input_update(player_input, i);     

            character *player = get_player_by_id(i);
            if (player == NULL) {
                // did a new player hit start?
                if (player_input->active_actions & ACTION_START) {
                    add_new_player(i, state);
                    state->joined_players++;
                    new_player_count++;
                }         
            }
        }
    
        if (new_player_count > 0) {
            // do we need to spawn new enemies into the level?
            int num_enemies_to_spawn = g_enemies_per_spawn_point[player_count + new_player_count - 1] - g_enemies_per_spawn_point[player_count - 1];
            add_new_enemies(state, num_enemies_to_spawn);
            state->frame++;
            return;
        }
    }
    
    for (int i = 0; i < player_count; i++) {
        character* player = get_player_by_id(i);
        input_state *player_input = &state->input[i];
        input_update(player_input, i);     
        character_update(player, player_input, state->level.map_data, dt);
    }

    // 2. Update your enemies using simulated AI inputs
    int num_enemies = get_enemy_count();
    
    for (int i = 0; i < num_enemies; i++) {        
        character* enemy = get_enemy_at(i);

        // Create a local, lightweight input instance on the stack for this loop iteration
        input_state simulated_input;
        simulated_input.active_actions = 0; // Clear it to zero clean slate

        simulate_enemy_ai(enemy, state, &simulated_input, dt);

        //Run them through the exact same update system!
        //Pass the player array down so enemies can physically interact with players
        character_update(enemy, &simulated_input, state->level.map_data, dt);
    }

     // 2. Count down your level match timer
    if (state->level_timer > 0.0f && state->match_state == STATE_PLAYING) {
        state->level_timer -= dt;
    }

    update_projectiles(dt);

    // 3. RESOLVE COMBAT OUTCOMES LAST
    // This evaluates modifications over clean, locked positions
    check_pve_combat(state, dt);

    player_count = get_player_count();
    int enemy_count = get_enemy_count();

    if (state->match_state == STATE_PLAYING) {          
        // --- TIMEOUT PRIORITY GATE (LIFTED OUTSIDE OF SPAWN CHECKS) ---
        // If the clock drops to zero or below, freeze the clock and force a hard DEFEAT state immediately.
        if (state->level_timer <= 0.001f || player_count == 0) {
            state->level_timer = 0.0f; // Clamp clock visual for your draw_hud string
            state->match_state = STATE_GAME_OVER;
        }        
        // Trigger a victory if all enemies are dead and active players are present on screen
        if (enemy_count == 0 && player_count > 0) {
            state->match_state = STATE_LEVEL_CLEARED;
        }
    }

    // =========================================================================
    // 4. --- UPDATE ANIMATION STATES (PLACE HERE) ---
    // =========================================================================
    // At this exact point, all movement, collisions, gravity adjustments, and 
    // combat knockbacks have locked down. We can now safely read the final positions.
    
    // Process all active players
    for (int i = 0; i < player_count; i++) {
        character *player = get_player_at(i);
        update_character_animation_state(player);
    }

    // Process all active enemies
    for (int i = 0; i < enemy_count; i++) {
        character* enemy = get_enemy_at(i);
        update_character_animation_state(enemy);
    }
    
    state->frame++;
}

void add_new_player(int player_id, game_state_t *state)
{
    character new_player = {0};
    //character_type type = rand() % 4;
    character_type type = KNIGHT;
    character_init(&new_player, type, false, player_id);
    determine_new_player_coordinates(&new_player, &state->level);
    spawn_player(&new_player);
    
    for(int i = 0; i <= player_id; i++) {
        camera_t *camera = get_camera_at(i);
        if (camera != NULL && i == player_id){
            camera_init(
                        camera,                  // Pass the address of this specific camera element
                        (float)(MAP_WIDTH * TILE_SIZE),        // World map bounds metrics
                        (float)(MAP_HEIGHT * TILE_SIZE), 
                        (float)SCREEN_WIDTH,                 // Full screen window dimensions as baseline seed
                        (float)SCREEN_HEIGHT, 
                        new_player.x,         // Safe spawn origin values
                        new_player.y
                    );            
            break;
        }

        if (camera == NULL) {
            camera_t new_camera = {0};
            camera_init(
                        &new_camera,                  // Pass the address of this specific camera element
                        (float)(MAP_WIDTH * TILE_SIZE),        // World map bounds metrics
                        (float)(MAP_HEIGHT * TILE_SIZE), 
                        (float)SCREEN_WIDTH,                 // Full screen window dimensions as baseline seed
                        (float)SCREEN_HEIGHT, 
                        i == player_id ? new_player.x : state->level.spawn_x,         // Safe spawn origin values
                        i == player_id ? new_player.y : state->level.spawn_y
                    );            
            spawn_camera(&new_camera);
        }
    }
}

void add_new_enemies(game_state_t *state, int num_enemies_to_spawn)
{
    // find furthest active player in the map
    character *furthest_active_player = find_furthest_active_player(NULL);
    camera_t *camera = get_camera_at(furthest_active_player->meta.id);

    int tile_start_idx = (int)((camera->x + camera->width + TILE_SIZE) / TILE_SIZE);

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = tile_start_idx; x < MAP_WIDTH; x++) {
            uint8_t tile_id =  get_tile_at(state->level.map_data, x, y);
            if (tile_id != ENEMY_SPAWN) continue;

            spawn_enemies(num_enemies_to_spawn, x, y);
        }
    }
}

void determine_new_player_coordinates(character *new_player, level_t *level)
{   
    character *furthest = find_furthest_active_player(new_player);

    float target_x = level->spawn_x;
    float target_y = level->spawn_y;
    float desired_offset = 20.0f;
    
    if (furthest != NULL) {
        bool moving_right = furthest->physics.state & MOVING_RIGHT;
        desired_offset = moving_right ? -20.0f : 20.0f;
        
        target_x = furthest->x + desired_offset;
        target_y = furthest->y;
    }

    // 1. CLAMP FIRST: Guarantee negative bounds are completely erased
    if (target_x < 0.0f) {
        target_x = 0.0f;
    }
    if (target_x + (float)new_player->meta.width > (float)(MAP_WIDTH * TILE_SIZE)) {
        target_x = (float)(MAP_WIDTH * TILE_SIZE) - (float)new_player->meta.width;
    }

    // 2. CHECK SOLID TILES SECOND
    int test_tile_x = (int)(target_x + ((float)new_player->meta.width / 2.0f)) / TILE_SIZE;
    int test_tile_y = (int)(target_y + ((float)new_player->meta.height / 2.0f)) / TILE_SIZE;
    uint8_t target_tile_block = get_tile_at(level->map_data, test_tile_x, test_tile_y);

    if (target_tile_block == 2 && furthest != NULL) {
        // Stack perfectly on top of friend if clamped space is a solid wall
        new_player->x = furthest->x;
        new_player->y = furthest->y;
    } 
    else {
        // Safe open air position assignment
        new_player->x = target_x;
        new_player->y = target_y;
    }
}

void simulate_enemy_ai(character *enemy, const game_state_t *state, input_state *dummy_input, float dt) {
    dummy_input->active_actions = 0;
    if (!enemy->meta.is_enemy) return;

    // =========================================================================
    // 1. FIXED MULTI-VIEWPORT CHECK (Decoupled from live player life status)
    // =========================================================================
    bool visible_in_any_viewport = false;
    float buffer = 32.0f; 

    // ALWAYS scan through all physical cameras up to MAX_PLAYERS.
    // Do NOT stop scanning a camera viewport just because its player died!
    int player_count = get_player_count();

    for (int v = 0; v < player_count; v++) {
        // If your camera system has a structural flag for active screens (e.g. split screen active)
        // check it here. Otherwise, let it read the persistent layout data.
        
        character *player = get_player_at(v);
        camera_t *camera = get_camera_at(player->meta.id);

        float cam_left   = (float)camera->x;
        float cam_right  = (float)(camera->x + camera->width);
        float cam_top    = (float)camera->y;
        float cam_bottom = (float)(camera->y + camera->height);

        // Check if the enemy overlaps this specific viewport window boundary layout
        if (enemy->x >= (cam_left - buffer)  && enemy->x <= (cam_right + buffer) &&
            enemy->y >= (cam_top - buffer)   && enemy->y <= (cam_bottom + buffer)) {
            visible_in_any_viewport = true;
            break; // Found it! Exit early to save tracking processing cycles
        }
    }

    // If completely hidden across all active layout windows, drop simulation tasks
    if (!visible_in_any_viewport) {
        return; 
    }

    // =========================================================================
    // 2. TIMING LAZY INITIALIZATION: Lock home row upon hitting screen view
    // =========================================================================
    enemy->meta.ai_home_row = (int)floorf((enemy->y + (float)enemy->meta.height + 4.0f) / (float)TILE_SIZE);

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

    for (int p = 0; p < player_count; p++) {
        const character *player = get_player_at(p);

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
    int enemy_count = get_enemy_count();

    for (int e = 0; e < enemy_count; e++) {
        const character *other = get_enemy_at(e);
        if (other == enemy) continue;

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
    // =========================================================================
    // PHASE 1: PROJECTILE COLLISIONS
    // =========================================================================
    // Loop backward through our dynamic projectile registry to safely handle deletions
    check_projectile_collisions(state);

    // =========================================================================
    // PHASE 2: MELEE AND BODY-TO-BODY COMBAT (Your Original Code)
    // =========================================================================
    check_melee_collisions(state);
}

void check_melee_collisions(game_state_t *state)
{
    int player_count = get_player_count();

    for (int p = 0; p < player_count; p++)
    {
        character *player = get_player_at(p);

        rect_t attack_box;
        bool is_attacking = get_character_secondary_hitbox(player, &attack_box);

        int num_enemies = get_enemy_count();

        for (int e = 0; e < num_enemies; e++)
        {            
            character *enemy = get_enemy_at(e);

            float e_x1 = enemy->x;
            float e_y1 = enemy->y;
            float e_x2 = enemy->x + (float)enemy->meta.width;
            float e_y2 = enemy->y + (float)enemy->meta.height;

            // Player Melee Attack vs Enemy
            if (is_attacking)
            {
                if (attack_box.x1 < e_x2 && attack_box.x2 > e_x1 &&
                    attack_box.y1 < e_y2 && attack_box.y2 > e_y1)
                {

                    if (enemy->meta.last_hit_by_attack_id != player->meta.current_attack_id)
                    {
                        enemy->meta.last_hit_by_attack_id = player->meta.current_attack_id;

                        enemy->meta.health -= player->meta.damage;
                        if (enemy->meta.health <= 0)
                        {
                            destroy_enemy(e);
                            num_enemies--; // The total pool shrank by one
                            e--;       
                        }
                        else
                        {
                            float p_center_x = player->x + ((float)player->meta.width / 2.0f);
                            float e_center_x = enemy->x + ((float)enemy->meta.width / 2.0f);
                            enemy->physics.vx = (e_center_x > p_center_x) ? 45.0f : -45.0f;
                            enemy->physics.vy = -20.0f;
                            enemy->physics.state &= ~GROUNDED;
                        }
                    }
                    continue;
                }
            }

            // Enemy Body Touch Damage vs Player
            if (player->meta.invincibility_frames == 0)
            {
                float p_x1 = player->x;
                float p_y1 = player->y;
                float p_x2 = player->x + (float)player->meta.width;
                float p_y2 = player->y + (float)player->meta.height;

                if (p_x1 < e_x2 && p_x2 > e_x1 && p_y1 < e_y2 && p_y2 > e_y1)
                {
                    player->meta.health-= enemy->meta.damage;
                    if (player->meta.health <= 0)
                    {
                        destroy_player(p);
                        player_count--; // The total pool shrank by one
                        p--;       
                        break;
                    }

                    float p_center_x = player->x + ((float)player->meta.width / 2.0f);
                    float e_center_x = enemy->x + ((float)enemy->meta.width / 2.0f);
                    player->physics.vx = (p_center_x < e_center_x) ? -120.0f : 120.0f;
                    player->physics.vy = -100.0f;
                    player->physics.state &= ~GROUNDED;
                    player->meta.invincibility_frames = 60;
                }
            }
        }
    }
}

void check_projectile_collisions(game_state_t *state)
{
    int proj_count = get_projectile_count();

    for (int i = proj_count - 1; i >= 0; i--)
    {
        projectile_t *proj = get_projectile_at(i);

        // Define projectile bounds
        float proj_x1 = proj->x;
        float proj_y1 = proj->y;
        float proj_x2 = proj->x + (float)proj->meta.width;
        float proj_y2 = proj->y + (float)proj->meta.height;

        bool hit_something = false;

        if (proj->meta.is_enemy)
        {
            int player_count = get_player_count();

            // ENEMY PROJECTILE VS PLAYERS
            for (int p = 0; p < player_count; p++)
            {
                character *player = get_player_at(p);
                if (player->meta.invincibility_frames > 0)
                    continue;

                float p_x1 = player->x;
                float p_y1 = player->y;
                float p_x2 = player->x + (float)player->meta.width;
                float p_y2 = player->y + (float)player->meta.height;

                // AABB Check
                if (proj_x1 < p_x2 && proj_x2 > p_x1 && proj_y1 < p_y2 && proj_y2 > p_y1)
                {
                    // Deal damage using projectile specific property
                    player->meta.health -= proj->meta.damage;
                    if (player->meta.health <= 0)
                    {
                        destroy_player(p);
                        player_count--; // The total pool shrank by one
                        p--;       
                    }
                    else
                    {
                        // Player Knockback
                        player->physics.vx = (player->x + (player->meta.width / 2.0f) < proj->x) ? -120.0f : 120.0f;
                        player->physics.vy = -100.0f;
                        player->physics.state &= ~GROUNDED;
                        player->meta.invincibility_frames = 60;
                    }
                    hit_something = true;
                    break; // Stop scanning players for this projectile
                }
            }
        }
        else
        {
            int num_enemies = get_enemy_count();

            // PLAYER PROJECTILE VS ENEMIES
            for (int e = 0; e < num_enemies; e++)
            {                
                character *enemy = get_enemy_at(e);

                float e_x1 = enemy->x;
                float e_y1 = enemy->y;
                float e_x2 = enemy->x + (float)enemy->meta.width;
                float e_y2 = enemy->y + (float)enemy->meta.height;

                // AABB Check
                if (proj_x1 < e_x2 && proj_x2 > e_x1 && proj_y1 < e_y2 && proj_y2 > e_y1)
                {                    
                    int player_id = proj->meta.owner->meta.id;
                    camera_t *camera = get_camera_at(player_id);

                    // 1. Get the player's world-space camera bounds
                    float cam_left   = (float)camera->x;
                    float cam_right  = (float)(camera->x + camera->width);
                    float cam_top    = (float)camera->y;
                    float cam_bottom = (float)(camera->y + camera->height);

                    // 2. Calculate the enemy's center in world-space
                    float enemy_center_x = enemy->x + ((float)enemy->meta.width / 2.0f);
                    float enemy_center_y = enemy->y + ((float)enemy->meta.height / 2.0f);

                    // 3. REFINED VIEWPORT BOUNDS CHECK (Pure World Space)
                    bool is_in_viewport = (enemy_center_x >= cam_left  && 
                                           enemy_center_x <= cam_right &&
                                           enemy_center_y >= cam_top   && 
                                           enemy_center_y <= cam_bottom);

                    if (is_in_viewport)
                    {
                        // Valid visible hit! Apply damage safely
                        enemy->meta.last_hit_by_attack_id = proj->meta.damage_source_id;
                        enemy->meta.health -= proj->meta.damage;
                                
                        if (enemy->meta.health <= 0)
                        {
                            destroy_enemy(e);
                            num_enemies--; // The total pool shrank by one
                            e--;       
                        }
                        else
                        {
                            // Enemy Knockback
                            float proj_center_x = proj->x + ((float)proj->meta.width / 2.0f);
                            enemy->physics.vx = (enemy_center_x > proj_center_x) ? 45.0f : -45.0f;
                            enemy->physics.vy = -20.0f;
                            enemy->physics.state &= ~GROUNDED;
                        }

                        hit_something = true;
                    }
                }
            }
        }

        if (!hit_something)
        {
            // Convert pixel bounds to tile index coordinates
            int tile_x1 = (int)proj_x1 / TILE_SIZE;
            int tile_y1 = (int)proj_y1 / TILE_SIZE;
            int tile_x2 = (int)proj_x2 / TILE_SIZE;
            int tile_y2 = (int)proj_y2 / TILE_SIZE;

            // Sample the map array data at the 4 corners of the bounding box.
            // Adjust 'state->map_data' if your map buffer name is different.
            uint8_t tl = get_tile_at(state->level.map_data, tile_x1, tile_y1); // Top-Left
            uint8_t tr = get_tile_at(state->level.map_data, tile_x2, tile_y1); // Top-Right
            uint8_t bl = get_tile_at(state->level.map_data, tile_x1, tile_y2); // Bottom-Left
            uint8_t br = get_tile_at(state->level.map_data, tile_x2, tile_y2); // Bottom-Right

            // Assuming tile index '0' is empty space/air, and values >= 1 are solid structures
            if (tl == 2 || tr == 2 || bl == 2 || br == 2)
            {
                hit_something = true;
            }
        }

        // If it hit a target, delete the projectile (assuming non-piercing)
        if (hit_something)
        {
            destroy_projectile(i);
            proj_count--; // The total pool shrank by one
            i--;       
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

    // 1. Trigger INITIAL attack from a non-attacking state (Idle, Walk, etc.)
    if (!(self->meta.state & ATTACKING) && (self->meta.state & WANTS_TO_ATTACK)) {
        self->meta.current_anim = ANIM_ATTACK;
        self->meta.current_anim_frame_index = 0;
        self->meta.state |= ATTACKING;
        self->meta.state &= ~WANTS_TO_ATTACK; // Consume the input immediately
        self->meta.current_attack_id++;       // Start a fresh hit register tracking loop
    }

    // 2. Existing Attack override code (Handles ticking frames and combo chaining)
    if (self->meta.current_anim == ANIM_ATTACK) {
        const anim_config_t *atk_cfg = &character_anims[self->meta.type][ANIM_ATTACK];
        
        // Allow a new attack to interrupt if we are in the last 3 frames
        int early_cancel_frame = atk_cfg->frame_count - 3; 
        
        if ((self->meta.state & WANTS_TO_ATTACK) && self->meta.current_anim_frame_index >= early_cancel_frame) {
            // Restart a fresh combo swing!
            self->meta.current_anim = ANIM_ATTACK;
            self->meta.current_anim_frame_index = 0;
            self->meta.state |= ATTACKING;
            self->meta.state &= ~WANTS_TO_ATTACK; // Consume input
            self->meta.current_attack_id++;       // New attack ID so it hits enemies again
            process_time_based_ticker = true;
        }
        else if (self->meta.current_anim_frame_index < atk_cfg->frame_count - 1) {
            process_time_based_ticker = true; 
        } 
        else {
            // Attack loop finished entirely with no buffered follow-up
            self->meta.current_anim = is_on_solid_surface ? ANIM_IDLE : ANIM_JUMP;
            self->meta.state &= ~ATTACKING; 
            self->meta.state &= ~WANTS_TO_ATTACK; // Wipe any old inputs safely
        }
    }

    // 3. Air states run ONLY if we are floating completely in mid-air
    if (self->meta.current_anim != ANIM_ATTACK && !is_on_solid_surface) {
        self->meta.current_anim = ANIM_JUMP;
        float vy = self->physics.vy;

        if (vy < prof->jump_fast_up_threshold) {
            self->meta.current_anim_frame_index = 0;
        } 
        else if (vy < prof->jump_slow_up_threshold) {
            self->meta.current_anim_frame_index = 1;
        } 
        else if (vy >= -prof->apex_threshold && vy <= prof->apex_threshold) {
            self->meta.current_anim_frame_index = 2;
        } 
        else if (vy <= prof->fall_slow_down_threshold) {
            self->meta.current_anim_frame_index = 3;
        } 
        else {
            self->meta.current_anim_frame_index = 4;
        }
        
        self->meta.anim_timer = 0;
        process_time_based_ticker = false;
    }
    
    // 4. Ground states (Idle or Walk) run if resting safely on world tiles OR on a teammate
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
        self->meta.current_anim_frame_index = 0;
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
                self->meta.current_anim_frame_index = (self->meta.current_anim_frame_index + 1) % cfg->frame_count;

                if (self->meta.current_anim == ANIM_ATTACK && 
                    cfg->hitbox.style == HITBOX_STYLE_PROJECTILE) {
                        if (self->meta.current_anim_frame_index == cfg->hitbox_start_frame) {
                            spawn_projectile_from_character(self);
                        }
                    }
            }
        }
    }

    self->meta.current_frame++;
}

bool get_character_secondary_hitbox(const character *chr, rect_t *out_rect)
{
    const anim_config_t *cfg = &character_anims[chr->meta.type][chr->meta.current_anim];
    if (!cfg || cfg->hitbox_start_frame <= 0 || cfg->hitbox.style != HITBOX_STYLE_MELEE_SWEEP) {
        return false;
    }

    // Check if the current frame is inside the active hitbox window
    if (chr->meta.current_anim_frame_index >= cfg->hitbox_start_frame &&
        chr->meta.current_anim_frame_index <= cfg->hitbox_end_frame)
    {
        float actual_offset_x = cfg->hitbox.offset_x;
        
        if (chr->physics.facing_direction == FACING_LEFT) {
            float character_width = chr->meta.width; 
            actual_offset_x = character_width - cfg->hitbox.offset_x - cfg->hitbox.width;
        }

        // Calculate absolute WORLD space coordinates
        out_rect->x1 = chr->x + actual_offset_x;
        out_rect->y1 = chr->y + cfg->hitbox.offset_y;
        out_rect->x2 = out_rect->x1 + cfg->hitbox.width;
        out_rect->y2 = out_rect->y1 + cfg->hitbox.height;
        
        return true;
    }

    return false;
}

void load_stage_by_index(game_state_t *state, int index) {
    // Cache the previous match state before we overwrite anything
    match_state_t previous_state = state->match_state;

    if (index < 0 || index >= MAX_LEVELS) {
        index = 0; // Safe fallback boundary clamp
    }
    
    if (previous_state == STATE_GAME_OVER ||
        previous_state == STATE_WAITING_TO_START){
        cleanup_enemy_registry();
        cleanup_camera_registry();
        init_enemy_registry(16);
        init_camera_registry(MAX_VIEWPORTS);
    }

    state->level_index = index;
    
    const char *target_file = level_playlist[index].filename;
    float target_time       = level_playlist[index].time_limit;
    load_level_binary(target_file, state);
    state->level_timer = target_time; 

    int player_count = get_player_count();

    if (previous_state == STATE_LEVEL_CLEARED) 
    { 
        for (int i = 0; i < player_count; i++) 
        {
            character *player = get_player_at(i);

            // If we just cleared a level and this specific player survived (is active), 
            // DO NOT kill them. Keep their active state and health intact!
            // Stop horizontal speeds so they don't slide into the new stage uncontrollably
            character_type type = player->meta.type;
            int player_id = player->meta.id;
            int health = player->meta.health;
            character_init(player, type, false, player_id);
            player->meta.health = health;
            determine_new_player_coordinates(player, &state->level);
        }

        state->joined_players = player_count;
    } 
    else if (previous_state == STATE_GAME_OVER) 
    {
            cleanup_player_registry();
            init_player_registry(MAX_PLAYERS);
            state->joined_players = 0;
            player_count = 0;
    } 

    if (player_count == 0) 
    {
        state->match_state = STATE_WAITING_TO_START;
    } 
    else 
    {
        state->match_state = STATE_STAGE_INTRO;
    }
}

character* find_furthest_active_player(character *self) {
    character *furthest_active_player = NULL;
    
    int player_count = get_player_count();

    for(int i = 0; i < player_count; i++) {
        character *player = get_player_at(i);
        if (self != NULL && player == self) continue;
        
        if (!furthest_active_player || furthest_active_player->x < player->x) {
            furthest_active_player = player;
        }
    }

    return furthest_active_player;
}