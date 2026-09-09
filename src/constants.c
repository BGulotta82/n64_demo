#include "constants.h"

// Physics Constants (Adjust these to tweak the "feel" of the jump)
const float FLOOR_Y = 200.0f;     // Dummy floor screen coordinate

// --- Horizontal Movement (A LITTLE FASTER) ---
const float TURN_MULTIPLIER = 3.0f;   
const float RUN_SPEED = 150.0f;       // Up from 130: gives a swifter, more energetic running pace
const float GROUND_ACCEL = 1250.0f;   // Scaled up so reaching max speed still feels tight (~0.12s)
const float GROUND_DRAG = 1150.0f;    // Crisp ground stopping

const float AIR_ACCEL = 600.0f;       // Responsive mid-air steering
const float AIR_DRAG = 180.0f;        // Retained: allows you to cross wider gaps easily

// --- Vertical Jump (HIGHER & SNAPPY) ---
const float JUMP_HEIGHT = 36.0f;      // Up from 28: cleanly clears a stack of two 16x16 blocks
const float TIME_TO_PEAK = 0.24f;     // Tuned for a fast ascent to the new peak height

// Math resolutions automatically scale to perfectly balance the new vertical forces
const float GRAVITY = (2.0f * JUMP_HEIGHT) / (TIME_TO_PEAK * TIME_TO_PEAK);   // Resolves to: 1250.0f
const float JUMP_VELOCITY = -(2.0f * JUMP_HEIGHT) / TIME_TO_PEAK;            // Resolves to: -300.0f

// --- Falling Physics ---
const float TERMINAL_VELOCITY = 400.0f; // Increased slightly to accommodate the heavier gravity step

// --- Input Buffers ---
const int JUMP_BUFFER_MAX = 5;        
const int COYOTE_MAX = 6;             
