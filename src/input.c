#include "input.h"
#include <libdragon.h>
#include <string.h>

void input_init(input_state *state) {
    if (!state) return;
    joypad_init();
    state->active_actions = ACTION_NONE;
    state->move_x = 0.0f;
    state->move_y = 0.0f;
}

void input_update(input_state *state) {
    if (!state) return;
    joypad_poll();
    joypad_inputs_t raw = joypad_get_inputs(0);
    joypad_buttons_t pressed = joypad_get_buttons(0);
    joypad_buttons_t held = joypad_get_buttons_held(0);
    joypad_buttons_t released = joypad_get_buttons_released(0);

    state->active_actions = ACTION_NONE;
    if (raw.btn.d_left) state->active_actions |= ACTION_MOVE_LEFT;
    if (raw.btn.d_right) state->active_actions |= ACTION_MOVE_RIGHT;
    if (pressed.a && !held.a) state->active_actions |= ACTION_JUMP;
    if (pressed.a) state->active_actions |= ACTION_JUMP_HELD;
    if (released.a) state->active_actions |= ACTION_JUMP_RELEASED;
    if (pressed.z && !held.z) state->active_actions |= ACTION_ATTACK;
    
    /* normalized analog stick values (-85..85) to -1.0..1.0 */
    state->move_x = (float)raw.stick_x / 85.0f;
    state->move_y = (float)raw.stick_y / 85.0f;
}
