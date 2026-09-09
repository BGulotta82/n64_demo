#include "constants.h"

// Physics Constants (Adjust these to tweak the "feel" of the jump)
const float FLOOR_Y = 200.0f;     // Dummy floor screen coordinate

// --- Horizontal Movement ---
const float RUN_SPEED = 140.0f;       // Tailored speed so you cross the screen in ~2.2 seconds
const float GROUND_ACCEL = 1000.0f;   // Smooth acceleration up to full speed (~0.14s)
const float GROUND_DRAG = 950.0f;     // Responsive ground stopping (~0.15s)

const float AIR_ACCEL = 450.0f;       // Gives you loose, satisfying air drift control
const float AIR_DRAG = 180.0f;        // Low friction keeps your forward jump arc wide and natural

// --- Vertical Jump (The 16x16 Fix) ---
const float JUMP_HEIGHT = 28.0f;      // Clears a 16x16 block easily (1.75x character height)
const float TIME_TO_PEAK = 0.32f;     // Faster peak time matches the smaller jump height beautifully

// Math resolutions based on the new 16x16 proportions
const float GRAVITY = (2.0f * JUMP_HEIGHT) / (TIME_TO_PEAK * TIME_TO_PEAK);   // Resolves to: 546.875f
const float JUMP_VELOCITY = -(2.0f * JUMP_HEIGHT) / TIME_TO_PEAK;            // Resolves to: -175.0f

// --- Falling Physics ---
// Set high enough so gravity can smoothly accelerate downward without clipping
const float TERMINAL_VELOCITY = 280.0f; 

// --- Input Buffers ---
const int JUMP_BUFFER_MAX = 5;        // Kept at your solid 5 frames
const int   COYOTE_MAX = 6;   // ~100ms at 60fps (allows a 6-frame grace window)
