#ifndef CHARACTER_H
#define CHARACTER_H
#include <stdbool.h>
#include "input.h"

typedef struct {
    float x, y;
    float vx;
    bool is_jumping;
} character;

void character_init(character *character);
void character_update(character *character, input_state *input);
#endif // CHARACTER_H
