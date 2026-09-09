#include <libdragon.h>
#include "renderer.h"
#include <stdio.h>

void renderer_init(void) {
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, FILTERS_RESAMPLE);
    rdpq_init();
}

void renderer_draw(const game_state_t *state) {    

    surface_t *disp = display_get();

    // Attach the buffers to the RDP (No z-buffer needed yet)
    rdpq_attach_clear(disp, NULL);

    // Fill the background with black
    rdpq_set_mode_fill(RGBA32(0x00, 0x00, 0x00, 0));
    rdpq_fill_rectangle(0, 0, display_get_width(), display_get_height());

    // Draw in a white rectangle
    rdpq_set_mode_fill(RGBA32(0xff, 0xff, 0xff, 0));
    rdpq_fill_rectangle((int)state->player1.x, (int)state->player1.y, (int)(state->player1.x + 16), (int)(state->player1.y + 16));

    // Send frame buffer to display (TV)
    rdpq_detach_show();
}
