#include "character.h"
#include <string.h>

void character_init(character *character) {
    memset(character, 0, sizeof(*character));
}

void character_update(character *character, input_state *input) {
    // Handle horizontal movement using decoupled actions
    character->vx = 0.0f;
    if (input->active_actions & ACTION_MOVE_LEFT) {
        character->vx = -4.0f;
    }
    if (input->active_actions & ACTION_MOVE_RIGHT) {
        character->vx = 4.0f;
    }

    // Alternatively, use the decoupled analog float values
    // character->vx = input->move_x * max_speed;

    // Handle jumping
    if ((input->active_actions & ACTION_JUMP) && !character->is_jumping) {
        character->vx = 10.0f;
        character->is_jumping = true;
    }
    
    // Apply physics calculations...
    character->x += character->vx;
}
