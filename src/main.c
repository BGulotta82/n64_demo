#include "engine.h"
#include "renderer.h"
#include "level.h"
#include "camera.h"
#include <libdragon.h>

camera_t camera;

float calculate_delta_time(unsigned long long *last_ticks);

int main(void) {
    game_state_t state;

    dfs_init(DFS_DEFAULT_LOCATION);
    renderer_init();
    timer_init();
    engine_init(&state);
    load_level_binary(&state, "/level_test.bin");
    camera_init(&camera, MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE, SCREEN_WIDTH, SCREEN_HEIGHT);

    unsigned long long last_ticks = timer_ticks();

    while (1) {
        // 1. SAFELY Lock the backbuffer. 
        // If the RDP is completely busy or the TV isn't ready, this returns NULL
        surface_t *disp = display_lock();
        
        // 2. THE FIXED VS-YNC PACER:
        // If it returns NULL, we skip the rest of the frame calculations. 
        // This naturally throttles your CPU loops directly to the N64's video interrupts!
        if (!disp) {
            continue; 
        }

        // 3. NOW calculate delta time. Because the loop is throttled by the display lock, 
        // dt will be beautifully stable (approx 0.0166s / 60 FPS).
        float dt = calculate_delta_time(&last_ticks);
        if (dt > 0.1f) dt = 0.1f; 

        // 4. Run your game logic updates
        engine_update(&state, dt);

        // 5. Gather tracking data and apply sub-pixel rounding to prevent the ground-glitch
        int player_x[MAX_PLAYERS];
        int player_y[MAX_PLAYERS];
        bool player_active[MAX_PLAYERS];

        for (int i = 0; i < MAX_PLAYERS; i++) {
            player_x[i] = (int)(state.players[i].x + 0.5f);
            player_y[i] = (int)(state.players[i].y + 0.5f);
            player_active[i] = state.players[i].active;
        }

        camera_update(&camera, player_x, player_y, player_active, MAX_PLAYERS);
        
        // 6. Draw your scene passing down the valid locked pointer
        renderer_draw(disp, &state); 
    }
}

float calculate_delta_time(unsigned long long *last_ticks) {
    unsigned long long current_ticks = timer_ticks();
    unsigned long long elapsed_ticks = current_ticks - *last_ticks;
    *last_ticks = current_ticks;

    float dt = (float)elapsed_ticks / TICKS_PER_SECOND;

    if (dt > 0.1f)   dt = 0.1f;
    if (dt <= 0.0f)  dt = 0.0166f;

    return dt;
}