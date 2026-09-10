#include <libdragon.h>
#include "renderer.h"
#include "level.h"
#include "camera.h"

extern camera_t camera;
sprite_t* tilesheet;

void renderer_init(void) {
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, FILTERS_RESAMPLE);
    rdpq_init();
    tilesheet  = sprite_load("rom:/tiles.sprite");
}

void renderer_draw(const game_state_t *state) {
    surface_t *disp = display_get();

    rdpq_attach_clear(disp, NULL);

    draw_level();
    draw_characters(state);

    rdpq_detach_show();
}

void draw_characters(const game_state_t *state) {
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
    // 1. MUST use standard mode for CI4 (Copy mode cannot parse palettes)
    rdpq_set_mode_standard();
    
    // 2. Configure the Texture Lookup Table and upload the palette
    rdpq_mode_tlut(TLUT_RGBA16);
    rdpq_tex_upload_tlut(sprite_get_palette(tilesheet), 0, 16);

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
            if (tile_id == 0) continue; // Assuming 0 is empty/air

            int tile_index = tile_id - 1;
            int tile_x = (tile_index % tilesheet->hslices) * TILE_SIZE;
            int tile_y = (tile_index / tilesheet->hslices) * TILE_SIZE;

            int screen_x = x * TILE_SIZE - camera.x;
            int screen_y = y * TILE_SIZE - camera.y;

            // 3. Define the source rectangle coordinates within the tilesheet
            rdpq_blitparms_t parms = {
                .s0 = tile_x,
                .t0 = tile_y,
                .width = TILE_SIZE,
                .height = TILE_SIZE,
            };

            // 4. Blit directly from the main tilesheet sprite using the parameters
            rdpq_sprite_blit(tilesheet, screen_x, screen_y, &parms);
        }
    }
}
