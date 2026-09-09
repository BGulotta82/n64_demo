#ifndef CHARACTER_H
#define CHARACTER_H
#include <stdbool.h>
#include "input.h"

typedef struct {
    float x, y;
    float vx, vy;
    bool is_grounded;
    int coyote_frames;       // Time allowed to jump AFTER leaving a ledge
    int jump_buffer_frames;  // Time to remember a jump press BEFORE touching down
} character;

void character_init(character *character);
void character_update(character *character, input_state *input, float dt);
void check_no_input(input_state *input, character *character);
void check_grounded(character *character);
void apply_gravity(character *character);
void apply_friction(character *character, input_state *input, float dt);
void move_character(character *character);
void handle_move_left(character *character, input_state *input);
void handle_move_right(character *character, input_state *input);
void handle_jump(character *character, input_state *input);
#endif // CHARACTER_H
