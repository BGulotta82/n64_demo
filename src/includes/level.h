#ifndef LEVEL_H
#define LEVEL_H

#include "character.h"
#include "structs.h"
#include <stdint.h>
#include <libdragon.h>

uint8_t get_tile_at(uint8_t *map_data, int x, int y);
void load_level_binary(const char *dfs_path, level_t *level, character *enemies);
void spawn_entities(level_t *level, character *enemies);

#endif // LEVEL_H