#include <libdragon.h>

int main(void) {
    // 1. Initialize core system display hardware
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, FILTERS_RESAMPLE);

    while (1) {
        // 2. Request a clean framebuffer from the hardware
        surface_t *disp = display_get();
        
        // 3. Fill screen color using the matching type conversion macro
        // If you see Green, the entire build-and-flash pipeline is working!
        graphics_fill_screen(disp, graphics_make_color(0, 150, 0, 255)); 
        
        // 4. Output framebuffer to video out
        display_show(disp);
    }
}
