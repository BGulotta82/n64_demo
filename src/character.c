#include "character.h"
#include "constants.h"
#include <string.h>

void character_init(character *character) {
    if(!character) return;
    character->x = 0.0f;
    character->y = 150.0f;
    character->vx = 0.0f;
    character->vy = 0.0f;
    character->is_grounded = false;
    character->coyote_frames = 0;
    character->jump_buffer_frames = 0;
}

void character_update(character *character, input_state *input) {
    if (!character || !input) return;

    // Apply physics calculations...
    if (input->active_actions == ACTION_NONE)
    {
        character->vx = 0.0f; // Stop horizontal movement if no input
    }

    handle_move_left(character, input);
    handle_move_right(character, input);
    handle_jump(character, input);
    move_character(character);
    apply_gravity(character);
    check_grounded(character);
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

void apply_gravity(character *character)
{
    // 1. Apply Gravity if in the air
    if (!character->is_grounded)
    {
        character->vy += GRAVITY;
        if (character->vy > TERMINAL_VELOCITY)
        {
            character->vy = TERMINAL_VELOCITY;
        }
    }
}

void handle_move_left(character *character, input_state *input)
{
    if (input->active_actions & ACTION_MOVE_LEFT)
    {
        if (character->is_grounded)
            character->vx = -RUN_SPEED;
        else {
            character->vx += -AIR_ACCEL;
            if (character->vx < -RUN_SPEED) character->vx = -RUN_SPEED;                 
        }
    }
}

void handle_move_right(character *character, input_state *input)
{
    if (input->active_actions & ACTION_MOVE_RIGHT)
    {
        if (character->is_grounded)
            character->vx = RUN_SPEED;
        else {
            character->vx += AIR_ACCEL;
            if (character->vx > RUN_SPEED) character->vx = RUN_SPEED;            
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
    } else if (character->jump_buffer_frames > 0) {
        character->jump_buffer_frames--;
    }
        
    if (character->jump_buffer_frames > 0 && character->coyote_frames > 0) {
        character->vy = JUMP_FORCE;
        character->is_grounded = 0;
        character->jump_buffer_frames = 0;
        character->coyote_frames = 0;
    }
}

void move_character(character *character)
{
    character->x += character->vx;
    character->y += character->vy;
}