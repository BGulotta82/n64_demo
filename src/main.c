#include "engine.h"
#include "renderer.h"
#include "main.h"
#include <libdragon.h>

int main(void) {
    game_state_t state;
    
    //console_init();
    //console_set_render_mode(RENDER_MANUAL);
    engine_init(&state);
    renderer_init();

    while (1) {
        engine_update(&state, 1.0f/60.0f);
        renderer_draw(&state);
        //print_debug_info(&state);
    }
}

void print_debug_info(game_state_t *state)
{
    console_clear();
    printf("character info:\n");
    printf("position: (%.2f, %.2f)\n", state->player1.x, state->player1.y);
    printf("velocity: (%.2f, %.2f)\n", state->player1.vx, state->player1.vy);
    printf("grounded: %s\n", state->player1.is_grounded ? "true" : "false");
    printf("coyote frames: %d\n", state->player1.coyote_frames);
    printf("active actions: %u\n", state->input.active_actions);
    printf("coyote frames: %d\n", state->player1.coyote_frames);
    printf("jump buffer frames: %d\n", state->player1.jump_buffer_frames);
    console_render();
}
