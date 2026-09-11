#ifndef LEVEL_H
#define LEVEL_H

#include "character.h"
#include "constants.h"
#include <stdint.h>

typedef struct {
    uint8_t map_data[TOTAL_TILES];
    
    // Initial spawning coordinates for the players
    float spawn_x;
    float spawn_y;
    
    // You can easily add more level metadata here later:
    // int music_id;
    int time_limit;
    int number_of_enemies;
} level_t;

uint8_t get_tile_at(uint8_t *map_data, int x, int y);
void load_level_binary(const char *dfs_path, level_t *level, character *enemies);
void spawn_entities(level_t *level, character *enemies);

#endif // LEVEL_H