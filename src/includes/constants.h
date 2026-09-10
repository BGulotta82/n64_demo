#ifndef CONSTANTS_H
#define CONSTANTS_H

// =========================================================================
// 1. MUST BE #DEFINE: Used for grid tracking, array sizes, and math bounds
// =========================================================================
#define SCREEN_WIDTH    (640)
#define SCREEN_HEIGHT   (480)
#define TILE_SIZE       (16)
#define MAP_WIDTH       (400)
#define MAP_HEIGHT      (30)
#define TOTAL_TILES     (MAP_WIDTH * MAP_HEIGHT)
#define PLAYER_WIDTH    (16)
#define PLAYER_HEIGHT   (16)
#define MAX_PLAYERS     (4)
#define NUMBER_OF_CHARACTER_TYPES (4)

// 2. Input Buffers
#define JUMP_BUFFER_MAX (5)
#define COYOTE_MAX (6)

#endif // CONSTANTS_H
