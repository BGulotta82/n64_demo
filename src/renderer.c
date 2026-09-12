#include <libdragon.h>
#include "renderer.h"
#include "level.h"
#include "camera.h"

// Global font handle
extern camera_t cameras[MAX_VIEWPORTS];
sprite_t* level_tilesheet;
sprite_t* character_sprites[NUMBER_OF_CHARACTER_TYPES];

void renderer_init(void) {
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
    rdpq_init();

    rdpq_font_t *builtin_font = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO);
    rdpq_text_register_font(1, builtin_font);

    level_tilesheet  = sprite_load("rom:/tiles.sprite");
    character_sprites[KNIGHT]   = sprite_load("rom:/knight.sprite");
    character_sprites[ELF]      = sprite_load("rom:/elf.sprite");
    character_sprites[WIZARD]   = sprite_load("rom:/wizard.sprite");
    character_sprites[DWARF]    = sprite_load("rom:/dwarf.sprite");
    character_sprites[GOOMBA]   = sprite_load("rom:/goomba.sprite");
    character_sprites[SKELETON] = sprite_load("rom:/skeleton.sprite");    
}

// Update your function signature to accept surface_t *disp
void renderer_draw(surface_t *disp, const game_state_t *state) {
    
    // Attach the RDP queue directly to the locked surface
    rdpq_attach_clear(disp, NULL);

    draw_dynamic_split_screen(state);

    // Detach and flip cleanly at the next VSync interval
    rdpq_detach_show();
}

void draw_dynamic_split_screen(const game_state_t *state) {
    // 1. Count current live player stats
    int active_count = 0;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].meta.state & ACTIVE) active_count++;
    }

    if (active_count == 0) return;

    int config_idx = active_count - 1; // Match layout index row (0 to 3)
    int current_viewport_slot = 0;

    // 2. Loop over player structures to draw active viewports
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!(state->players[i].meta.state & ACTIVE)) continue;

        // Fetch our dynamic screen dimension layout boundary configurations
        viewport_layout_t layout = viewport_configs[config_idx][current_viewport_slot];

        // =========================================================================
        // --- N64 HARDWARE SCISSOR WINDOW GATE ---
        // =========================================================================
        // Constrain drawing operations tightly to this quadrant block
        rdpq_set_scissor(layout.screen_x, layout.screen_y, layout.screen_x + layout.width, layout.screen_y + layout.height);

        // A. Draw Background Map Map geometry from this specific camera slot view
        // Ensure your tile engine calculates views using cameras[i] and factors layout offsets!
        draw_map_tiles(&state->level, &cameras[i], layout.screen_x, layout.screen_y, layout.width, layout.height);

        // B. Render overlapping active characters inside this quadrant window context
        for (int p = 0; p < MAX_PLAYERS; p++) {
            draw_single_character(&state->players[p], &cameras[i], layout.screen_x, layout.screen_y, layout.width, layout.height);
        }

        // C. Render active AI monsters inside this quadrant window context
        for (int e = 0; e < MAX_ENEMIES; e++) {
            draw_single_character(&state->enemies[e], &cameras[i], layout.screen_x, layout.screen_y, layout.width, layout.height);
        }

        current_viewport_slot++;
    }

      // =========================================================================
    // --- THE HUD PASS (PLACED LAST) ---
    // =========================================================================
    // CRITICAL STEP: Reset the N64 hardware scissor to open full-screen bounds (320x240).
    // If you don't do this, the UI elements drawn for Player 3 or 4 will be completely 
    // cut off and invisible on screen!
    rdpq_set_scissor(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // Configure your transparency and standard rendering modes for user interfaces
    rdpq_set_mode_standard(); 
    rdpq_mode_alphacompare(1);

    // Call your HUD method here!
    draw_hud(state);
}

void draw_single_character(const character *chr, const camera_t *active_cam, int off_x, int off_y, int view_w, int view_h) {
    if (!chr || !(chr->meta.state & ACTIVE)) return;

      // =========================================================================
    // --- ADDED: MULTI-VIEWPORT INDEPENDENT INVINCIBILITY FLICKER ---
    // =========================================================================
    // If the character is a player experiencing active invincibility frames, 
    // skip drawing on alternating frames to create a crisp flashing effect.
    // Changing the modulo values lets you fine-tune the blink speed.
    if (!chr->meta.is_enemy && chr->meta.invincibility_frames > 0) {
        if (chr->meta.invincibility_frames % 4 < 2) {
            return; // Skip drawing ONLY this character instance on this viewport pass!
        }
    }
    
    // 1. Calculate base screen space positions relative to this camera context
    int screen_x = (int)(chr->x + 0.5f) - active_cam->x;
    int screen_y = (int)(chr->y + 0.5f) - active_cam->y;

    // 2. Adjust coordinates by adding the physical viewport anchors on the TV layout
    screen_x += off_x;
    screen_y += off_y;

    // Viewport Window Culling: Only draw if inside this quadrant's frame bounds
    if (screen_x + chr->meta.width < off_x  || screen_x > off_x + view_w ||
        screen_y + chr->meta.height < off_y || screen_y > off_y + view_h) {
        return; 
    }

    sprite_t *sheet = character_sprites[chr->meta.type];
    if (!sheet) return;

    // Scale calculation factoring asset bounds
    float asset_width  = (float)sheet->width;
    float asset_height = (float)sheet->height;
    float dynamic_scale_x = (float)chr->meta.width  / asset_width;
    float dynamic_scale_y = (float)chr->meta.height / asset_height;

    // if (chr->physics.facing_direction == FACING_LEFT) {
    //     dynamic_scale_x = -dynamic_scale_x;
    // }

    rdpq_blitparms_t parms = {
        .s0 = 0, .t0 = 0,
        .width  = sheet->width,
        .height = sheet->height,
        .cx = sheet->width / 2.0f,
        .scale_x = dynamic_scale_x,
        .scale_y = dynamic_scale_y,
    };

    rdpq_sprite_blit(sheet, screen_x, screen_y, &parms);
}

void draw_map_tiles(const level_t *level, const camera_t *active_cam, int off_x, int off_y, int view_w, int view_h) {
    if (!level || !level_tilesheet) return;

    // 1. MUST use standard mode for CI4 (Copy mode cannot parse palettes)
    rdpq_set_mode_standard();
    
    // 2. Configure the Texture Lookup Table and upload the palette
    rdpq_mode_tlut(TLUT_RGBA16);
    rdpq_tex_upload_tlut(sprite_get_palette(level_tilesheet), 0, 16);

    // =========================================================================
    // --- UPDATED: CALCULATE GRIDS DYNAMICALLY PER VIEWPORT PERSPECTIVE ---
    // =========================================================================
    // Replaced global "camera" loops with the incoming explicit viewport boundaries!
    int start_x = active_cam->x / TILE_SIZE;
    int start_y = active_cam->y / TILE_SIZE;
    
    // We add 2 to bounds padding to prevent flashing gaps at screen borders when scrolling fast
    int end_x = (active_cam->x + view_w) / TILE_SIZE + 2;
    int end_y = (active_cam->y + view_h) / TILE_SIZE + 2;

    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {
            if (x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_HEIGHT) {
                continue;
            }

            uint8_t tile_id = level->map_data[y * MAP_WIDTH + x];
            if (tile_id == 0) continue; 

            int tile_index = tile_id - 1;
            int tile_x = (tile_index % level_tilesheet->hslices) * TILE_SIZE;
            int tile_y = (tile_index / level_tilesheet->hslices) * TILE_SIZE;

            // =========================================================================
            // --- UPDATED: APPLY VIEWPORT POSITION SHIFTS AND BOUNDARY CULLING ---
            // =========================================================================
            // A. Calculate screen position matching this camera perspective
            int screen_x = x * TILE_SIZE - active_cam->x;
            int screen_y = y * TILE_SIZE - active_cam->y;

            // B. Add the viewport offsets to snap tiles directly into the correct quadrant cell
            screen_x += off_x;
            screen_y += off_y;

            // C. Strict Viewport Culling: Skip drawing if tile coordinates bleed out of this quadrant box frame
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
    // A. LEFT SIDE: Individual Player Hits Left
    // =========================================================================
    int horizontal_offset = 8;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!(state->players[i].meta.state & ACTIVE)) continue;

        char player_string[16]; 
        sprintf(player_string, "P%d:%d", i + 1, state->players[i].meta.health);

        rdpq_text_printf(NULL, 1, horizontal_offset, 14, player_string);
        horizontal_offset += 45; 
    }

    // =========================================================================
    // B. CENTER SCREEN: Enemy Counter & Countdown Timer
    // =========================================================================
    char center_string[32];
    int time_int = (int)state->level_timer;
    if (time_int < 0) time_int = 0;

    sprintf(center_string, "ENEMIES:%02d | %03d", state->total_enemies_left, time_int);
    rdpq_text_printf(NULL, 1, 140, 14, center_string);

    // =========================================================================
    // C. MASTER STATE TEXT OVERLAYS & CONTROLLER PROMPTS (THE ADDITION)
    // =========================================================================
    if (state->match_state == STATE_GAME_OVER) {
        // Render a large dark box over the center of the viewport screen
        rdpq_set_mode_fill(RGBA32(0x00, 0x00, 0x00, 200));
        rdpq_fill_rectangle(30, 90, 290, 150); // Elongated slightly to fit two lines
        
        rdpq_set_mode_standard();
        // Line 1: Primary Status
        rdpq_text_printf(NULL, 1, 120, 114, "GAME OVER");
        // Line 2: Interactivity Menu prompt (Centered on 320px screen width)
        rdpq_text_printf(NULL, 1, 68, 134, "PRESS START TO RETRY STAGE");
    } 
    else if (state->match_state == STATE_LEVEL_CLEARED) {
        // Render a green tinted victory box overlay
        rdpq_set_mode_fill(RGBA32(0x10, 0x40, 0x10, 200));
        rdpq_fill_rectangle(30, 90, 290, 150);
        
        rdpq_set_mode_standard();
        // Line 1: Primary Status
        rdpq_text_printf(NULL, 1, 108, 114, "STAGE CLEARED!");
        // Line 2: Interactivity Menu prompt
        rdpq_text_printf(NULL, 1, 64, 134, "PRESS START FOR NEXT STAGE");
    }
}

