#include <libdragon.h>
#include "renderer.h"
#include "level.h"
#include "camera.h"
#include "character.h"

// Global font handle
extern camera_t cameras[MAX_VIEWPORTS];
sprite_t* level_tilesheet;
sprite_t* character_sprites[NUMBER_OF_CHARACTER_TYPES][NUMBER_OF_ANIMATION_STATES];

typedef struct {
    int offset_x;  // Manual pixel adjustment: positive moves right, negative moves left
    int offset_y;  // Manual pixel adjustment: positive moves down, negative moves up
    float flip_offset_correction;
} visual_layout_t;

static const visual_layout_t character_visual_configs[NUMBER_OF_CHARACTER_TYPES][NUMBER_OF_ANIMATION_STATES] = {
    [KNIGHT] = {
        [ANIM_IDLE]   = { .offset_x = 4,  .offset_y = 16, .flip_offset_correction = 2.0f },
        [ANIM_WALK]   = { .offset_x = 6,  .offset_y = 16, .flip_offset_correction = 1.0f }, // Lean forward slightly
        [ANIM_ATTACK] = { .offset_x = -2, .offset_y = 16, .flip_offset_correction = 4.0f }, // Sword extends forward
    },
    [ELF] = {
        [ANIM_IDLE]   = { .offset_x = 4,  .offset_y = 16, .flip_offset_correction = 2.0f },
        [ANIM_WALK]   = { .offset_x = 6,  .offset_y = 16, .flip_offset_correction = 1.0f }, // Lean forward slightly
        [ANIM_ATTACK] = { .offset_x = -2, .offset_y = 16, .flip_offset_correction = 4.0f }, // Sword extends forward
    },
    [WIZARD] = {
        [ANIM_IDLE]   = { .offset_x = 4,  .offset_y = 16, .flip_offset_correction = 2.0f },
        [ANIM_WALK]   = { .offset_x = 6,  .offset_y = 16, .flip_offset_correction = 1.0f }, // Lean forward slightly
        [ANIM_ATTACK] = { .offset_x = -2, .offset_y = 16, .flip_offset_correction = 4.0f }, // Sword extends forward
    },
    [DWARF] = {
        [ANIM_IDLE]   = { .offset_x = 4,  .offset_y = 16, .flip_offset_correction = 2.0f },
        [ANIM_WALK]   = { .offset_x = 6,  .offset_y = 16, .flip_offset_correction = 1.0f }, // Lean forward slightly
        [ANIM_ATTACK] = { .offset_x = -2, .offset_y = 16, .flip_offset_correction = 4.0f }, // Sword extends forward
    },
    [GOOMBA] = {
        [ANIM_IDLE]   = { .offset_x = 4,  .offset_y = 16, .flip_offset_correction = 2.0f },
        [ANIM_WALK]   = { .offset_x = 6,  .offset_y = 16, .flip_offset_correction = 1.0f }, // Lean forward slightly
        [ANIM_ATTACK] = { .offset_x = -2, .offset_y = 16, .flip_offset_correction = 4.0f }, // Sword extends forward
    },
    [SKELETON] = {
        [ANIM_IDLE]   = { .offset_x = 4,  .offset_y = 16, .flip_offset_correction = 2.0f },
        [ANIM_WALK]   = { .offset_x = 6,  .offset_y = 16, .flip_offset_correction = 1.0f }, // Lean forward slightly
        [ANIM_ATTACK] = { .offset_x = -2, .offset_y = 16, .flip_offset_correction = 4.0f }, // Sword extends forward
    }
};

void renderer_init(void) {
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
    rdpq_init();

    rdpq_font_t *builtin_font = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO);
    rdpq_text_register_font(1, builtin_font);

    level_tilesheet  = sprite_load("rom:/tiles.sprite");
    character_sprites[KNIGHT][ANIM_IDLE]   = sprite_load("rom:/knight.sprite");
    character_sprites[KNIGHT][ANIM_WALK]   = sprite_load("rom:/knight.sprite");
    character_sprites[KNIGHT][ANIM_ATTACK]   = sprite_load("rom:/knight.sprite");
    character_sprites[ELF][ANIM_IDLE]   = sprite_load("rom:/elf.sprite");
    character_sprites[ELF][ANIM_WALK]   = sprite_load("rom:/elf.sprite");
    character_sprites[ELF][ANIM_ATTACK]   = sprite_load("rom:/elf.sprite");
    character_sprites[WIZARD][ANIM_IDLE]   = sprite_load("rom:/wizard.sprite");
    character_sprites[WIZARD][ANIM_WALK]   = sprite_load("rom:/wizard.sprite");
    character_sprites[WIZARD][ANIM_ATTACK]   = sprite_load("rom:/wizard.sprite");
    character_sprites[DWARF][ANIM_IDLE]   = sprite_load("rom:/dwarf.sprite");
    character_sprites[DWARF][ANIM_WALK]   = sprite_load("rom:/dwarf.sprite");
    character_sprites[DWARF][ANIM_ATTACK]   = sprite_load("rom:/dwarf.sprite");
    character_sprites[GOOMBA][ANIM_IDLE]   = sprite_load("rom:/goomba.sprite");
    character_sprites[GOOMBA][ANIM_WALK]   = sprite_load("rom:/goomba.sprite");
    character_sprites[GOOMBA][ANIM_ATTACK]   = sprite_load("rom:/goomba.sprite");
    character_sprites[SKELETON][ANIM_IDLE]   = sprite_load("rom:/skeleton.sprite");
    character_sprites[SKELETON][ANIM_WALK]   = sprite_load("rom:/skeleton.sprite");
    character_sprites[SKELETON][ANIM_ATTACK]   = sprite_load("rom:/skeleton.sprite");
}

// Update your function signature to accept surface_t *disp
void renderer_draw(surface_t *disp, const game_state_t *state) {
    
    // Attach the RDP queue directly to the locked surface
    rdpq_attach_clear(disp, NULL);

    if (state->match_state == STATE_WAITING_TO_START) {
        // Render a large dark box over the center of the viewport screen
        rdpq_set_mode_fill(RGBA32(0x00, 0x00, 0x00, 200));
        rdpq_fill_rectangle(30, 90, 290, 150); // Elongated slightly to fit two lines

        // (Blinks every 30 frames at 60 FPS (~0.5 seconds))
        if (state->frame % 60 < 30) {            
            // Render a large dark box over the center of the viewport screen
            rdpq_set_mode_fill(RGBA32(0xFF, 0xFF, 0xFF, 200));
            rdpq_fill_rectangle(30, 90, 290, 150); // Elongated slightly to fit two lines
            
            rdpq_set_mode_standard();
            // Line 1: Primary Status
            rdpq_text_printf(NULL, 1, 120, 114, "PRESS START");

        }        
    }

    draw_dynamic_split_screen(state);
    
    // Detach and flip cleanly at the next VSync interval
    rdpq_detach_show();
}

void draw_dynamic_split_screen(const game_state_t *state) {
    // Determine the viewport configuration layout structure safely
    int active_count = 0;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].meta.state & ACTIVE) active_count++;
    }

    if (active_count == 0) return;

    int config_idx = active_count - 1; 
    int current_viewport_slot = 0;

    // Loop through ALL camera profiles sequentially to render screens reliably
    for (int i = 0; i < MAX_PLAYERS; i++) {
        // Only skip rendering if the profile is truly unallocated,
        // but decouple player survival states from structural loop limits.
        if (!(state->players[i].meta.state & ACTIVE)) continue;

        viewport_layout_t layout = viewport_configs[config_idx][current_viewport_slot];

        // --- N64 HARDWARE SCISSOR WINDOW GATE ---
        rdpq_set_scissor(layout.screen_x, layout.screen_y, layout.screen_x + layout.width, layout.screen_y + layout.height);

            // Explicit floor-casts prevent fractional alignment offsets
        int cam_x_floor = (int)floorf(cameras[i].x);
        int cam_y_floor = (int)floorf(cameras[i].y);

    // Draw Map Tiles using stabilized tile space constraints
    draw_map_tiles(&state->level, &cameras[i], layout.screen_x, layout.screen_y, layout.width, layout.height);

    // =========================================================================
    // --- RENDER PLAYERS ---
    // =========================================================================
    for (int p = 0; p < MAX_PLAYERS; p++) {
        // Draw the player's visual sprite
        draw_single_character(&state->players[p], &cameras[i], layout.screen_x, layout.screen_y, layout.width, layout.height);
        
        // Draw the player's matching physical hitbox (Bright Green)
        debug_draw_character_hitbox(&state->players[p], &cameras[i], layout.screen_x, layout.screen_y, 0x00FF00FF);
    }

    // =========================================================================
    // --- RENDER AI MONSTERS ---
    // =========================================================================
    for (int e = 0; e < MAX_ENEMIES; e++) {
        // Draw the enemy's visual sprite
        draw_single_character(&state->enemies[e], &cameras[i], layout.screen_x, layout.screen_y, layout.width, layout.height);
        
        // Draw the enemy's matching physical hitbox (Bright Red for clear visibility)
        debug_draw_character_hitbox(&state->enemies[e], &cameras[i], layout.screen_x, layout.screen_y, 0xFF0000FF);
    }

        current_viewport_slot++;
    }

    // Reset scissor to full-screen limits safely
    rdpq_set_scissor(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    rdpq_set_mode_standard(); 
    rdpq_mode_alphacompare(1);
    draw_hud(state);
}

void debug_draw_character_hitbox(const character *chr, const camera_t *active_cam, int off_x, int off_y, uint32_t color_rgba) {
    if (!chr || !(chr->meta.state & ACTIVE)) return;

    // 1. Calculate the exact screen space position using the same floored camera math
    int cam_x_floor = (int)floorf(active_cam->x);
    int cam_y_floor = (int)floorf(active_cam->y);
    int chr_x_floor = (int)floorf(chr->x);
    int chr_y_floor = (int)floorf(chr->y);

    int screen_x = chr_x_floor - cam_x_floor + off_x;
    int screen_y = chr_y_floor - cam_y_floor + off_y;

    // 2. Extract the physical bounds directly from the character's physics meta data
    int x1 = screen_x;
    int y1 = screen_y;
    int x2 = screen_x + chr->meta.width;
    int y2 = screen_y + chr->meta.height;

    // 3. Configure the N64 RDP Blitter to draw primitive outlines
    rdpq_set_mode_fill(RGBA32(
        (color_rgba >> 24) & 0xFF,
        (color_rgba >> 16) & 0xFF,
        (color_rgba >> 8)  & 0xFF,
        color_rgba         & 0xFF
    ));

    // Draw the 4 edges of the box using lines or tight fills
    // Top Edge
    rdpq_fill_rectangle(x1, y1, x2, y1 + 1);
    // Bottom Edge
    rdpq_fill_rectangle(x1, y2 - 1, x2, y2);
    // Left Edge
    rdpq_fill_rectangle(x1, y1, x1 + 1, y2);
    // Right Edge
    rdpq_fill_rectangle(x2 - 1, y1, x2, y2);
}

void draw_single_character(const character *chr, const camera_t *active_cam, int off_x, int off_y, int view_w, int view_h) {
    if (!chr || !(chr->meta.state & ACTIVE)) return;

    // =========================================================================
    // --- MULTI-VIEWPORT INDEPENDENT INVINCIBILITY FLICKER ---
    // =========================================================================
    if (!chr->meta.is_enemy && chr->meta.invincibility_frames > 0) {
        if (chr->meta.invincibility_frames % 4 < 2) {
            return; 
        }
    }

    // =========================================================================
    // --- FLOORED COORDINATE SHIFTS & SCREEN POSITIONING ---
    // =========================================================================
    int cam_x_floor = (int)floorf(active_cam->x);
    int cam_y_floor = (int)floorf(active_cam->y);
    int chr_x_floor = (int)floorf(chr->x);
    int chr_y_floor = (int)floorf(chr->y);

    int screen_x = chr_x_floor - cam_x_floor + off_x;
    int screen_y = chr_y_floor - cam_y_floor + off_y;

    // Fetch the active sprite asset sheet
    sprite_t *sheet = character_sprites[chr->meta.type][chr->meta.current_anim];
    if (!sheet) return;

    int tile_dim = sheet->height; // Returns cell dimensions (e.g., 32)

    // =========================================================================
    // --- EXPLICIT DATA-DRIVEN MANUAL PIXEL OFFSETS ---
    // =========================================================================
 // NEW: Fetch using both type and current animation state
    const visual_layout_t *vis = &character_visual_configs[chr->meta.type][chr->meta.current_anim];

    screen_x += vis->offset_x;
    screen_y += vis->offset_y;

    // =========================================================================

    // Dynamic Viewport Window Culling using our adjusted base coordinates
    if (screen_x + tile_dim < off_x  || screen_x > off_x + view_w ||
        screen_y + tile_dim < off_y || screen_y > off_y + view_h) {
        return; 
    }

    // Safety checks for frame bounds to protect TMEM boundaries
    int max_sheet_frames = sheet->width / tile_dim;
    int visual_frame = chr->meta.current_frame_index;
    if (visual_frame >= max_sheet_frames || visual_frame < 0) {
        visual_frame = 0;
    }

    int tex_src_x = visual_frame * tile_dim;

    float flip_scale_x = 1.0f;
    float center_x = (float)tile_dim / 2.0f;
    float center_y = (float)tile_dim / 2.0f;

    // Adjust the RDP transformation anchor if mirrored to sync with hitbox
    if (chr->physics.facing_direction == FACING_LEFT) {
        flip_scale_x = -1.0f; 
        
      // Dynamically pull the custom pixel correction factor for this character type
        center_x += ((float)vis->offset_x * 2.0f) + vis->flip_offset_correction; 
     }

    rdpq_blitparms_t parms = {
        .s0 = tex_src_x,              
        .t0 = 0,                      
        .width  = tile_dim,           
        .height = tile_dim,           
        
        .scale_x = flip_scale_x,      // Flips the sprite when negative
        
        .cx = center_x,               // Adjusted center axis for correct mirroring
        .cy = center_y,               
    };
    
    rdpq_set_mode_standard(); // Configures the RDP blender for sprite layers
    rdpq_mode_alphacompare(1); // Drops solid background pixels if using a color-key
    
    rdpq_sprite_blit(sheet, (float)screen_x, (float)screen_y, &parms);
}

void draw_map_tiles(const level_t *level, const camera_t *active_cam, int off_x, int off_y, int view_w, int view_h) {
    if (!level || !level_tilesheet) return;

    rdpq_set_mode_standard();
    rdpq_mode_tlut(TLUT_RGBA16);
    rdpq_tex_upload_tlut(sprite_get_palette(level_tilesheet), 0, 16);

    // 1. Explicitly use floorf to convert the camera floats safely to tile tracking indices
    int cam_x_floor = (int)floorf(active_cam->x);
    int cam_y_floor = (int)floorf(active_cam->y);

    int start_x = cam_x_floor / TILE_SIZE;
    int start_y = cam_y_floor / TILE_SIZE;
    
    int end_x = (int)floorf(active_cam->x + (float)view_w) / TILE_SIZE + 2;
    int end_y = (int)floorf(active_cam->y + (float)view_h) / TILE_SIZE + 2;

    // Safety clamps on map constraints to prevent off-boundary array reads
    if (start_x < 0) start_x = 0;
    if (start_y < 0) start_y = 0;
    if (end_x > MAP_WIDTH)  end_x = MAP_WIDTH;
    if (end_y > MAP_HEIGHT) end_y = MAP_HEIGHT;

    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {
            uint8_t tile_id = level->map_data[y * MAP_WIDTH + x];
            if (tile_id == 0) continue; 

            int tile_index = tile_id - 1;
            int tile_x = (tile_index % level_tilesheet->hslices) * TILE_SIZE;
            int tile_y = (tile_index / level_tilesheet->hslices) * TILE_SIZE;

            // =========================================================================
            // --- FIXED: APPLY CALCULATED FLOOR VALUES TO SCREEN SPACE SHIFTS ---
            // =========================================================================
            // Use the pre-calculated floor positions instead of subtracting raw floats.
            // This prevents subpixel truncation drift on the N64 rasterizer.
            int screen_x = (x * TILE_SIZE) - cam_x_floor;
            int screen_y = (y * TILE_SIZE) - cam_y_floor;

            // Add physical viewport offsets to position tiles into the correct quadrant cell
            screen_x += off_x;
            screen_y += off_y;

            // Strict Viewport Culling
            if (screen_x + TILE_SIZE < off_x  || screen_x > off_x + view_w ||
                screen_y + TILE_SIZE < off_y || screen_y > off_y + view_h) {
                continue;
            }

            rdpq_blitparms_t parms = {
                .s0 = tile_x,
                .t0 = tile_y,
                .width = TILE_SIZE,
                .height = TILE_SIZE,
            };

            // Blit directly to the hardware scissored quadrant box matrix
            rdpq_sprite_blit(level_tilesheet, screen_x, screen_y, &parms);
        }
    }
}

void draw_hud(const game_state_t *state) {
    // 1. Render the top 20px dark banner background
    rdpq_set_mode_fill(RGBA32(0x00, 0x00, 0x00, 180)); 
    rdpq_fill_rectangle(0, 0, 320, 20);

    // 2. Prepare standard mode for text blitting
    rdpq_set_mode_standard();

    // =========================================================================
    // A. LEFT SIDE: Players 1 & 2 (Anchored before center text blocks)
    // =========================================================================
    int left_offset = 8;
    for (int i = 0; i < 2; i++) {
        if (!(state->players[i].meta.state & ACTIVE)) continue;

        char player_string[16]; 
        sprintf(player_string, "P%d:%d", i + 1, state->players[i].meta.health);

        rdpq_text_printf(NULL, 1, left_offset, 14, player_string);
        left_offset += 45; // P1 at 8px, P2 at 53px max
    }

    // =========================================================================
    // B. CENTER SCREEN: Enemy Counter & Countdown Timer (Shifted for spacing)
    // =========================================================================
    char center_string[32];
    int time_int = (int)state->level_timer;
    if (time_int < 0) time_int = 0;

    sprintf(center_string, "FOES:%02d | %03d", state->total_enemies_left, time_int);
    // Adjusted from 140 down to 105 to center perfectly inside the safe gap channel
    rdpq_text_printf(NULL, 1, 105, 14, center_string);

    // =========================================================================
    // C. RIGHT SIDE: Players 3 & 4 (Anchored safely past center text channel)
    // =========================================================================
    int right_offset = 224; 
    for (int i = 2; i < MAX_PLAYERS; i++) {
        if (!(state->players[i].meta.state & ACTIVE)) continue;

        char player_string[16]; 
        sprintf(player_string, "P%d:%d", i + 1, state->players[i].meta.health);

        rdpq_text_printf(NULL, 1, right_offset, 14, player_string);
        right_offset += 45; // P3 at 224px, P4 at 269px
    }

    // =========================================================================
    // D. MASTER STATE TEXT OVERLAYS & CONTROLLER PROMPTS
    // =========================================================================
    if (state->match_state == STATE_GAME_OVER) {
        rdpq_set_mode_fill(RGBA32(0x00, 0x00, 0x00, 200));
        rdpq_fill_rectangle(30, 90, 290, 150); 
        
        rdpq_set_mode_standard();
        rdpq_text_printf(NULL, 1, 120, 114, "GAME OVER");
        rdpq_text_printf(NULL, 1, 68, 134, "PRESS START TO RETRY STAGE");
    } 
    else if (state->match_state == STATE_LEVEL_CLEARED) {
        rdpq_set_mode_fill(RGBA32(0x10, 0x40, 0x10, 200));
        rdpq_fill_rectangle(30, 90, 290, 150);
        
        rdpq_set_mode_standard();
        rdpq_text_printf(NULL, 1, 108, 114, "STAGE CLEARED!");
        rdpq_text_printf(NULL, 1, 64, 134, "PRESS START FOR NEXT STAGE");
    }
}