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

        // Render Characters
        for (int p = 0; p < MAX_PLAYERS; p++) {
            draw_single_character(&state->players[p], &cameras[i], layout.screen_x, layout.screen_y, layout.width, layout.height);
        }

        // Render AI Monsters
        for (int e = 0; e < MAX_ENEMIES; e++) {
            draw_single_character(&state->enemies[e], &cameras[i], layout.screen_x, layout.screen_y, layout.width, layout.height);
        }

        current_viewport_slot++;
    }

    // Reset scissor to full-screen limits safely
    rdpq_set_scissor(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    rdpq_set_mode_standard(); 
    rdpq_mode_alphacompare(1);
    draw_hud(state);
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
    // --- FIXED: MATCHED FLOORED COORDINATE SHIFTS ---
    // =========================================================================
    // Extract consistent integer floor definitions for both world entities and camera perspectives
    int cam_x_floor = (int)floorf(active_cam->x);
    int cam_y_floor = (int)floorf(active_cam->y);
    int chr_x_floor = (int)floorf(chr->x);
    int chr_y_floor = (int)floorf(chr->y);

    // Calculate base screen space positions relative to this stabilized camera context
    int screen_x = chr_x_floor - cam_x_floor;
    int screen_y = chr_y_floor - cam_y_floor;

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

    // Optional flip logic (Uncomment if needed, it works perfectly with the new math!)
    // if (chr->physics.facing_direction == FACING_LEFT) {
    //     dynamic_scale_x = -dynamic_scale_x;
    // }

    rdpq_blitparms_t parms = {
        .s0 = 0, .t0 = 0,
        .width  = sheet->width,
        .height = sheet->height,
        // Center of rotation/scale pivot matches half width precisely
        .cx = asset_width / 2.0f, 
        .scale_x = dynamic_scale_x,
        .scale_y = dynamic_scale_y,
    };

    rdpq_sprite_blit(sheet, screen_x, screen_y, &parms);
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