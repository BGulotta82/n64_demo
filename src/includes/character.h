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
    KNIGHT      = 1,
    ELF         = 2,
    WIZARD      = 3,
    DWARF       = 4,
    GOOMBA      = 5,
    SKELETON    = 6
} character_type;

typedef enum {
    PHYSICS_NONE        =  0,
    GROUNDED    =  1 << 0, 
    MOVING_LEFT = 1 << 1, 
    MOVING_RGHT = 1 << 2, 
    JUMPING     = 1 << 3  
} physics_state;

typedef enum {
    CHARACTER_NONE        =  0,
    ACTIVE      =  1 << 0, 
    SPAWNED     = 1 << 1,
    SUPPORTED_BY_PLAYER = 1 << 2  
} character_state;

typedef struct {
    int coyote_frames;       // Time allowed to jump AFTER leaving a ledge
    int jump_buffer_frames, invincibility_frames;;  // Time to remember a jump press BEFORE touching down
    int health;
    bool is_enemy;
    character_state state;                // Is this character active in the game world?
    character_type type;      // Type of character (e.g., KNIGHT, ELF, etc.)
} character_meta;

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
    character_meta meta;
    character_physics physics; // Physics properties for the character
} character;

void character_init(character *character, character_type type);
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

 #endif // CHARACTER_H
