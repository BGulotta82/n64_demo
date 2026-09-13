#ifndef ENGINE_H
#define ENGINE_H

#include "level.h"
#include "input.h"
#include "character.h"
#include "camera.h"

typedef enum {
    STATE_WAITING_TO_START,
    STATE_PLAYING,
    STATE_LEVEL_CLEARED,
    STATE_GAME_OVER,
    STATE_STAGE_INTRO
} match_state_t;

typedef struct {
    int frame;
    input_state input[MAX_PLAYERS];
    character players[MAX_PLAYERS];
    character enemies[MAX_ENEMIES];
    level_t level; 
    int level_index; 
    float level_timer;
    int total_enemies_left;
    match_state_t match_state; 
} game_state_t;

typedef enum {
    HITBOX_STYLE_NONE,
    HITBOX_STYLE_MELEE_SWEEP, // E.g., Knight's sword arc
    HITBOX_STYLE_PROJECTILE,  // E.g., Wizard's spell origin, Elf's arrow
    HITBOX_STYLE_AOE,         // E.g., Ground slam, explosion
    HITBOX_STYLE_BITE         // E.g., Goomba's direct contact touch
} hitbox_style_t;

typedef struct {
    float x1, y1;
    float x2, y2;
} rect_t;

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


void engine_init(game_state_t *state);
void engine_update(game_state_t *state, float dt);
void load_stage_by_index(game_state_t *state, int index);
void check_new_player_spawn(character *self, character *players, level_t *level, input_state *input);
void spawn_new_player(character *self, character *players, level_t *level);
void check_pve_combat(game_state_t *state, float dt);
void simulate_enemy_ai(character *enemy, const game_state_t *state, input_state *dummy_input, float dt);
void ai_behavior_skeleton(character *enemy, const character *target, float distance, bool hit_cliff_edge, input_state *dummy_input, bool *move_left, bool *move_right);
void ai_behavior_goomba(const character *enemy, const character *target, bool *move_left, bool *move_right);
void update_character_animation_state(character *self);
bool get_character_secondary_hitbox(const character *chr, rect_t *out_rect);

#endif // ENGINE_H
