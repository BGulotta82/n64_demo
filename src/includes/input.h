#ifndef INPUT_H
#define INPUT_H

typedef enum {
    ACTION_NONE     = 0,
    ACTION_MOVE_LEFT  = 1 << 0,
    ACTION_MOVE_RIGHT = 1 << 1,
    ACTION_JUMP       = 1 << 2,
    ACTION_JUMP_HELD  = 1 << 3,
    ACTION_ATTACK     = 1 << 4
} game_action;

typedef struct {
    int active_actions;      // Bitmask of current actions
    float move_x;            // Normalized -1.0 to 1.0 (from analog stick)
    float move_y;            // Normalized -1.0 to 1.0
} input_state;

void input_init(input_state *state);
void input_update(input_state *state);

#endif // INPUT_H
