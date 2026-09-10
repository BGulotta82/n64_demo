#include "character.h"
#include "constants.h"
#include "level.h"
#include "engine.h"
#include <math.h>

void character_init(character *character) {
    if(!character) return;
    character->x = 20.0f;
    character->y = 150.0f;
    character->vx = 0.0f;
    character->vy = 0.0f;
    character->is_grounded = false;
    character->coyote_frames = 0;
    character->jump_buffer_frames = 0;
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
                self->vx = 0.0f;
            } else {
                // Vertical collision
                if (self->y < other->y && self->vy >= 0.0f) {
                    self->y = other->y - 16.0f;
                    self->vy = 0.0f;
                    self->is_grounded = true;
                    self->coyote_frames = COYOTE_MAX;
                } else if (self->y > other->y && self->vy < 0.0f) {
                    self->y = other->y + 16.0f;
                    self->vy = 0.0f;
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

    if (character->is_grounded) {
        // Stops the player quickly on the ground
        character->vx = approach(character->vx, 0, GROUND_DRAG * dt);
    } else {
        // Gently shaves off a bit of forward speed in mid-air
        character->vx = approach(character->vx, 0, AIR_DRAG * dt);
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
        character->vy = 0.0f;
        character->is_grounded = true;
        character->coyote_frames = COYOTE_MAX;
    } 
    else
    {
        character->is_grounded = false;
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
        character->vx = 0.0f;
    }
    if (tile_right == 2) {
        character->x = (float)(tile_right_x * TILE_SIZE - 16.0f);
        character->vx = 0.0f;
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
        if (character->vy < 0.0f) {
            character->vy = 0.0f;
        }
    }
}

void apply_gravity(character *character, float dt)
{
    // 1. Apply Gravity if in the air
    if (!character->is_grounded)
    {
        character->vy += GRAVITY * dt;
        if (character->vy > TERMINAL_VELOCITY)
        {
            character->vy = TERMINAL_VELOCITY;
        }
    }
}

void handle_move_right(character *character, input_state *input, float dt)
{
    if (input->active_actions & ACTION_MOVE_RIGHT)
    {
        if (character->is_grounded) {
            // CHECK FOR TURNAROUND: Player is moving LEFT (< 0) but holding RIGHT
            if (character->vx < 0.0f) {
                character->vx = approach(character->vx, RUN_SPEED, GROUND_ACCEL * TURN_MULTIPLIER * dt);
            } else {
                character->vx = approach(character->vx, RUN_SPEED, GROUND_ACCEL * dt);
            }
        }
        else {
            // Air turnaround (optional: can keep it lower than ground for loose air control)
            if (character->vx < 0.0f) {
                character->vx = approach(character->vx, RUN_SPEED, AIR_ACCEL * 1.5f * dt);
            } else {
                character->vx = approach(character->vx, RUN_SPEED, AIR_ACCEL * dt);
            }
        }
    }
}

void handle_move_left(character *character, input_state *input, float dt)
{
    if (input->active_actions & ACTION_MOVE_LEFT)
    {
        if (character->is_grounded) {
            // CHECK FOR TURNAROUND: Player is moving RIGHT (> 0) but holding LEFT
            if (character->vx > 0.0f) {
                character->vx = approach(character->vx, -RUN_SPEED, GROUND_ACCEL * TURN_MULTIPLIER * dt);
            } else {
                character->vx = approach(character->vx, -RUN_SPEED, GROUND_ACCEL * dt);
            }
        }
        else {
            // Air turnaround
            if (character->vx > 0.0f) {
                character->vx = approach(character->vx, -RUN_SPEED, AIR_ACCEL * 1.5f * dt);
            } else {
                character->vx = approach(character->vx, -RUN_SPEED, AIR_ACCEL * dt);
            }
        }
    }
}

void handle_jump(character *character, input_state *input)
{
    // Handle jumping
    if ((input->active_actions & ACTION_JUMP))
    { 
        character->is_grounded = false;
        character->jump_buffer_frames = JUMP_BUFFER_MAX; // Reset jump buffer when jump is pressed
    } 
    else if (character->jump_buffer_frames > 0) {
        character->jump_buffer_frames--;
    }
        
    if (character->jump_buffer_frames > 0 && character->coyote_frames > 0) {
        character->vy = JUMP_VELOCITY;
        character->is_grounded = 0;
        character->jump_buffer_frames = 0;
        character->coyote_frames = 0;
    }

    bool jump_released = !(input->active_actions & ACTION_JUMP_HELD) || (input->active_actions & ACTION_JUMP_RELEASED);

    if (jump_released && !character->is_grounded && character->vy < 0.0f)
    {
        const float MIN_JUMP_UPWARD_VELOCITY = -60.0f; 

        if (character->vy < MIN_JUMP_UPWARD_VELOCITY) {
            character->vy = MIN_JUMP_UPWARD_VELOCITY;
        }
    }    
}

void move_character(character *character, float dt)
{
    character->x += character->vx * dt;
    character->y += character->vy * dt;
}