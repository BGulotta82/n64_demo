#include "level.h"


static enemy_registry_t g_enemies = { NULL, 0, 0 };

int get_enemy_count(void) {
    return g_enemies.count;
}

character* get_enemy_at(int index) {
    if (index < 0 || index >= g_enemies.count) return NULL;
    return &g_enemies.data[index];
}

void init_enemy_registry(int initial_capacity) {
    g_enemies.capacity = initial_capacity;
    g_enemies.count = 0;
    g_enemies.data = (character *)malloc(initial_capacity * sizeof(character));
}

// Take a pointer. Use 'const' because we are only reading the data, not changing it.
bool spawn_enemy(const character *new_enemy) {
    if (g_enemies.count >= g_enemies.capacity) {
        int next_capacity = (g_enemies.capacity == 0) ? 16 : g_enemies.capacity * 2;
        
        // Safe realloc check
        character *temp = (character *)realloc(g_enemies.data, next_capacity * sizeof(character));
        if (temp == NULL) return false; // Out of memory! Safe abort.

        g_enemies.data = temp;
        g_enemies.capacity = next_capacity;
    }

    // Dereference the pointer (*) to copy the structural contents into the array
    g_enemies.data[g_enemies.count] = *new_enemy; 
    g_enemies.count++;
    
    return true; // Success
}

void destroy_enemy(int index) {
    if (index < 0 || index >= g_enemies.count) return;

    // Fast Unordered Deletion: 
    // Swap the dead element with the absolute last element in the array, then decrement count.
    g_enemies.data[index] = g_enemies.data[g_enemies.count - 1];
    g_enemies.count--;
}

void cleanup_enemy_registry(void) {
    if (g_enemies.data) {
        free(g_enemies.data);
        g_enemies.data = NULL;
    }
    g_enemies.count = 0;
    g_enemies.capacity = 0;
}

void load_level_binary(const char *dfs_path, level_t *level, int num_players) {
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

    spawn_entities(level, num_players);
}

void spawn_entities(level_t *level, int num_players){
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
                break;
            }
            else if (tile_id == ENEMY_SPAWN) {

                // spawn x number of entities at the enemy spawn point 
                // based on how many players are in the game
                if (num_players == 0)
                    num_players = 1;

                int enemies_to_spawn = 1;
                switch(num_players) {
                    case 1:
                        enemies_to_spawn = 3;
                        break;
                    case 2:
                        enemies_to_spawn = 6;
                        break;
                    case 3:
                        enemies_to_spawn = 8;
                        break;
                    case 4:
                        enemies_to_spawn = 10;
                        break;
                }

                for (int i = 0; i < enemies_to_spawn; i++) {
                    character enemy = {0};
                    character_type enemy_type = (rand() % 2) + 4;

                    // Use current total count for ID
                    character_init(&enemy, enemy_type, true, level->number_of_enemies);

                    enemy.x = (float)(x * TILE_SIZE);
                    enemy.y = (float)(y * TILE_SIZE) - ((float)enemy.meta.height - (float)TILE_SIZE); 
                    
                    enemy.meta.state |= ACTIVE;
                    enemy.physics.state |= MOVING_LEFT;

                    // Pass the address to avoid pushing the entire struct on the stack
                    if (spawn_enemy(&enemy)) {
                        level->number_of_enemies++;
                    }
                }

                // Clear the map spot back to Air (0) so it doesn't block movement
                level->map_data[y * MAP_WIDTH + x] = 0;
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