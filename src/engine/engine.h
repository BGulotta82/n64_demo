#ifndef ENGINE_H
#define ENGINE_H

typedef struct {
    int frame;
} game_state_t;

void engine_init(game_state_t *state);
void engine_update(game_state_t *state, float dt);

#endif // ENGINE_H
