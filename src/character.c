#include "character.h"
#include "level.h"

float physics_constants[NUMBER_OF_CHARACTER_TYPES][9] = {
    // KNIGHT
    {3.0f, 150.0f, 1250.0f, 1150.0f, 600.0f, 180.0f, 36.0f, 0.24f, 400.0f },
    // ELF
    {3.5f, 160.0f, 1300.0f, 1100.0f, 650.0f, 170.0f, 40.0f, 0.22f, 400.0f},
    // WIZARD
    {2.5f, 140.0f, 1200.0f, 1200.0f, 550.0f, 190.0f, 32.0f, 0.26f, 400.0f},
    // DWARF
    {2.8f, 145.0f, 1225.0f, 1175.0f, 575.0f, 185.0f, 34.0f, 0.25f, 400.0f}
};

void character_init(character *character, character_type type) {
    if(!character) return;

    character->active = false;
    character->type = type;
    character->x = 20.0f;
    character->y = 150.0f;
    character->coyote_frames = 0;
    character->jump_buffer_frames = 0;

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

void character_update(character *self, character *players, input_state *input, float dt) {
    if (!self || !input || !self->active) return;

    // 1. Process Input & Friction Modifications
    apply_friction(self, input, dt);
    handle_move_left(self, input, dt);
    handle_move_right(self, input, dt);
    handle_jump(self, input); // Triggers your JUMP_VELOCITY

    // 2. Apply Environmental Forces over Time (THE FIX)
    apply_gravity(self, dt);

    // 3. Move the character position based on final velocities
    move_character(self, dt);

    // 4. Resolve Collisions and Reset Ground Flags
    check_grounded(self);
    check_wall_collision(self);
    check_ceiling_collision(self);
    check_character_collisions(self, players);
}

void check_character_collisions(character *self, character *players) {
    if (!self || !players) return;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        character *other = &players[i];
        if (other == self || !other->active) continue;

        if (self->x < other->x + 16 &&
            self->x + 16 > other->x &&
            self->y < other->y + 16 &&
            self->y + 16 > other->y) {

            float overlap_x = fminf(self->x + 16.0f - other->x, other->x + 16.0f - self->x);
            float overlap_y = fminf(self->y + 16.0f - other->y, other->y + 16.0f - self->y);

            if (overlap_x < overlap_y) {
                // Side collision
                if (self->x < other->x) {
                    self->x -= overlap_x;
                } else {
                    self->x += overlap_x;
                }
                self->physics.vx = 0.0f;
            } else {
                // Vertical collision
                if (self->y < other->y && self->physics.vy >= 0.0f) {
                    self->y = other->y - 16.0f;
                    self->physics.vy = 0.0f;
                    self->physics.state |= GROUNDED;
                    self->coyote_frames = COYOTE_MAX;
                } else if (self->y > other->y && self->physics.vy < 0.0f) {
                    self->y = other->y + 16.0f;
                    self->physics.vy = 0.0f;
                }
            }
        }
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

void check_grounded(character *character)
{
    // 1. Calculate the tile position right beneath the character's feet
    // We check the center-bottom of the 16x16 bounding box
    int tile_x = (int)(character->x + 8.0f) / TILE_SIZE;
    int tile_y = (int)(character->y + 16.0f) / TILE_SIZE;

    // 2. Look up the tile value from our binary matrix array
    uint8_t tile_below = get_tile_at(tile_x, tile_y);

    if (tile_below == 2) // If it's a solid block layout point
    {
        // Snap the character perfectly on top of the tile boundary edge pixel
        character->y = (float)(tile_y * TILE_SIZE) - 16.0f;
        character->physics.vy = 0.0f;
        character->physics.state |= GROUNDED;
        character->physics.state &= ~ JUMPING;
        character->coyote_frames = COYOTE_MAX;
    } 
    else
    {
        character->physics.state &= ~GROUNDED;
        if (character->coyote_frames > 0) {
            character->coyote_frames--;
        }
    }
}

void check_wall_collision(character *character)
{
    // 1. Calculate the tile position at the character's left and right edges
    int tile_left_x = (int)(character->x) / TILE_SIZE;
    int tile_right_x = (int)(character->x + 16.0f) / TILE_SIZE;
    int tile_y = (int)(character->y + 8.0f) / TILE_SIZE; // Check mid-height for horizontal collisions

    // 2. Look up the tile values from our binary matrix array
    uint8_t tile_left = get_tile_at(tile_left_x, tile_y);
    uint8_t tile_right = get_tile_at(tile_right_x, tile_y);

    // 3. Resolve collisions with solid blocks
    if (tile_left == 2) {
        character->x = (float)((tile_left_x + 1) * TILE_SIZE);
        character->physics.vx = 0.0f;
    }
    if (tile_right == 2) {
        character->x = (float)(tile_right_x * TILE_SIZE - 16.0f);
        character->physics.vx = 0.0f;
    }
}

void check_ceiling_collision(character *character)
{
    // 1. Calculate the tile position at the character's top edge
    int tile_x = (int)(character->x + 8.0f) / TILE_SIZE; // Check mid-width for vertical collisions
    int tile_top_y = (int)(character->y) / TILE_SIZE;

    // 2. Look up the tile value from our binary matrix array
    uint8_t tile_above = get_tile_at(tile_x, tile_top_y);

    // 3. Resolve collision with solid blocks
    if (tile_above == 2) {
        character->y = (float)((tile_top_y + 1) * TILE_SIZE);
        if (character->physics.vy < 0.0f) {
            character->physics.vy = 0.0f;
        }
    }
}

void apply_gravity(character *character, float dt)
{
    // 1. Apply Gravity if in the air
    if (!(character->physics.state & GROUNDED))
    {
        character->physics.vy += character->physics.gravity_scale * dt;
        if (character->physics.vy > character->physics.terminal_velocity)
        {
            character->physics.vy = character->physics.terminal_velocity;
        }
    }
}

void handle_move_right(character *character, input_state *input, float dt)
{
    if (input->active_actions & ACTION_MOVE_RIGHT)
    {
        character->physics.state |= MOVING_RGHT;

        if (character->physics.state & GROUNDED) {
            // CHECK FOR TURNAROUND: Player is moving LEFT (< 0) but holding RIGHT
            if (character->physics.vx < 0.0f) {
                character->physics.vx = approach(character->physics.vx, character->physics.max_speed, character->physics.ground_acceleration * character->physics.turn_multiplier * dt);
            } else {
                character->physics.vx = approach(character->physics.vx, character->physics.max_speed, character->physics.ground_acceleration * dt);
            }
        }
        else {
            // Air turnaround (optional: can keep it lower than ground for loose air control)
            if (character->physics.vx < 0.0f) {
                character->physics.vx = approach(character->physics.vx, character->physics.max_speed, character->physics.air_acceleration * 1.5f * dt);
            } else {
                character->physics.vx = approach(character->physics.vx, character->physics.max_speed, character->physics.air_acceleration * dt);
            }
        }
    }
    else {
        character->physics.state &= ~ MOVING_RGHT;
    }
}

void handle_move_left(character *character, input_state *input, float dt)
{
    if (input->active_actions & ACTION_MOVE_LEFT)
    {
        character->physics.state |= MOVING_LEFT;

        if (character->physics.state & GROUNDED) {
            // CHECK FOR TURNAROUND: Player is moving RIGHT (> 0) but holding LEFT
            if (character->physics.vx > 0.0f) {
                character->physics.vx = approach(character->physics.vx, -character->physics.max_speed, character->physics.ground_acceleration * character->physics.turn_multiplier * dt);
            } else {
                character->physics.vx = approach(character->physics.vx, -character->physics.max_speed, character->physics.ground_acceleration * dt);
            }
        }
        else {
            // Air turnaround
            if (character->physics.vx > 0.0f) {
                character->physics.vx = approach(character->physics.vx, -character->physics.max_speed, character->physics.air_acceleration * 1.5f * dt);
            } else {
                character->physics.vx = approach(character->physics.vx, -character->physics.max_speed, character->physics.air_acceleration * dt);
            }
        }
    } 
    else {
        character->physics.state &= ~ MOVING_LEFT;
    } 
}

void handle_jump(character *character, input_state *input)
{
    // Handle jumping
    if ((input->active_actions & ACTION_JUMP))
    { 
        character->physics.state &= ~ GROUNDED;
        character->jump_buffer_frames = JUMP_BUFFER_MAX; // Reset jump buffer when jump is pressed
    } 
    else if (character->jump_buffer_frames > 0) {
        character->jump_buffer_frames--;
    }
        
    if (character->jump_buffer_frames > 0 && character->coyote_frames > 0) {
        character->physics.vy = character->physics.jump_force;
        character->physics.state &= ~ GROUNDED;
        character->physics.state |= JUMPING;
        character->jump_buffer_frames = 0;
        character->coyote_frames = 0;
    } 
}

void move_character(character *character, float dt)
{
    character->x += character->physics.vx * dt;
    character->y += character->physics.vy * dt;
}