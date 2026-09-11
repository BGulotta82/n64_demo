#include "level.h"
#include <libdragon.h>

#define PLAYER_SPAWN (101)

// --- THE LOGIC TRANSLATION TABLE ---
uint8_t tile_to_enemy_map[256] = {
    [101] = PLAYER_SPAWN,
    [102] = KNIGHT,
    [103] = ELF,
    [104] = WIZARD,
    [105] = DWARF,
    [106] = GOOMBA,
    [107] = SKELETON
};

void load_level_binary(const char *dfs_path, level_t *level, character *enemies) {
    int fd = dfs_open(dfs_path);
    if (fd < 0) {
        printf("ERROR: Failed to open level file at %s\n", dfs_path);
        return;
    }

    int bytes_read = dfs_read(level->map_data, 1, TOTAL_TILES, (uint32_t)fd);
    dfs_close(fd);

    printf("SUCCESS: Loaded %d bytes from level file.\n", (int)bytes_read);

    if (bytes_read != TOTAL_TILES) {
        printf("WARNING: Expected %d tiles, but only read %d!\n", TOTAL_TILES, (int)bytes_read);
    }

    spawn_entities(level, enemies);
}

void spawn_entities(level_t *level, character *enemies){
    memset(enemies, 0, sizeof(character) * MAX_ENEMIES);

    int enemy_index = 0;
    level->number_of_enemies = 0;

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            uint8_t tile_id = level->map_data[y * MAP_WIDTH + x];

            if (tile_id == PLAYER_SPAWN) {
                // constant since an active player struct object instance hasn't passed through here yet.
                level->spawn_x = (float)(x * TILE_SIZE);
                level->spawn_y = (float)(y * TILE_SIZE) - (PLAYER_HEIGHT - TILE_SIZE);
                
                // Clear out the marker so it acts as empty air
                level->map_data[y * MAP_WIDTH + x] = 0;
            }            
            else if (tile_id > PLAYER_SPAWN && tile_to_enemy_map[tile_id] != 0) {
                if (enemy_index >= MAX_ENEMIES) {
                    level->map_data[y * MAP_WIDTH + x] = 0;
                    continue;
                }

                // Resolve the enum type dynamically from our configuration table
                character_type enemy_type = (character_type)tile_to_enemy_map[tile_id];
                character *enemy = &enemies[enemy_index];
                
                // FIXED LOGICAL CHRONOLOGY: Execute character_init FIRST so the custom meta sizes
                // are extracted and assigned to the struct variables BEFORE calculating position parameters!
                character_init(enemy, enemy_type, true);

                // Calculate spawn boundaries safely
                enemy->x = (float)(x * TILE_SIZE);
                // This ensures 16px Goombas and 8px Skeletons sit perfectly on top of the map tiles.
                enemy->y = (float)(y * TILE_SIZE) - ((float)enemy->meta.height - (float)TILE_SIZE); 
                
                enemy->meta.state |= ACTIVE;
                enemy->physics.state |= MOVING_LEFT;

                // Clear the map spot back to Air (0) so it doesn't block movement
                level->map_data[y * MAP_WIDTH + x] = 0;
                level->number_of_enemies++;

                enemy_index++;
            }
        }
    }
}

// Quick inline lookup function to check tile values using X and Y grid spaces
uint8_t get_tile_at(uint8_t *map_data, int x, int y) {
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
        return 1; // Out of bounds acts as solid wall
    }
    // Convert 2D spatial logic to our flat binary array indexing
    return map_data[y * MAP_WIDTH + x];
}