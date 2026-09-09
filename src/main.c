#include "engine.h"
#include "renderer.h"
#include "main.h"
#include "level.h"
#include <libdragon.h>

float calculate_delta_time(unsigned long long *last_ticks);

int main(void) {
    game_state_t state;
    console_set_render_mode(RENDER_MANUAL);
    timer_init();
    engine_init(&state);
    renderer_init();
    dfs_init(DFS_DEFAULT_LOCATION); 
    load_level_binary("/level_test.bin"); 
    
    // Track the precise time of the previous frame
    unsigned long long last_ticks = timer_ticks();

    while (1) {

        float dt = calculate_delta_time(&last_ticks);

        engine_update(&state, dt);
        renderer_draw(&state);
    }
}

float calculate_delta_time(unsigned long long *last_ticks) {
    // 1. Get the current CPU timestamp
    unsigned long long current_ticks = timer_ticks();
    
    // 2. Subtract to find out how many ticks passed since last frame
    unsigned long long elapsed_ticks = current_ticks - *last_ticks;
    
    // 3. Save the current timestamp for the next iteration loop
    *last_ticks = current_ticks;

    // 4. Convert the raw CPU ticks into fractional seconds
    float dt = (float)elapsed_ticks / TICKS_PER_SECOND;

    // 5. Hard Safeguard (Safety Clamp)
    if (dt > 0.1f)   dt = 0.1f;    // Prevents massive physics breaks if the emulator stutters
    if (dt <= 0.0f)  dt = 0.0166f; // Standard baseline safe fallback

    return dt;
}