#ifndef LEVEL_H
#define LEVEL_H

#include "character.h"
#include "structs.h"
#include "engine.h"
#include <stdint.h>
#include <libdragon.h>
#include <stdbool.h>

uint8_t get_tile_at(uint8_t *map_data, int x, int y);
void load_level_binary(const char *dfs_path, game_state_t *state);
void spawn_entities(game_state_t *state);
void spawn_enemies(int num_enemies_to_spawn, int x, int y);
int get_enemy_count(void);
#endif // LEVEL_H