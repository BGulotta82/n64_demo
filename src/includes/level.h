#ifndef LEVEL_H
#define LEVEL_H

#include "constants.h"
#include "engine.h"
#include <stdint.h>

void load_level_binary(game_state_t *state, char *dfs_path);
void spawn_entities();
uint8_t get_tile_at(int x, int y);
extern uint8_t current_map[TOTAL_TILES];

#endif // LEVEL_H