#ifndef CONSTANTS_H
#define CONSTANTS_H

// =========================================================================
// 1. MUST BE #DEFINE: Used for grid tracking, array sizes, and math bounds
// =========================================================================
#define TILE_SIZE       (16)
#define MAP_WIDTH       (20)
#define MAP_HEIGHT      (15)
#define TOTAL_TILES     (MAP_WIDTH * MAP_HEIGHT)

// =========================================================================
// 2. ALLOWED AS EXTERN: Purely used as mathematical data in physics loops
// =========================================================================
extern const float GRAVITY;
extern const float JUMP_VELOCITY;
extern const float TERMINAL_VELOCITY;
extern const float FLOOR_Y;
extern const int COYOTE_MAX;
extern const int JUMP_BUFFER_MAX;
extern const float RUN_SPEED;
extern const float AIR_ACCEL;
extern const float GROUND_DRAG;
extern const float AIR_DRAG;
extern const float JUMP_HEIGHT;
extern const float TIME_TO_PEAK;
extern const float GROUND_ACCEL;
extern const float TURN_MULTIPLIER;

#endif // CONSTANTS_H
