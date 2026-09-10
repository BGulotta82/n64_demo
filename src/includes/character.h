#ifndef CHARACTER_H
#define CHARACTER_H
#include "constants.h"
#include "input.h"
#include <stdbool.h>

/* 
   Constants for each character type are now stored in a 2D array for easy access and modification. 
   Each row corresponds to a character type (KNIGHT, ELF, WIZARD, DWARF) and contains 
   the following parameters in order: turn_multiplier, max_speed, ground_acceleration, 
   ground_friction, air_acceleration, air_friction, jump_height, jump_time_to_peak, terminal_velocity.
*/   
extern float physics_constants[NUMBER_OF_CHARACTER_TYPES][9];

typedef enum {
    KNIGHT     = 0,
    ELF        = 1,
    WIZARD     = 2,
    DWARF      = 3
} character_type;

typedef enum {
    NONE       =  0,
    GROUNDED   =  1 << 0, 
    MOVING_LEFT = 1 << 1, 
    MOVING_RGHT = 1 << 2, 
    JUMPING     = 1 << 3  
} physics_state;

typedef struct {
    // Current State (Changes every frame)
    float vx, vy;            // Velocity X and Y
    float ax, ay;            // Acceleration X and Y
    physics_state state;

    // Movement Constants (Configurable limits)
    float max_speed, turn_multiplier;
    float ground_acceleration, air_acceleration;
    float ground_friction, air_friction;
    float gravity_scale;     // Multiplier for global world gravity
    float jump_height, jump_force, jump_time_to_peak; // Jump force and time to peak for jump calculations
    float terminal_velocity; // Maximum falling speed
} character_physics;

typedef struct {
    float x, y;
    int coyote_frames;       // Time allowed to jump AFTER leaving a ledge
    int jump_buffer_frames;  // Time to remember a jump press BEFORE touching down
    bool active, supported_by_player;               // Is this character active in the game world?
    character_type type;      // Type of character (e.g., KNIGHT, ELF, etc.)
    character_physics physics; // Physics properties for the character
} character;

void character_init(character *character, character_type type);
void character_update(character *self, character *players, input_state *input, float dt);
void check_no_input(input_state *input, character *character);
void check_grounded(character *character);
void check_wall_collision(character *character);
void check_ceiling_collision(character *character);
void check_character_collisions(character *self, character *players, float dt);
float approach(float current, float target, float step);
void apply_gravity(character *character, float dt);
void apply_friction(character *character, input_state *input, float dt);
void handle_move_left(character *character, input_state *input, float dt);
void handle_move_right(character *character, input_state *input, float dt);
void move_character(character *character, float dt);
void handle_jump(character *self, character *players, input_state *input);
 #endif // CHARACTER_H
