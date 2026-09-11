#include "character.h"
#include "level.h"

float physics_constants[NUMBER_OF_CHARACTER_TYPES][9] = {
    // KNIGHT: Solid jump height boost to clear enemies easily
    {3.0f, 150.0f, 1250.0f, 1150.0f, 600.0f, 180.0f, 52.0f, 0.24f, 400.0f },
    
    // ELF: Agility class. Highest jump height (56px) for effortless stomping
    {3.5f, 160.0f, 1300.0f, 1100.0f, 650.0f, 170.0f, 56.0f, 0.22f, 400.0f},
    
    // WIZARD: Floatier style jump. Increased height with slightly longer peak time
    {2.5f, 140.0f, 1200.0f, 1200.0f, 550.0f, 190.0f, 48.0f, 0.28f, 400.0f},
    
    // DWARF: Heavy class. Respectable jump height increase while keeping a dense feel
    {2.8f, 145.0f, 1225.0f, 1175.0f, 575.0f, 185.0f, 46.0f, 0.25f, 400.0f},
    
    // --- ENEMY PHYSICS (Kept slow and distinct) ---
    // GOOMBA
    {1.0f, 40.0f,  400.0f,  600.0f,  100.0f, 100.0f, 32.0f, 0.24f, 400.0f},
    // SKELETON
    {2.0f, 75.0f,  500.0f,  600.0f,  300.0f, 150.0f, 28.0f, 0.24f, 400.0f}
};

void character_init(character *character, character_type type) {
    if(!character) return;

    character->active = false;
    character->supported_by_player = false;
    character->type = type;
    character->x = 20.0f;
    character->y = 150.0f;
    character->coyote_frames = 0;
    character->jump_buffer_frames = 0;
    character->invincibility_frames = 0;

    if (type >= 4) { 
        character->is_enemy = true;
        character->health = 1;
    } else {
        character->health = 3;
        character->is_enemy = false;
    }

    // initialize physics struct
    character->physics.vx = 0.0f;
    character->physics.vy = 0.0f;
    character->physics.ax = 0.0f;
    character->physics.ay = 0.0f;
    character->physics.turn_multiplier = physics_constants[type][0];
    character->physics.max_speed = physics_constants[type][1];
    character->physics.ground_acceleration = physics_constants[type][2];
    character->physics.ground_friction = physics_constants[type][3];
    character->physics.air_acceleration = physics_constants[type][4];
    character->physics.air_friction = physics_constants[type][5];
    character->physics.jump_height = physics_constants[type][6];
    character->physics.jump_time_to_peak = physics_constants[type][7];
    character->physics.terminal_velocity = physics_constants[type][8];
    character->physics.gravity_scale = (2.0f * physics_constants[type][6]) / (physics_constants[type][7] * physics_constants[type][7]);
    character->physics.jump_force = -(2.0f * physics_constants[type][6]) / physics_constants[type][7];
    character->physics.state = NONE;
}

void character_update(character *self, character *players, input_state *input, uint8_t *map_data, float dt) {
    if (!self || !input || !self->active) return;

    // Reset player-grounding flag before checking collisions this frame
    bool was_supported_by_player = self->supported_by_player;
    self->supported_by_player = false;

    apply_friction(self, input, dt);
    handle_move_left(self, input, dt);
    handle_move_right(self, input, dt);
    handle_jump(self, players, input); 

    apply_gravity(self, dt);
    
    // --- X Axis ---
    self->x += self->physics.vx * dt;
    check_wall_collision(self, map_data);

    // --- Y Axis ---
    self->y += self->physics.vy * dt;
    check_ceiling_collision(self, map_data);
    check_grounded(self, map_data); // Sets GROUNDED if touching solid world map tiles

    // --- Dynamic Inter-character Collisions ---
    // If we aren't touching world tiles, this might re-apply GROUNDED if we land on a player
    check_character_collisions(self, players, dt); 

    // --- CRITICAL COYOTE FIX ---
    // If we were standing on a player last frame, but we aren't standing on a player 
    // OR a world tile this frame, strip the GROUNDED flag so we fall instantly.
    if (!(self->physics.state & GROUNDED) && !self->supported_by_player && was_supported_by_player) {
         self->physics.state &= ~GROUNDED;
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
    if (!(character->physics.state & GROUNDED) && !character->supported_by_player)
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

        // Choose acceleration based on ground vs air status
        float current_accel = (character->physics.state & GROUNDED || character->supported_by_player) 
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

        // Choose acceleration based on ground vs air status
        float current_accel = (character->physics.state & GROUNDED || character->supported_by_player) 
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
    // Check if jump button is pressed
    if (input->active_actions & ACTION_JUMP)
    {
        // Allowed to jump if grounded on a tile OR supported by a player
        if ((self->physics.state & GROUNDED) || self->supported_by_player || self->coyote_frames > 0)
        {
            // Apply the jump velocity macro calculation
            self->physics.vy = self->physics.jump_force;
            
            // Clear ground states
            self->physics.state &= ~GROUNDED;
            self->physics.state |= JUMPING;
            self->coyote_frames = 0;

            // --- MOMENTUM TRANSFER ---
            // If we are jumping off a teammate, inherit their X speed so we don't drop straight down
            if (self->supported_by_player && players) {
                for (int i = 0; i < MAX_PLAYERS; i++) {
                    character *other = &players[i];
                    if (other == self || !other->active) continue;

                    // Verify if this is the player directly beneath our feet
                    if (self->x < other->x + PLAYER_WIDTH &&
                        self->x + PLAYER_WIDTH > other->x &&
                        fabsf((self->y + PLAYER_HEIGHT) - other->y) < 2.0f) {
                        
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
    // 1. Calculate X tile coordinates for left and right edges
    int tile_left_x  = (int)(character->x) / TILE_SIZE;
    int tile_right_x = (int)(character->x + PLAYER_WIDTH) / TILE_SIZE;

    // 2. Calculate Y tile coordinates for 3 vertical check points (Head, Torso, Feet)
    // Tucked in closer by 0.1f so the feet checks don't clip the floor tile beneath you
    int tile_head_y  = (int)(character->y + 0.1f) / TILE_SIZE;
    int tile_torso_y = (int)(character->y + (PLAYER_HEIGHT / 2.0f)) / TILE_SIZE;
    int tile_feet_y  = (int)(character->y + PLAYER_HEIGHT - 0.1f) / TILE_SIZE;

    // 3. Look up all tile values from the binary matrix array
    uint8_t left_head  = get_tile_at(map_data, tile_left_x, tile_head_y);
    uint8_t left_torso = get_tile_at(map_data, tile_left_x, tile_torso_y);
    uint8_t left_feet  = get_tile_at(map_data, tile_left_x, tile_feet_y);

    uint8_t right_head  = get_tile_at(map_data, tile_right_x, tile_head_y);
    uint8_t right_torso = get_tile_at(map_data, tile_right_x, tile_torso_y);
    uint8_t right_feet  = get_tile_at(map_data, tile_right_x, tile_feet_y);

    // 4. Resolve left wall collisions
    if (left_head == 2 || left_torso == 2 || left_feet == 2) {
        character->x = (float)((tile_left_x + 1) * TILE_SIZE);
        character->physics.vx = 0.0f;
    }
    
    // 5. Resolve right wall collisions
    if (right_head == 2 || right_torso == 2 || right_feet == 2) {
        character->x = (float)(tile_right_x * TILE_SIZE - PLAYER_WIDTH);
        character->physics.vx = 0.0f;
    }
}

void check_ceiling_collision(character *character, uint8_t *map_data)
{
    // Double point check for the ceiling using a tiny sub-pixel look-ahead
    int tile_left_x  = (int)(character->x + 1.0f) / TILE_SIZE;
    int tile_right_x = (int)(character->x + PLAYER_WIDTH - 1.0f) / TILE_SIZE;
    
    // Changed from -1.0f to -0.01f so standing under a low ceiling doesn't register a collision
    int tile_top_y   = (int)(character->y - 0.01f) / TILE_SIZE; 

    uint8_t tile_above_left  = get_tile_at(map_data, tile_left_x, tile_top_y);
    uint8_t tile_above_right = get_tile_at(map_data, tile_right_x, tile_top_y);

    if (tile_above_left == 2 || tile_above_right == 2) {
        // Push down and apply a tiny sub-pixel cushion down so you don't instantly clip the roof
        character->y = (float)((tile_top_y + 1) * TILE_SIZE) + 0.01f;
        if (character->physics.vy < 0.0f) {
            character->physics.vy = 0.0f;
        }
    }
}

void check_grounded(character *character, uint8_t *map_data)
{
    int tile_left_x  = (int)(character->x + 1.0f) / TILE_SIZE;
    int tile_right_x = (int)(character->x + PLAYER_WIDTH - 1.0f) / TILE_SIZE;

    // Scan vertical range down
    int start_tile_y = (int)(character->y + PLAYER_HEIGHT - 4.0f) / TILE_SIZE; 
    
    // Changed from +1.0f to +0.01f to match our precise sub-pixel cushion lift
    int end_tile_y   = (int)(character->y + PLAYER_HEIGHT + 0.01f) / TILE_SIZE;

    if (start_tile_y > end_tile_y) start_tile_y = end_tile_y;

    for (int tile_y = start_tile_y; tile_y <= end_tile_y; tile_y++)
    {
        uint8_t tile_below_left  = get_tile_at(map_data, tile_left_x, tile_y);
        uint8_t tile_below_right = get_tile_at(map_data, tile_right_x, tile_y);

        if (tile_below_left == 2 || tile_below_right == 2) 
        {
            // Lift by 0.01f off the grid floor row so you fit perfectly in 2-tile high gaps
            character->y = (float)(tile_y * TILE_SIZE) - PLAYER_HEIGHT - 0.01f;
            character->physics.vy = 0.0f;
            character->physics.state |= GROUNDED;
            character->physics.state &= ~JUMPING;
            character->coyote_frames = COYOTE_MAX;
            return; 
        }
    }

    character->physics.state &= ~GROUNDED;
    if (character->coyote_frames > 0) {
        character->coyote_frames--;
    }
}

void check_character_collisions(character *self, character *players, float dt) {
    if (!self || !players) return;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        character *other = &players[i];
        if (other == self || !other->active) continue;

        if (self->x < other->x + PLAYER_WIDTH &&
            self->x + PLAYER_WIDTH > other->x &&
            self->y < other->y + PLAYER_HEIGHT &&
            self->y + PLAYER_HEIGHT > other->y) {

            float overlap_x = fminf(self->x + PLAYER_WIDTH - other->x, other->x + PLAYER_WIDTH - self->x);
            float overlap_y = fminf(self->y + PLAYER_HEIGHT - other->y, other->y + PLAYER_HEIGHT - self->y);

            if (overlap_x < overlap_y) {
                if (self->x < other->x) {
                    self->x -= overlap_x;
                } else {
                    self->x += overlap_x;
                }
                self->physics.vx = 0.0f;
            } else {
                if (self->y < other->y && self->physics.vy >= 0.0f) {
                    self->y = other->y - PLAYER_HEIGHT;
                    
                    // --- THE FIX ---
                    // Match the velocity of the player underneath so you move with them smoothly
                    self->x += other->physics.vx * dt; // Ensure dt is passed into this function or handled
                    
                    self->physics.vy = 0.0f;
                    self->physics.state |= GROUNDED;
                    self->supported_by_player = true; // Mark that a player is holding us up
                    self->coyote_frames = COYOTE_MAX;
                } else if (self->y > other->y && self->physics.vy < 0.0f) {
                    self->y = other->y + PLAYER_HEIGHT;
                    self->physics.vy = 0.0f;
                }
            }
        }
    }
}