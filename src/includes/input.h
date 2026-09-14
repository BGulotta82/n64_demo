#ifndef INPUT_H
#define INPUT_H

#include "structs.h"
#include <string.h>
#include <libdragon.h>

void input_init(input_state *state);
void input_update(input_state *state, joypad_port_t port);
#endif // INPUT_H
