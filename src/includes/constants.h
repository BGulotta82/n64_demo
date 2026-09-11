#ifndef CONSTANTS_H
#define CONSTANTS_H

// =========================================================================
// 1. MUST BE #DEFINE: Used for grid tracking, array sizes, and math bounds
// =========================================================================
#define SCREEN_WIDTH    (320)
#define SCREEN_HEIGHT   (240)
#define TILE_SIZE       (16)
#define MAP_WIDTH       (400)
#define MAP_HEIGHT      (30)
#define TOTAL_TILES     (MAP_WIDTH * MAP_HEIGHT)
#define PLAYER_WIDTH    (16.0f)
#define PLAYER_HEIGHT   (32.0f)
#define MAX_PLAYERS     (4)
#define MAX_ENEMIES     (100)
#define NUMBER_OF_CHARACTER_TYPES (6)
#define MAX_LEVELS      (3)

// 2. Input Buffers
#define JUMP_BUFFER_MAX (5)
#define COYOTE_MAX (6)

#endif // CONSTANTS_H
