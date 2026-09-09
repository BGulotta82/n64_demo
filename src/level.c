#include "level.h"
#include <libdragon.h>

uint8_t current_map[TOTAL_TILES];

void load_level_binary(const char *dfs_path) {
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
}

// Quick inline lookup function to check tile values using X and Y grid spaces
uint8_t get_tile_at(int x, int y) {
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
        return 1; // Out of bounds acts as solid wall
    }
    // Convert 2D spatial logic to our flat binary array indexing
    return current_map[y * MAP_WIDTH + x];
}