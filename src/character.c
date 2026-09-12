#include "character.h"
#include "level.h"

extern const animation_profile_t CHARACTER_ANIMATION_PROFILES[CHARACTER_TYPE_MAX];

float physics_constants[NUMBER_OF_CHARACTER_TYPES][9]= {
    // Column Guide:
    // 0: Turn Multiplier (Responsiveness when snapping opposite direction)
    // 1: Max Speed (Cap on horizontal physics velocity)
    // 2: Ground Acceleration (Rate of horizontal speed buildup on floor)
    // 3: Ground Friction (Deceleration stopping rate when idling)
    // 4: Air Acceleration (Horizontal steering push while airborne)
    // 5: Air Friction (Wind resistance slowing forward momentum in air)
    // 6: Jump Height (Target peak distance in absolute pixels)
    // 7: Jump Time To Peak (Duration in seconds to reach the peak height)
    // 8: Terminal Velocity (Maximum allowed falling velocity)

    // =========================================================================
    // --- HERO CLASSES (+10-15% Velocity Boost, High Friction Braking) ---
    // =========================================================================
    
    // KNIGHT: The baseline standard. Paced perfectly at ~1.4 pixels per frame.
    { 3.0f, 85.0f,  285.0f, 1250.0f, 175.0f, 200.0f, 46.0f, 0.25f, 320.0f },   
    
    // ELF: Super snappy, responsive, and light on its feet. Highest mobility class.
    { 3.5f, 96.0f,  400.0f, 1450.0f, 230.0f, 150.0f, 52.0f, 0.22f, 320.0f },   
    
    // WIZARD: Floatier style. Slow, deliberate build-up with a long, sweeping jump arc.
    { 2.5f, 74.0f,  230.0f, 1050.0f, 140.0f, 100.0f, 44.0f, 0.29f, 320.0f },    
    
    // DWARF: Heavy tank class. Slower to get going, but halts instantly.
    { 2.8f, 79.0f,  250.0f, 1650.0f, 130.0f, 250.0f, 40.0f, 0.27f, 320.0f },    

    // =========================================================================
    // --- ENEMY CLASSES (Slightly boosted to keep up with players) ---
    // =========================================================================
    
    // GOOMBA: Methodical walk. Just fast enough that you can't completely ignore it.
    { 1.0f, 28.0f,  140.0f, 800.0f,  90.0f,  100.0f, 24.0f, 0.24f, 320.0f },
    
    // SKELETON: Fast, aggressive patrolling pacing to catch careless players.
    { 2.0f, 42.0f,  210.0f, 800.0f,  175.0f, 150.0f, 18.0f, 0.24f, 320.0f }
};

void character_init(character *character, character_type type, bool is_enemy) {
    if(!character) return;

    // init meta
    character->meta.anim_profile = &CHARACTER_ANIMATION_PROFILES[type];
    character->meta.state = CHARACTER_NONE;
    character->meta.type = type;
    character->meta.coyote_frames = 0;
    character->meta.jump_buffer_frames = 0;
    character->meta.invincibility_frames = 0;
    character->meta.ai_home_row = 0;
    character->meta.ai_jump_cooldown = 0;
    character->meta.current_frame = 0;
    character->meta.anim_timer = 0;
    character->meta.current_anim = 0;
    character->meta.is_enemy = is_enemy;
    character->meta.current_anim = ANIM_IDLE;
    character->physics.facing_direction = FACING_LEFT;

    if (!is_enemy) {
        character->meta.width = PLAYER_WIDTH;
        character->meta.height = PLAYER_HEIGHT;
        character->physics.facing_direction = FACING_RIGHT;
    }

    switch(type) {
        case KNIGHT:
        character->meta.width = 18.0f;
        character->meta.height = 32.0f;
            character->meta.health = 4;
        break;
        case ELF:
            character->meta.health = 3;
        break;
        case WIZARD:
            character->meta.health = 3;
        break;
        case DWARF:
            character->meta.health = 5;
        break;
        case GOOMBA:
            character->meta.health = 2;
            character->meta.width = 20;
            character->meta.height = 16;
        break;
        case SKELETON:
            character->meta.health = 1;
            character->meta.width = 14;
            character->meta.height = 28;
        break;
        default:
            character->meta.health = 0;
            character->meta.width = 0;
            character->meta.height = 0;
            break;
    }

    // init position
    character->x = 0.0f;
    character->y = 0.0f;

    int physics_index = type;

    // initialize physics struct
    character->physics.vx = 0.0f;
    character->physics.vy = 0.0f;
    character->physics.ax = 0.0f;
    character->physics.ay = 0.0f;
    character->physics.turn_multiplier = physics_constants[physics_index][0];
    character->physics.max_speed = physics_constants[physics_index][1];
    character->physics.ground_acceleration = physics_constants[physics_index][2];
    character->physics.ground_friction = physics_constants[physics_index][3];
    character->physics.air_acceleration = physics_constants[physics_index][4];
    character->physics.air_friction = physics_constants[physics_index][5];
    character->physics.jump_height = physics_constants[physics_index][6];
    character->physics.jump_time_to_peak = physics_constants[physics_index][7];
    character->physics.terminal_velocity = physics_constants[physics_index][8];
    character->physics.gravity_scale = (2.0f * physics_constants[physics_index][6]) / (physics_constants[physics_index][7] * physics_constants[physics_index][7]);
    character->physics.jump_force = -(2.0f * physics_constants[physics_index][6]) / physics_constants[physics_index][7];
    character->physics.state = PHYSICS_NONE;
}

void character_update(character *self, character *players, input_state *input, uint8_t *map_data, float dt) {
    if (!self || !input || !(self->meta.state & ACTIVE)) return;

    bool was_supported_by_player = self->meta.state & SUPPORTED_BY_PLAYER;
    self->meta.state &= ~SUPPORTED_BY_PLAYER;

    apply_friction(self, input, dt);
    handle_move_left(self, input, dt);
    handle_move_right(self, input, dt);
    handle_jump(self, players, input); 

    apply_gravity(self, dt);
    
    // =========================================================================
    // STEP 1: RESOLVE ALL X-AXIS MOVEMENT FIRST
    // =========================================================================
    // Move by your own input velocity
    self->x += self->physics.vx * dt;

    // Carry teammate momentum across the X axis BEFORE testing wall limits
    if (was_supported_by_player) {
        // Find the player we were standing on last frame to inherit velocity safely
        for (int i = 0; i < MAX_PLAYERS; i++) {
            character *other = &players[i];
            if (other != self && (other->meta.state & ACTIVE)) {
                // Check if we are still safely aligned vertically above them
                if (self->x < other->x + (float)other->meta.width &&
                    self->x + (float)self->meta.width > other->x) {
                    self->x += other->physics.vx * dt;
                    break;
                }
            }
        }
    }
    
    // Clamp to walls after all physical X translations are fully completed
    check_wall_collision(self, map_data);

    // =========================================================================
    // STEP 2: RESOLVE ALL Y-AXIS MOVEMENT
    // =========================================================================
    self->y += self->physics.vy * dt;
    check_ceiling_collision(self, map_data);
    
    // Clear grounding state to build it fresh from geometry checks
    self->physics.state &= ~GROUNDED;
    check_grounded(self, map_data); 

    // =========================================================================
    // STEP 3: RESOLVE INTER-CHARACTER RESOLUTION
    // =========================================================================
    // Modified check_character_collisions must NO LONGER inject "self->x += other->physics.vx * dt;"
    check_character_collisions(self, players, dt); 

    // =========================================================================
    // STEP 4: CLEAN UP COYOTE STATE
    // =========================================================================
    if (!(self->physics.state & GROUNDED) && !(self->meta.state & SUPPORTED_BY_PLAYER) && was_supported_by_player) {
        self->physics.state &= ~GROUNDED;
        // If they just slipped off a player, make sure they start falling naturally
        if (self->physics.vy < 0.0f) self->physics.vy = 0.0f; 
    }

    if (self->meta.invincibility_frames > 0) {
        self->meta.invincibility_frames--;
    }
}


float approach(float current, float target, float step) {
    if (fabsf(target - current) <= step) {
        return target;
    }
    return current + (target > current ? step : -step);
}

void apply_friction(character *character, input_state *input, float dt)
{
    if (input->active_actions & ACTION_MOVE_LEFT || input->active_actions & ACTION_MOVE_RIGHT) 
        return;

    if (character->physics.state & GROUNDED) {
        // Stops the player quickly on the ground
        character->physics.vx = approach(character->physics.vx, 0, character->physics.ground_friction * dt);
    } else {
        // Gently shaves off a bit of forward speed in mid-air
        character->physics.vx = approach(character->physics.vx, 0, character->physics.air_friction * dt);
    }
}

void apply_gravity(character *character, float dt)
{
    // Apply gravity if we aren't resting on a tile AND aren't resting on a teammate
    if (!(character->physics.state & GROUNDED) && !(character->meta.state & SUPPORTED_BY_PLAYER))
    {
        character->physics.vy += character->physics.gravity_scale * dt;
        if (character->physics.vy > character->physics.terminal_velocity)
        {
            character->physics.vy = character->physics.terminal_velocity;
        }
    }
}

void handle_move_left(character *character, input_state *input, float dt)
{
    if (input->active_actions & ACTION_MOVE_LEFT)
    {
        character->physics.state |= MOVING_LEFT;
        character->physics.state &= ~MOVING_RGHT;
        character->physics.facing_direction = FACING_LEFT; 

        // Choose acceleration based on ground vs air status
        float current_accel = (character->physics.state & GROUNDED || character->meta.state & SUPPORTED_BY_PLAYER) 
            ? character->physics.ground_acceleration 
            : character->physics.air_acceleration;

        // --- TURNAROUND CHECK ---
        // If player is moving RIGHT (> 0) but holding LEFT, apply turn_multiplier for instant responsiveness
        if (character->physics.vx > 0.0f) 
        {
            character->physics.vx -= current_accel * character->physics.turn_multiplier * dt;
        } 
        else 
        {
            character->physics.vx -= current_accel * dt;
        }

        // Clamp to maximum speed
        if (character->physics.vx < -character->physics.max_speed) 
        {
            character->physics.vx = -character->physics.max_speed;
        }
    }
}

void handle_move_right(character *character, input_state *input, float dt)
{
    if (input->active_actions & ACTION_MOVE_RIGHT)
    {
        character->physics.state |= MOVING_RGHT;
        character->physics.state &= ~MOVING_LEFT;
        character->physics.facing_direction = FACING_RIGHT; 

        // Choose acceleration based on ground vs air status
        float current_accel = (character->physics.state & GROUNDED || character->meta.state & SUPPORTED_BY_PLAYER) 
            ? character->physics.ground_acceleration 
            : character->physics.air_acceleration;

        // --- TURNAROUND CHECK ---
        // If player is moving LEFT (< 0) but holding RIGHT, apply turn_multiplier for instant responsiveness
        if (character->physics.vx < 0.0f) 
        {
            character->physics.vx += current_accel * character->physics.turn_multiplier * dt;
        } 
        else 
        {
            character->physics.vx += current_accel * dt;
        }

        // Clamp to maximum speed
        if (character->physics.vx > character->physics.max_speed) 
        {
            character->physics.vx = character->physics.max_speed;
        }
    }
}

void handle_jump(character *self, character *players, input_state *input)
{
    if (input->active_actions & ACTION_JUMP)
    {
        if ((self->physics.state & GROUNDED) || self->meta.state & SUPPORTED_BY_PLAYER || self->meta.coyote_frames > 0)
        {
            self->physics.vy = self->physics.jump_force;
            
            self->physics.state &= ~GROUNDED;
            self->physics.state |= JUMPING;
            self->meta.coyote_frames = 0;

            // --- MOMENTUM TRANSFER ---
            if (self->meta.state & SUPPORTED_BY_PLAYER && players) {
                for (int i = 0; i < MAX_PLAYERS; i++) {
                    character *other = &players[i];
                    if (other == self || !(other->meta.state & ACTIVE)) continue;

                    // FIXED: Dynamic size comparisons for stacked calculations
                    if (self->x < other->x + (float)other->meta.width &&
                        self->x + (float)self->meta.width > other->x &&
                        fabsf((self->y + (float)self->meta.height) - other->y) < 2.0f) {
                        
                        self->physics.vx += other->physics.vx; 
                        break;
                    }
                }
            }
        }
    }
}

void check_wall_collision(character *character, uint8_t *map_data)
{
    // FIXED: Uses local meta.width for edge calculation
    int tile_left_x  = (int)(character->x) / TILE_SIZE;
    int tile_right_x = (int)(character->x + (float)character->meta.width) / TILE_SIZE;

    // FIXED: Midpoint and feet heights pull from unique meta.height
    int tile_head_y  = (int)(character->y + 0.1f) / TILE_SIZE;
    int tile_torso_y = (int)(character->y + ((float)character->meta.height / 2.0f)) / TILE_SIZE;
    int tile_feet_y  = (int)(character->y + (float)character->meta.height - 0.1f) / TILE_SIZE;

    uint8_t left_head  = get_tile_at(map_data, tile_left_x, tile_head_y);
    uint8_t left_torso = get_tile_at(map_data, tile_left_x, tile_torso_y);
    uint8_t left_feet  = get_tile_at(map_data, tile_left_x, tile_feet_y);

    uint8_t right_head  = get_tile_at(map_data, tile_right_x, tile_head_y);
    uint8_t right_torso = get_tile_at(map_data, tile_right_x, tile_torso_y);
    uint8_t right_feet  = get_tile_at(map_data, tile_right_x, tile_feet_y);

    if (left_head == 2 || left_torso == 2 || left_feet == 2) {
        character->x = (float)((tile_left_x + 1) * TILE_SIZE);
        character->physics.vx = 0.0f;
    }
    
    if (right_head == 2 || right_torso == 2 || right_feet == 2) {
        // FIXED: Pushes left wall pushback bounds using local meta.width
        character->x = (float)(tile_right_x * TILE_SIZE - character->meta.width);
        character->physics.vx = 0.0f;
    }
}

void check_ceiling_collision(character *character, uint8_t *map_data)
{
    // FIXED: Right edge check bounds use local meta.width
    int tile_left_x  = (int)(character->x + 1.0f) / TILE_SIZE;
    int tile_right_x = (int)(character->x + (float)character->meta.width - 1.0f) / TILE_SIZE;
    
    int tile_top_y   = (int)(character->y - 0.01f) / TILE_SIZE; 

    uint8_t tile_above_left  = get_tile_at(map_data, tile_left_x, tile_top_y);
    uint8_t tile_above_right = get_tile_at(map_data, tile_right_x, tile_top_y);

    if (tile_above_left == 2 || tile_above_right == 2) {
        character->y = (float)((tile_top_y + 1) * TILE_SIZE) + 0.01f;
        if (character->physics.vy < 0.0f) {
            character->physics.vy = 0.0f;
        }
    }
}

void check_grounded(character *character, uint8_t *map_data)
{
    // FIXED: Uses unique meta.width for foot columns
    int tile_left_x  = (int)(character->x + 1.0f) / TILE_SIZE;
    int tile_right_x = (int)(character->x + (float)character->meta.width - 1.0f) / TILE_SIZE;

    // FIXED: Checks scan ranges lower using dynamic meta.height properties
    int start_tile_y = (int)(character->y + (float)character->meta.height - 4.0f) / TILE_SIZE; 
    int end_tile_y   = (int)(character->y + (float)character->meta.height + 0.01f) / TILE_SIZE;

    if (start_tile_y > end_tile_y) start_tile_y = end_tile_y;

    for (int tile_y = start_tile_y; tile_y <= end_tile_y; tile_y++)
    {
        uint8_t tile_below_left  = get_tile_at(map_data, tile_left_x, tile_y);
        uint8_t tile_below_right = get_tile_at(map_data, tile_right_x, tile_y);

        if (tile_below_left == 2 || tile_below_right == 2) 
        {
            // FIXED: Floor landing snap position lifts using dynamic character meta.height
            character->y = (float)(tile_y * TILE_SIZE) - (float)character->meta.height - 0.01f;
            character->physics.vy = 0.0f;
            character->physics.state |= GROUNDED;
            character->physics.state &= ~JUMPING;
            character->meta.coyote_frames = COYOTE_MAX;
            return; 
        }
    }

    character->physics.state &= ~GROUNDED;
    if (character->meta.coyote_frames > 0) {
        character->meta.coyote_frames--;
    }
}

void check_character_collisions(character *self, character *players, float dt) {
    if (!self || !players) return;

    // A tiny padding value to protect against floating point inaccuracy
    const float EPSILON = 0.05f;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        character *other = &players[i];
        if (other == self || !(other->meta.state & ACTIVE)) continue;

        // FIXED: Changed boundaries to inclusive (<= and >=) with a tiny EPSILON buffer 
        // to ensure touching/overlapping borders capture the collision state correctly
        if (self->x <= other->x + (float)other->meta.width + EPSILON &&
            self->x + (float)self->meta.width >= other->x - EPSILON &&
            self->y <= other->y + (float)other->meta.height + EPSILON &&
            self->y + (float)self->meta.height >= other->y - EPSILON) {

            float overlap_x = fminf(self->x + (float)self->meta.width - other->x, other->x + (float)other->meta.width - self->x);
            float overlap_y = fminf(self->y + (float)self->meta.height - other->y, other->y + (float)other->meta.height - self->y);

            // Prevent vertical jitter from accidentally choosing X axis correction
            if (overlap_x < (overlap_y - EPSILON)) {
                if (self->x < other->x) {
                    self->x -= overlap_x;
                } else {
                    self->x += overlap_x;
                }
                self->physics.vx = 0.0f;
            } else {
                // Standing on top of a player
                if (self->y <= other->y && self->physics.vy >= -EPSILON) {
                    
                    // Snap position perfectly
                    self->y = other->y - (float)self->meta.height;
                    
                    // Keep teammate momentum drift out of this box resolution stage
                    // self->x += other->physics.vx * dt; // Handled in update loop stage now
                    
                    self->physics.vy = 0.0f;
                    
                    // Explicitly strip JUMPING/FALLING statuses so state machine matches
                    self->physics.state &= ~JUMPING;
                    self->physics.state |= GROUNDED;                    
                    self->meta.state |= SUPPORTED_BY_PLAYER; 
                    self->meta.coyote_frames = COYOTE_MAX;
                } else if (self->y > other->y && self->physics.vy < 0.0f) {
                    // Head bump displacement
                    self->y = other->y + (float)other->meta.height;
                    self->physics.vy = 0.0f;
                }
            }
        }
    }
}
