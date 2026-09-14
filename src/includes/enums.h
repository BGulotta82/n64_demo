#ifndef ENUMS_H
#define ENUMS_H

typedef enum {
    KNIGHT      = 0,
    ELF         = 1,
    WIZARD      = 2,
    DWARF       = 3,
    GOOMBA      = 4,
    SKELETON    = 5,
    CHAR_TYPE_MAX
} character_type;

typedef enum {
    FACING_RIGHT = 0,
    FACING_LEFT  = 1
} facing_dir;

typedef enum {
    PHYSICS_NONE        =  0,
    GROUNDED    =  1 << 0, 
    MOVING_LEFT = 1 << 1, 
    MOVING_RIGHT = 1 << 2, 
    JUMPING     = 1 << 3  
} physics_state;

typedef enum {
    CHARACTER_NONE        =  0,
    ACTIVE      =  1 << 0, 
    SPAWNED     = 1 << 1,
    SUPPORTED_BY_PLAYER = 1 << 2,
    ATTACKING             = 1 << 3,  // 8  
    WANTS_TO_ATTACK       = 1 << 4 // 16
} character_state;

typedef enum {
    ANIM_IDLE,
    ANIM_WALK,
    ANIM_ATTACK,
    ANIM_JUMP,
    NUMBER_OF_ANIMATION_STATES
} anim_state_t;

typedef enum {
    STATE_WAITING_TO_START,
    STATE_PLAYING,
    STATE_LEVEL_CLEARED,
    STATE_GAME_OVER,
    STATE_STAGE_INTRO
} match_state_t;

typedef enum {
    HITBOX_STYLE_NONE,
    HITBOX_STYLE_MELEE_SWEEP, // E.g., Knight's sword arc
    HITBOX_STYLE_PROJECTILE,  // E.g., Wizard's spell origin, Elf's arrow
    HITBOX_STYLE_AOE,         // E.g., Ground slam, explosion
    HITBOX_STYLE_BITE         // E.g., Goomba's direct contact touch
} hitbox_style_t;

typedef enum {
    ACTION_NONE     = 0,
    ACTION_MOVE_LEFT  = 1 << 0,
    ACTION_MOVE_RIGHT = 1 << 1,
    ACTION_JUMP       = 1 << 2,
    ACTION_JUMP_HELD  = 1 << 3,
    ACTION_JUMP_RELEASED = 1 << 4,
    ACTION_ATTACK     = 1 << 5,
    ACTION_START      = 1 << 6
} game_action;
#endif