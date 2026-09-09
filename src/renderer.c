#include <libdragon.h>
#include "renderer.h"
#include "level.h"
#include <stdio.h>

void renderer_init(void) {
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, FILTERS_RESAMPLE);
    rdpq_init();
}

void renderer_draw(const game_state_t *state) {    

    surface_t *disp = display_get();

    // Attach the buffers to the RDP (No z-buffer needed yet)
    rdpq_attach_clear(disp, NULL);

    draw_background();
    draw_level();
    draw_character(state);

    // Send frame buffer to display (TV)
    rdpq_detach_show();
}

void draw_background()
{
    // Fill the background with black
    rdpq_set_mode_fill(RGBA32(0x00, 0x00, 0x00, 0));
    rdpq_fill_rectangle(0, 0, display_get_width(), display_get_height());
}

void draw_character(const game_state_t *state)
{
    // Draw in a white rectangle
    rdpq_set_mode_fill(RGBA32(0xff, 0xff, 0xff, 0));
    rdpq_fill_rectangle((int)state->player1.x, (int)state->player1.y, (int)(state->player1.x + 16), (int)(state->player1.y + 16));
}

void draw_level()
{
    rdpq_set_mode_fill(RGBA32(0x80, 0x80, 0x80, 0));

    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            // Accessible here seamlessly without duplicate array definitions!
            uint8_t tile_id = current_map[y * MAP_WIDTH + x];

            if (tile_id == 1)
            {
                int screen_x1 = x * TILE_SIZE;
                int screen_y1 = y * TILE_SIZE;
                rdpq_fill_rectangle(screen_x1, screen_y1, screen_x1 + TILE_SIZE, screen_y1 + TILE_SIZE);
            }
        }
    }
}
