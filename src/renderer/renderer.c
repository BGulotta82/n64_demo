#include <libdragon.h>
#include "renderer.h"
#include <stdio.h>

void renderer_init(void) {
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, FILTERS_RESAMPLE);
}

void renderer_draw(const game_state_t *state) {
    surface_t *disp = display_get();
    graphics_fill_screen(disp, graphics_make_color(0, 0, 0, 255));
    char buf[32];
    snprintf(buf, sizeof(buf), "Frame %d", state->frame);
    graphics_draw_text(disp, 10, 10, buf);
    display_show(disp);
}
