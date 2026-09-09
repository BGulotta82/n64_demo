#include <libdragon.h>
#include "renderer.h"
#include "level.h"
#include "camera.h"

extern camera_t camera;

void renderer_init(void) {
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, FILTERS_RESAMPLE);
    rdpq_init();
}

void renderer_draw(const game_state_t *state) {
    surface_t *disp = display_get();

    rdpq_attach_clear(disp, NULL);

    draw_background();
    draw_level();
    draw_character(state);

    rdpq_detach_show();
}

void draw_background() {
    rdpq_set_mode_fill(RGBA32(0x00, 0x00, 0x00, 0));
    rdpq_fill_rectangle(0, 0, 320, 240);
}

void draw_character(const game_state_t *state) {
    for(int i = 0; i < MAX_PLAYERS; i++) {
        
        if (!state->players[i].active) {
            continue; // Skip inactive players
        }

        int screen_x = state->players[i].x - camera.x;
        int screen_y = state->players[i].y - camera.y;

        rdpq_set_mode_fill(RGBA32(0xFF, 0xFF, 0xFF, 0));
        rdpq_fill_rectangle(screen_x, screen_y, screen_x + 16, screen_y + 16);
    }
}

void draw_level() {
    rdpq_set_mode_fill(RGBA32(0x80, 0x80, 0x80, 0));

    int start_x = camera.x / TILE_SIZE;
    int start_y = camera.y / TILE_SIZE;
    int end_x = (camera.x + 320) / TILE_SIZE + 1;
    int end_y = (camera.y + 240) / TILE_SIZE + 1;

    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {
            if (x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_HEIGHT) {
                continue;
            }

            uint8_t tile_id = current_map[y * MAP_WIDTH + x];

            if (tile_id == 1) {
                int screen_x = x * TILE_SIZE - camera.x;
                int screen_y = y * TILE_SIZE - camera.y;
                rdpq_fill_rectangle(screen_x, screen_y, screen_x + TILE_SIZE, screen_y + TILE_SIZE);
            }
        }
    }
}
