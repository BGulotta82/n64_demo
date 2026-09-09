#include "constants.h"

// Physics Constants (Adjust these to tweak the "feel" of the jump)
const float GRAVITY = 0.5f;       // Pulls the player down every frame
const float JUMP_FORCE = -12.0f;   // Initial upward blast (negative moves UP in 2D screen space)
const float TERMINAL_VELOCITY = 14.0f; // Maximum falling speed
const float FLOOR_Y = 200.0f;     // Dummy floor screen coordinate
const int COYOTE_MAX = 6;   // ~100ms at 60fps (allows a 6-frame grace window)
const int JUMP_BUFFER_MAX = 5;   // ~80ms at 60fps
const float RUN_SPEED = 4.5f;
const float AIR_ACCEL = 0.2f; // Slightly slower steering in mid-air