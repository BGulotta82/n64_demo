#include "character.h"
#include "constants.h"
#include <string.h>
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

void character_update(character *character, input_state *input, float dt) {
    if (!character || !input) return;

    // 1. Process Input & Friction Modifications
    apply_friction(character, input, dt);
    handle_move_left(character, input, dt);
    handle_move_right(character, input, dt);
    handle_jump(character, input); // Triggers your JUMP_VELOCITY

    // 2. Apply Environmental Forces over Time (THE FIX)
    apply_gravity(character, dt);

    // 3. Move the character position based on final velocities
    move_character(character, dt);

    // 4. Resolve Collisions and Reset Ground Flags
    check_grounded(character);
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
    // 5. Basic Environment Collision (Floor Check)
    // Replace this with your actual tilemap/hitbox collision routine
    if (character->y >= FLOOR_Y)
    {
        character->y = FLOOR_Y;
        character->vy = 0.0f;
        character->is_grounded = true;
        character->coyote_frames = COYOTE_MAX;
    } 
    else
    {
        character->is_grounded = false;
        if (character->coyote_frames > 0) {
            character->coyote_frames--; // Count down while in mid-air
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