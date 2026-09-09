#include "engine.h"
#include "renderer.h"
#include "level.h"
#include "camera.h"
#include <libdragon.h>

camera_t camera;

float calculate_delta_time(unsigned long long *last_ticks);

int main(void) {
    game_state_t state;

    console_set_render_mode(RENDER_MANUAL);
    timer_init();
    engine_init(&state);
    renderer_init();
    dfs_init(DFS_DEFAULT_LOCATION);
    load_level_binary("/level_test.bin");
    camera_init(&camera, MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE, 320, 240);

    unsigned long long last_ticks = timer_ticks();

    while (1) {
        float dt = calculate_delta_time(&last_ticks);

        engine_update(&state, dt);

        int player_x[MAX_PLAYERS];
        int player_y[MAX_PLAYERS];

        for (int i = 0; i < MAX_PLAYERS; i++) {
            player_x[i] = state.players[i].x;
            player_y[i] = state.players[i].y;
        }

        camera_update(&camera, player_x, player_y, MAX_PLAYERS);
        renderer_draw(&state);
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