#include "level.h"

extern const int g_enemies_per_spawn_point[MAX_PLAYERS];

void load_level_binary(const char *dfs_path, game_state_t *state) {
    int fd = dfs_open(dfs_path);
    if (fd < 0) {
        printf("ERROR: Failed to open level file at %s\n", dfs_path);
        return;
    }

    int bytes_read = dfs_read(state->level.map_data, 1, TOTAL_TILES, (uint32_t)fd);
    dfs_close(fd);

    printf("SUCCESS: Loaded %d bytes from level file.\n", (int)bytes_read);

    if (bytes_read != TOTAL_TILES) {
        printf("WARNING: Expected %d tiles, but only read %d!\n", TOTAL_TILES, (int)bytes_read);
    }

    spawn_entities(state);
}

void spawn_entities(game_state_t *state){

    level_t *level = &state->level; 

    int num_players = get_player_count();
    if (num_players == 0 || state->match_state == STATE_GAME_OVER)
        num_players = 1;
        
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            uint8_t tile_id = level->map_data[y * MAP_WIDTH + x];

            if (tile_id == PLAYER_SPAWN) {
                // constant since an active player struct object instance hasn't passed through here yet.
                level->spawn_x = (float)(x * TILE_SIZE);
                level->spawn_y = (float)(y * TILE_SIZE) - (PLAYER_HEIGHT - TILE_SIZE);
            }
            else if (tile_id == ENEMY_SPAWN) {                
                // spawn x number of entities at the enemy spawn point 
                // based on how many players are in the game
                int num_enemies_to_spawn = g_enemies_per_spawn_point[num_players - 1];
                spawn_enemies(num_enemies_to_spawn, x, y);
            }
        }
    }
}

void spawn_enemies(int num_enemies_to_spawn, int x, int y)
{
    int num_enemies = get_enemy_count();

    for (int i = 0; i < num_enemies_to_spawn; i++)
    {
        character enemy = {0};
        character_type enemy_type = (rand() % 2) + 4;

        // Use current total count for ID
        character_init(&enemy, enemy_type, true, num_enemies+1);

        enemy.x = (float)(x * TILE_SIZE);
        enemy.y = (float)(y * TILE_SIZE) - ((float)enemy.meta.height - (float)TILE_SIZE);

        enemy.physics.state |= MOVING_LEFT;

        // Pass the address to avoid pushing the entire struct on the stack
        if (spawn_enemy(&enemy))
        {
            num_enemies++;
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