#ifndef STRUCTS_H
#define STRUCTS_H

#include "constants.h"
#include "enums.h"
#include <stdint.h>
#include <stdbool.h>

#pragma region INPUT_STRUCTS
typedef struct {
    int active_actions;      // Bitmask of current actions
    float move_x;            // Normalized -1.0 to 1.0 (from analog stick)
    float move_y;            // Normalized -1.0 to 1.0
} input_state;
#pragma endregion
#pragma region CAMERA_STRUCTS
typedef struct {
    int screen_x;      // Physical pixel start position on the TV (X axis)
    int screen_y;      // Physical pixel start position on the TV (Y axis)
    int width;         // Width boundary of this player's viewport window
    int height;        // Height boundary of this player's viewport window
} viewport_layout_t;

typedef struct {
    float x;
    float y;
    float width;
    float height;
    float world_width;
    float world_height;
} camera_t;
#pragma endregion
#pragma region CHARACTER_STRUCTS
typedef struct {
    float jump_fast_up_threshold;   // Velocity below which Frame 1 triggers
    float jump_slow_up_threshold;   // Velocity below which Frame 2 triggers
    float apex_threshold;           // Absolute window bounds for Frame 3 (+/-)
    float fall_slow_down_threshold; // Velocity below which Frame 4 triggers
    float walk_deadzone;            // Minimum speed threshold to leave IDLE state
} animation_profile_t;

typedef struct {
    int coyote_frames;       // Time allowed to jump AFTER leaving a ledge
    int jump_buffer_frames, invincibility_frames;  // Time to remember a jump press BEFORE touching down
    int health;
    int current_attack_id;      // Increment this every time the player presses the attack button
    int last_hit_by_attack_id;  //
    int ai_home_row;         
    int ai_jump_cooldown;    
    bool is_enemy;
    int width;   
    int height; 
    anim_state_t current_anim;  // e.g., ANIM_WALK
    int anim_timer;             // Ticks passed in current frame
    int current_frame;
    int current_anim_frame_index;    // 0, 1, 2, 3... (Abstract frame index)
    const animation_profile_t *anim_profile;
    character_state state;                
    character_type type;    
    int id;  
} character_meta;

typedef struct {
    // Current State (Changes every frame)
    float vx, vy;            // Velocity X and Y
    float ax, ay;            // Acceleration X and Y
    physics_state state;
    facing_dir facing_direction;
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
#pragma endregion
#pragma region LEVEL_STRUCTS
typedef struct {
    character *data; // Pointer to the dynamic array buffer
    int count;          // Current number of active projectiles on screen
    int capacity;       // Maximum slots currently allocated
} enemy_registry_t;

typedef struct {
    uint8_t map_data[TOTAL_TILES];
    
    // Initial spawning coordinates for the players
    float spawn_x;
    float spawn_y;
    
    // You can easily add more level metadata here later:
    // int music_id;
    int number_of_enemies;
} level_t;
#pragma endregion
#pragma region PROJECTILE_STRUCTS
typedef struct {
    // Keep from character_meta
    int width, height;
    bool is_enemy;
    int damage_source_id; // Remapped from last_hit_by_attack_id    
    // Animation (if applicable)
    const animation_profile_t *anim_profile;
    int current_anim_frame_index;
    int anim_timer;
    // --- CRITICAL PROJECTILE ADDITIONS ---
    projectile_type type;   // MAGIC, ARROW, etc.
    int damage;             // How much health to subtract on hit
    int lifetime_frames;    // Despawn timer so missed shots don't fly forever
    bool pierces;           // Does it disappear on hit, or go through targets?
    const character *owner;    
    // Physics parameters (if not handled externally)
    float speed;            
    float gravity_scale;    // 0.0 for magic, 1.0 for arrows
} projectile_meta;

typedef struct {
    // Current State
    float vx, vy;         // Velocity X and Y
    float ax, ay;         // Acceleration X and Y
    // Projectile Constants
    float max_speed;
    float air_friction;   // Optional: For drag
    float gravity_scale;  // Optional: 0 for straight lines, 1+ for arcs
    float terminal_velocity;
} projectile_physics;

typedef struct {
    float x, y;
    projectile_meta meta;
    projectile_physics physics; // Physics properties for the projectile
} projectile_t;

typedef struct {
    projectile_t *data; // Pointer to the dynamic array buffer
    int count;          // Current number of active projectiles on screen
    int capacity;       // Maximum slots currently allocated
} projectile_registry_t;
#pragma endregion

#pragma region ENGINE_STRUCTS
typedef struct {
    float x1, y1;
    float x2, y2;
} rect_t;

typedef struct {
    int frame;
    input_state input[MAX_PLAYERS];
    character players[MAX_PLAYERS];
    level_t level; 
    int level_index; 
    float level_timer;
    int total_enemies_left;
    match_state_t match_state; 
} game_state_t;

typedef struct {
    hitbox_style_t style;
    float width;
    float height;
    float offset_x; // Position relative to character X (accounts for facing direction)
    float offset_y; // Position relative to character Y
} hitbox_config_t;

typedef struct {
    const char *filename;
    float time_limit;
} stage_config_t;

typedef struct {
    int frame_count;   // Abstract number of frames in this action
    int frame_duration;// How many game ticks to hold each frame
    int hitbox_start_frame; // The frame the hitbox becomes active
    int hitbox_end_frame;   // The frame the hitbox disappears
    hitbox_config_t hitbox; 
} anim_config_t;
#pragma endregion
#endif // STRUCTS_H