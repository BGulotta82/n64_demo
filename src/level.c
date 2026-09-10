#include "level.h"
#include <libdragon.h>

uint8_t current_map[TOTAL_TILES];

void load_level_binary(game_state_t *state, char *dfs_path) {
    int fd = dfs_open(dfs_path);
    if (fd < 0) {
        printf("ERROR: Failed to open level file at %s\n", dfs_path);
        return;
    }

    int bytes_read = dfs_read(current_map, 1, TOTAL_TILES, (uint32_t)fd);
    dfs_close(fd);

    printf("SUCCESS: Loaded %d bytes from level file.\n", (int)bytes_read);
    printf("First 16 tiles: ");
    for (int i = 0; i < 16; i++) {
        printf("%u\n ", current_map[i]);
    }
    printf("\n");

    if (bytes_read != TOTAL_TILES) {
        printf("WARNING: Expected %d tiles, but only read %d!\n", TOTAL_TILES, (int)bytes_read);
    }

    spawn_entities(state);
}

void spawn_entities(game_state_t *state){
    int enemy_index = 0;
    
    // Clear out any stale enemies from previous levels
    memset(state->enemies, 0, sizeof(state->enemies));

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            uint8_t tile_id = current_map[y * MAP_WIDTH + x];

            // Define custom tile IDs for your enemies (e.g., ID 100 for Goomba, 101 for Skeleton)
            if (tile_id == 100 || tile_id == 101) {
                if (enemy_index >= MAX_ENEMIES) {
                    printf("WARNING: Maximum enemy count reached! Skipping spawn at grid (%d,%d)\n", x, y);
                    current_map[y * MAP_WIDTH + x] = 0; // Clear it anyway so it's not a mystery block
                    continue;
                }

                character *enemy = &state->enemies[enemy_index];
                character_type type = (tile_id == 100) ? GOOMBA : SKELETON;

                // 1. Initialize enemy structures and assign its specialized type
                character_init(enemy, type);

                // 2. Map screen grid coordinates to exact pixel coordinates
                enemy->x = (float)(x * TILE_SIZE);
                
                // Account for 32px height differences so they don't clip into the floor on spawn
                enemy->y = (float)(y * TILE_SIZE) - (PLAYER_HEIGHT - TILE_SIZE); 
                
                enemy->active = true;
                enemy->physics.state |= MOVING_LEFT; // Give it a starting direction

                // 3. Clear the map position back to 0 (Air) so it behaves normally
                current_map[y * MAP_WIDTH + x] = 0;

                enemy_index++;
            }
        }
    }
    printf("SUCCESS: Dynamically spawned %d enemies from the grid map file.\n", enemy_index);
}

// Quick inline lookup function to check tile values using X and Y grid spaces
uint8_t get_tile_at(int x, int y) {
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
        return 1; // Out of bounds acts as solid wall
    }
    // Convert 2D spatial logic to our flat binary array indexing
    return current_map[y * MAP_WIDTH + x];
}