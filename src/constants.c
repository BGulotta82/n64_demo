#include "constants.h"

// Physics Constants (Adjust these to tweak the "feel" of the jump)
const float FLOOR_Y = 200.0f;     // Dummy floor screen coordinate

// --- Horizontal Movement (Slower Running Pace) ---
const float TURN_MULTIPLIER = 3.0f;   
const float RUN_SPEED = 130.0f;       // Keeps your preferred slower running speed
const float GROUND_ACCEL = 1100.0f;   // Bumped slightly to make the initial step snappier
const float GROUND_DRAG = 1000.0f;     

const float AIR_ACCEL = 550.0f;       
const float AIR_DRAG = 180.0f;        

// --- Vertical Jump (THE SNAPPY FIX) ---
const float JUMP_HEIGHT = 28.0f;      // Still clears a 16x16 block with ease
const float TIME_TO_PEAK = 0.22f;     // Down from 0.30s: makes the ascent and descent very quick

// Math resolutions automatically scale up for snappy gravity physics
const float GRAVITY = (2.0f * JUMP_HEIGHT) / (TIME_TO_PEAK * TIME_TO_PEAK);   // Resolves to: 1157.02f
const float JUMP_VELOCITY = -(2.0f * JUMP_HEIGHT) / TIME_TO_PEAK;            // Resolves to: -254.54f

// --- Falling Physics ---
const float TERMINAL_VELOCITY = 380.0f; // Increased so gravity can fully pull the player down fast

// --- Input Buffers ---
const int JUMP_BUFFER_MAX = 5;        
const int COYOTE_MAX = 6;             
