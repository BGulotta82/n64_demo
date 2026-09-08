#include "engine.h"
#include "renderer.h"

int main(void) {
    game_state_t state;
    
    engine_init(&state);
    renderer_init();

    while (1) {
        engine_update(&state, 1.0f/60.0f);
        renderer_draw(&state);
    }
}
