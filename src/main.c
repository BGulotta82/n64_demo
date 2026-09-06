#include "engine/engine.h"
#include "renderer/renderer.h"

int main(void) {
    game_state_t state;

    renderer_init();
    engine_init(&state);

    while (1) {
        engine_update(&state, 1.0f/60.0f);
        renderer_draw(&state);
    }
}
