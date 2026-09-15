#ifndef LEVEL_H
#define LEVEL_H

#include "character.h"
#include "structs.h"
#include <stdint.h>
#include <libdragon.h>

uint8_t get_tile_at(uint8_t *map_data, int x, int y);
void load_level_binary(const char *dfs_path, level_t *level);
void spawn_entities(level_t *level);
int get_enemy_count(void);
character* get_enemy_at(int index);
void init_enemy_registry(int initial_capacity);
void spawn_enemy(character new_enemy);
void destroy_enemy(int index);
void cleanup_enemy_registry(void);

#endif // LEVEL_H