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
void character_update(character *character, input_state *input, uint8_t *map_data, float dt);
void check_no_input(character *character, input_state *input);
void check_grounded(character *character, uint8_t *map_data);
void check_wall_collision(character *character, uint8_t *map_data);
void check_ceiling_collision(character *character, uint8_t *map_data);
void check_character_collisions(character *self, float dt);
void apply_gravity(character *character, float dt);
void apply_friction(character *character, input_state *input, float dt);
void handle_move_left(character *character, input_state *input, float dt);
void handle_move_right(character *character, input_state *input, float dt);
void move_character(character *character, float dt);
void handle_jump(character *character, input_state *input);
void handle_attack(character *character, input_state *input);

int get_enemy_count(void);
character* get_enemy_at(int index);
void init_enemy_registry(int initial_capacity);
bool spawn_enemy(const character *new_enemy);
void destroy_enemy(int index);
void cleanup_enemy_registry(void);

int get_player_count(void);
character* get_player_at(int index);
void init_player_registry(int initial_capacity);
bool spawn_player(const character *new_player);
void destroy_player(int index);
void cleanup_player_registry(void);


float approach(float current, float target, float step);
 #endif // CHARACTER_H
