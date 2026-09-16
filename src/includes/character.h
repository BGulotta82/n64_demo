#ifndef CHARACTER_H
#define CHARACTER_H

#include "level.h"
#include "structs.h"
#include <math.h>
#include <stdbool.h>

/* 
   Constants for each character type are now stored in a 2D array for easy access and modification. 
   Each row corresponds to a character type (KNIGHT, ELF, WIZARD, DWARF) and contains 
   the following parameters in order: turn_multiplier, max_speed, ground_acceleration, 
   ground_friction, air_acceleration, air_friction, jump_height, jump_time_to_peak, terminal_velocity.
*/   
extern float physics_constants[NUMBER_OF_CHARACTER_TYPES][9];


void character_init(character *character, character_type type, bool is_enemy, int id);
void character_update(character *self, character *players, input_state *input, uint8_t *map_data, float dt);
void check_no_input(input_state *input, character *character);
void check_grounded(character *character, uint8_t *map_data);
void check_wall_collision(character *character, uint8_t *map_data);
void check_ceiling_collision(character *character, uint8_t *map_data);
void check_character_collisions(character *self, character *players, float dt);
float approach(float current, float target, float step);
void apply_gravity(character *character, float dt);
void apply_friction(character *character, input_state *input, float dt);
void handle_move_left(character *character, input_state *input, float dt);
void handle_move_right(character *character, input_state *input, float dt);
void move_character(character *character, float dt);
void handle_jump(character *self, character *players, input_state *input);
void handle_attack(character *character, input_state *input);
character * find_furthest_active_player(character *players, const character *self);

 #endif // CHARACTER_H
