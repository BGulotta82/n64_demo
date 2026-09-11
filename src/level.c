#include "level.h"
#include <libdragon.h>

#define PLAYER_SPAWN (102)

// --- THE LOGIC TRANSLATION TABLE ---
character_type tile_to_enemy_map[256] = {
    [100] = GOOMBA,
    [101] = SKELETON,
    [102] = PLAYER_SPAWN
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
    memset(enemies, 0, sizeof(&enemies));

    int enemy_index = 0;

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            uint8_t tile_id = level->map_data[y * MAP_WIDTH + x];

            if (tile_id == PLAYER_SPAWN) {
                level->spawn_x = (float)(x * TILE_SIZE);
                level->spawn_y = (float)(y * TILE_SIZE) - (PLAYER_HEIGHT - TILE_SIZE);
                
                // Clear out the marker so it acts as empty air
                level->map_data[y * MAP_WIDTH + x] = 0;
            }            
            else if (tile_id >= 100 && tile_to_enemy_map[tile_id] != 0) {
                if (enemy_index >= MAX_ENEMIES) {
                    level->map_data[y * MAP_WIDTH + x] = 0;
                    continue;
                }

                // Resolve the enum type dynamically from our configuration table
                character_type determined_type = tile_to_enemy_map[tile_id];
                character *enemy = &enemies[enemy_index];
                
                // Initialize using your uniform engine functions
                character_init(enemy, determined_type);

                // Calculate spawn boundaries safely
                enemy->x = (float)(x * TILE_SIZE);
                enemy->y = (float)(y * TILE_SIZE) - (PLAYER_HEIGHT - TILE_SIZE); 
                enemy->active = true;
                enemy->physics.state |= MOVING_LEFT;

                // Clear the map spot back to Air (0) so it doesn't block movement
                level->map_data[y * MAP_WIDTH + x] = 0;

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