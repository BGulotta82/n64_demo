#include "renderer.h"

// Global font handle
extern anim_config_t character_anims[CHAR_TYPE_MAX][NUMBER_OF_ANIMATION_STATES];

sprite_t* level_tilesheet;

typedef struct {
    int offset_x;  // Manual pixel adjustment: positive moves right, negative moves left
    int offset_y;  // Manual pixel adjustment: positive moves down, negative moves up
    float flip_offset_correction;
    int frame_width;
    sprite_t* sprite_sheet;
} visual_layout_t;

visual_layout_t character_visuals[NUMBER_OF_CHARACTER_TYPES][NUMBER_OF_ANIMATION_STATES] = {
    [KNIGHT] = {
        [ANIM_IDLE] = { .offset_x = -18.0f, .offset_y = -16.0f, .flip_offset_correction = 4.0f, .frame_width = 48 },
        [ANIM_WALK]   = { .offset_x = -16.0f, .offset_y = -8.0f, .flip_offset_correction = 0.0f, .frame_width = 48 }, 
        [ANIM_ATTACK] = { .offset_x = -22.0f, .offset_y = -16.0f, .flip_offset_correction = -4.0f, .frame_width = 64 }, 
        [ANIM_JUMP]   = { .offset_x = -16.0f, .offset_y = -16.0f, .flip_offset_correction = 0.0f, .frame_width = 48 }
    },
    /*[ELF] = {
        [ANIM_IDLE]   = { .offset_x = SCALE_VAL(4),  .offset_y = SCALE_VAL(16), .flip_offset_correction = SCALE_FLT(2.0f) },
        [ANIM_WALK]   = { .offset_x = SCALE_VAL(6),  .offset_y = SCALE_VAL(16), .flip_offset_correction = SCALE_FLT(1.0f) }, 
        [ANIM_ATTACK] = { .offset_x = SCALE_VAL(-2), .offset_y = SCALE_VAL(16), .flip_offset_correction = SCALE_FLT(4.0f) }, 
        [ANIM_JUMP]   = { .offset_x = SCALE_VAL(-2), .offset_y = SCALE_VAL(16), .flip_offset_correction = SCALE_FLT(4.0f) }
    },
    [WIZARD] = {
        [ANIM_IDLE]   = { .offset_x = SCALE_VAL(4),  .offset_y = SCALE_VAL(16), .flip_offset_correction = SCALE_FLT(2.0f) },
        [ANIM_WALK]   = { .offset_x = SCALE_VAL(6),  .offset_y = SCALE_VAL(16), .flip_offset_correction = SCALE_FLT(1.0f) }, 
        [ANIM_ATTACK] = { .offset_x = SCALE_VAL(-2), .offset_y = SCALE_VAL(16), .flip_offset_correction = SCALE_FLT(4.0f) }, 
        [ANIM_JUMP]   = { .offset_x = SCALE_VAL(-2), .offset_y = SCALE_VAL(16), .flip_offset_correction = SCALE_FLT(4.0f) }
    },
    [DWARF] = {
        [ANIM_IDLE]   = { .offset_x = SCALE_VAL(4),  .offset_y = SCALE_VAL(16), .flip_offset_correction = SCALE_FLT(2.0f) },
        [ANIM_WALK]   = { .offset_x = SCALE_VAL(6),  .offset_y = SCALE_VAL(16), .flip_offset_correction = SCALE_FLT(1.0f) }, 
        [ANIM_ATTACK] = { .offset_x = SCALE_VAL(-2), .offset_y = SCALE_VAL(16), .flip_offset_correction = SCALE_FLT(4.0f) }, 
        [ANIM_JUMP]   = { .offset_x = SCALE_VAL(-2), .offset_y = SCALE_VAL(16), .flip_offset_correction = SCALE_FLT(4.0f) }
    },
    [GOOMBA] = {
        [ANIM_IDLE]   = { .offset_x = SCALE_VAL(4),  .offset_y = SCALE_VAL(16), .flip_offset_correction = SCALE_FLT(2.0f) },
        [ANIM_WALK]   = { .offset_x = SCALE_VAL(6),  .offset_y = SCALE_VAL(16), .flip_offset_correction = SCALE_FLT(1.0f) }, 
        [ANIM_ATTACK] = { .offset_x = SCALE_VAL(-2), .offset_y = SCALE_VAL(16), .flip_offset_correction = SCALE_FLT(4.0f) }, 
        [ANIM_JUMP]   = { .offset_x = SCALE_VAL(-2), .offset_y = SCALE_VAL(16), .flip_offset_correction = SCALE_FLT(4.0f) }
    },
    [SKELETON] = {
        [ANIM_IDLE]   = { .offset_x = SCALE_VAL(4),  .offset_y = SCALE_VAL(16), .flip_offset_correction = SCALE_FLT(2.0f) },
        [ANIM_WALK]   = { .offset_x = SCALE_VAL(6),  .offset_y = SCALE_VAL(16), .flip_offset_correction = SCALE_FLT(1.0f) }, 
        [ANIM_ATTACK] = { .offset_x = SCALE_VAL(-2), .offset_y = SCALE_VAL(16), .flip_offset_correction = SCALE_FLT(4.0f) }, 
        [ANIM_JUMP]   = { .offset_x = SCALE_VAL(-2), .offset_y = SCALE_VAL(16), .flip_offset_correction = SCALE_FLT(4.0f) }
    }*/
};

void renderer_init(void) {
    display_init(RESOLUTION_640x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
    rdpq_init();

    rdpq_font_t *builtin_font = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO);
    rdpq_text_register_font(1, builtin_font);

    level_tilesheet  = sprite_load("rom:/tiles.sprite");
    character_visuals[KNIGHT][ANIM_IDLE].sprite_sheet   = sprite_load("rom:/knight-idle.sprite");
    character_visuals[KNIGHT][ANIM_WALK].sprite_sheet   = sprite_load("rom:/knight-walk.sprite");
    character_visuals[KNIGHT][ANIM_ATTACK].sprite_sheet   = sprite_load("rom:/knight-attack.sprite");
    character_visuals[KNIGHT][ANIM_JUMP].sprite_sheet   = sprite_load("rom:/knight-jump.sprite");
    character_visuals[ELF][ANIM_IDLE].sprite_sheet   = sprite_load("rom:/elf-idle.sprite");
    character_visuals[ELF][ANIM_WALK].sprite_sheet   = sprite_load("rom:/elf-walk.sprite");
    character_visuals[ELF][ANIM_ATTACK].sprite_sheet   = sprite_load("rom:/elf-idle.sprite");
    character_visuals[ELF][ANIM_JUMP].sprite_sheet   = sprite_load("rom:/elf-idle.sprite");
    character_visuals[WIZARD][ANIM_IDLE].sprite_sheet   = sprite_load("rom:/elf-idle.sprite");
    character_visuals[WIZARD][ANIM_WALK].sprite_sheet   = sprite_load("rom:/elf-walk.sprite");
    character_visuals[WIZARD][ANIM_ATTACK].sprite_sheet   = sprite_load("rom:/elf-idle.sprite");
    character_visuals[WIZARD][ANIM_JUMP].sprite_sheet   = sprite_load("rom:/elf-idle.sprite");
    character_visuals[DWARF][ANIM_IDLE].sprite_sheet   = sprite_load("rom:/elf-idle.sprite");
    character_visuals[DWARF][ANIM_WALK].sprite_sheet   = sprite_load("rom:/elf-walk.sprite");
    character_visuals[DWARF][ANIM_ATTACK].sprite_sheet   = sprite_load("rom:/elf-idle.sprite");
    character_visuals[DWARF][ANIM_JUMP].sprite_sheet   = sprite_load("rom:/elf-idle.sprite");
    character_visuals[GOOMBA][ANIM_IDLE].sprite_sheet   = sprite_load("rom:/elf-idle.sprite");
    character_visuals[GOOMBA][ANIM_WALK].sprite_sheet   = sprite_load("rom:/elf-walk.sprite");
    character_visuals[GOOMBA][ANIM_ATTACK].sprite_sheet   = sprite_load("rom:/elf-idle.sprite");
    character_visuals[SKELETON][ANIM_IDLE].sprite_sheet   = sprite_load("rom:/elf-idle.sprite");
    character_visuals[SKELETON][ANIM_WALK].sprite_sheet   = sprite_load("rom:/elf-walk.sprite");
    character_visuals[SKELETON][ANIM_ATTACK].sprite_sheet   = sprite_load("rom:/elf-idle.sprite");
    character_visuals[SKELETON][ANIM_JUMP].sprite_sheet   = sprite_load("rom:/elf-jump.sprite");
}

// Update your function signature to accept surface_t *disp
void renderer_draw(surface_t *disp, const game_state_t *state) {
    
    // Attach the RDP queue directly to the locked surface
    rdpq_attach_clear(disp, NULL);

    draw_dynamic_split_screen(state);

    draw_game_state(state);

    // Detach and flip cleanly at the next VSync interval
    rdpq_detach_show();
}

void draw_game_state(const game_state_t *state)
{
    // Define the dimensions of the overlay box (260x60 pixels wide/tall)
    const int box_w = 260;
    const int box_h = 60;

    // Dynamically calculate the centered coordinates for the rectangle box
    int box_x1 = (SCREEN_WIDTH - box_w) / 2;
    int box_y1 = (SCREEN_HEIGHT - box_h) / 2;
    int box_x2 = box_x1 + box_w;
    int box_y2 = box_y1 + box_h;

    // Dynamic screen center anchors for text alignment
    int center_x = SCREEN_WIDTH / 2;
    int center_y = SCREEN_HEIGHT / 2;

    if (state->match_state == STATE_GAME_OVER) {
        rdpq_set_mode_fill(RGBA32(0x00, 0x00, 0x00, 200));
        rdpq_fill_rectangle(box_x1, box_y1, box_x2, box_y2); 
        
        rdpq_set_mode_standard();
        // Text positions aligned relative to center axis
        rdpq_text_printf(NULL, 1, center_x - 40, center_y - 6, "GAME OVER");
        rdpq_text_printf(NULL, 1, center_x - 92, center_y + 14, "PRESS START TO RETRY STAGE");
    } 
    else if (state->match_state == STATE_LEVEL_CLEARED) {
        rdpq_set_mode_fill(RGBA32(0x10, 0x40, 0x10, 200));
        rdpq_fill_rectangle(box_x1, box_y1, box_x2, box_y2);
        
        rdpq_set_mode_standard();
        rdpq_text_printf(NULL, 1, center_x - 52, center_y - 6, "STAGE CLEARED!");
        rdpq_text_printf(NULL, 1, center_x - 96, center_y + 14, "PRESS START FOR NEXT STAGE");
    } 
    else if (state->match_state == STATE_WAITING_TO_START) {
        rdpq_set_mode_fill(RGBA32(0x00, 0x00, 0x00, 200));
        rdpq_fill_rectangle(box_x1, box_y1, box_x2, box_y2);

        // Blinks every 30 frames at 60 FPS (~0.5 seconds)
        if (state->frame % 60 < 30)
        {
            rdpq_set_mode_fill(RGBA32(0xFF, 0xFF, 0xFF, 200));
            rdpq_fill_rectangle(box_x1, box_y1, box_x2, box_y2);

            rdpq_set_mode_standard();
            rdpq_text_printf(NULL, 1, center_x - 40, center_y - 6, "PRESS START");
        }
    }
}

void draw_dynamic_split_screen(const game_state_t *state) {
    int joined_players = state->joined_players; 
    if (joined_players <= 0) return;
    
    int config_idx = joined_players - 1; 
    int player_count = get_player_count();
    int enemy_count = get_enemy_count();

    for (int i = 0; i < player_count; i++) {
        character *current_player = get_player_at(i);
        camera_t *camera = get_camera_at(current_player->meta.id);
        viewport_layout_t layout = viewport_configs[config_idx][current_player->meta.id];

        // --- N64 HARDWARE SCISSOR WINDOW GATE ---
        rdpq_set_scissor(layout.screen_x, layout.screen_y, layout.screen_x + layout.width, layout.screen_y + layout.height);

        // Pre-calculate floored camera coordinates once per viewport to save heavy FPU cycles
        int cam_x_floor = (int)floorf(camera->x);
        int cam_y_floor = (int)floorf(camera->y);

        // =========================================================================
        // PASS 1: BACKGROUND & WORLD SPRITES (Standard / Copy Mode)
        // =========================================================================
        // draw_map_tiles will set its own internal copy mode cleanly
        draw_map_tiles(&state->level, cam_x_floor, cam_y_floor, layout.screen_x, layout.screen_y, layout.width, layout.height);

        rdpq_sync_pipe();           
        rdpq_set_mode_standard(); 
        rdpq_mode_alphacompare(1); 

        #ifdef DEBUG
        float x_offset = 32.0f;
        float y_offset = 32.0f;

        debug_render_character_telemetry(current_player, layout.screen_x + x_offset, layout.screen_y + y_offset);
        #endif

        // Render Players
        for (int p = 0; p < player_count; p++) {
            character *player = get_player_at(p);
            draw_single_character(player, cam_x_floor, cam_y_floor, layout.screen_x, layout.screen_y, layout.width, layout.height, p);
        }

        // Render Enemies
        for (int e = 0; e < enemy_count; e++) {
            character *enemy = get_enemy_at(e);
            draw_single_character(enemy, cam_x_floor, cam_y_floor, layout.screen_x, layout.screen_y, layout.width, layout.height, e);
        }

        // =========================================================================
        // PASS 2: BATCHED HITBOXES & DEBUG OVERLAYS (Fill Mode)
        // =========================================================================
        // We set Fill Mode ONCE here, completely eliminating state flipping inside the loop!
        rdpq_set_mode_fill(RGBA32(0, 255, 0, 255)); // Default to player green

        #ifdef DEBUG
        // Draw Player Hitboxes
        for (int p = 0; p < player_count; p++) {
            character *player = get_player_at(p);

            // Pass our pre-calculated camera variables to bypass inner floorf() calls
            debug_draw_character_hitbox(player, cam_x_floor, cam_y_floor, layout.screen_x, layout.screen_y, RGBA32(0, 255, 0, 255));
        }
        #endif

        // Switch color once for enemies
        rdpq_set_mode_fill(RGBA32(255, 0, 0, 255));

        // Draw Enemy Hitboxes
        for (int e = 0; e < enemy_count; e++) {
            character *enemy = get_enemy_at(e);

            debug_draw_character_hitbox(enemy, cam_x_floor, cam_y_floor, layout.screen_x, layout.screen_y, RGBA32(255, 0, 0, 255));
        }

        // Clean up project hitboxes (Make sure this function doesn't fight over states)
        debug_draw_projectiles_hitbox(cam_x_floor, cam_y_floor, layout.screen_x, layout.screen_y);
    }

    // Reset scissor to full-screen limits safely
    rdpq_set_scissor(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    rdpq_set_mode_standard(); 
    rdpq_mode_alphacompare(1);
    draw_hud(state);
}

void debug_draw_character_hitbox(const character *chr, int cam_x, int cam_y, int off_x, int off_y, color_t color_rgba) {
    int chr_x_floor = (int)floorf(chr->x);
    int chr_y_floor = (int)floorf(chr->y);

    int screen_x = chr_x_floor - cam_x + off_x;
    int screen_y = chr_y_floor - cam_y + off_y;

    int x1 = screen_x;
    int y1 = screen_y;
    int x2 = screen_x + chr->meta.width;
    int y2 = screen_y + chr->meta.height;

    // Draw lines via optimized thin rectangles without touching RDP state registers
    rdpq_fill_rectangle(x1, y1, x2, y1 + 1);       // Top
    rdpq_fill_rectangle(x1, y2 - 1, x2, y2);       // Bottom
    rdpq_fill_rectangle(x1, y1, x1 + 1, y2);       // Left
    rdpq_fill_rectangle(x2 - 1, y1, x2, y2);       // Right

    // Optional Secondary Hitbox Processing
    rect_t hitbox;
    if (get_character_secondary_hitbox(chr, &hitbox)) {
        int sx1 = (int)floorf(hitbox.x1) - cam_x + off_x;
        int sy1 = (int)floorf(hitbox.y1) - cam_y + off_y;
        int sx2 = (int)floorf(hitbox.x2) - cam_x + off_x;
        int sy2 = (int)floorf(hitbox.y2) - cam_y + off_y;

        rdpq_fill_rectangle(sx1, sy1, sx2, sy1 + 1);   // Top
        rdpq_fill_rectangle(sx1, sy2 - 1, sx2, sy2);   // Bottom
        rdpq_fill_rectangle(sx1, sy1, sx1 + 1, sy2);   // Left
        rdpq_fill_rectangle(sx2 - 1, sy1, sx2, sy2);   // Right
    }
}

void debug_render_character_telemetry(const character *c, float x, float y) {
    if (!c) return;

    char msg[512];
    snprintf(msg, sizeof(msg),
        "--- %s TELEMETRY ---\n"
        "Pos:   X:%.2f  Y:%.2f\n"
        "State: %d  |  Type: %d\n"
        "Vel:   VX:%.2f  VY:%.2f\n"
        "Acc:   AX:%.2f  AY:%.2f\n"
        "Phys State: %d  |  Dir: %d\n"
        "HP:    %d  |  Is Enemy: %s\n"
        "Coyote: %d |  Jump Buf: %d\n"
        "Invinc: %d |  AI Cool: %d\n"
        "Anim:  St:%d  Frm:%d (Idx:%d)\n"
        "Timer: %d",
        c->meta.is_enemy ? "ENEMY" : "PLAYER",
        c->x, c->y,
        c->meta.state, c->meta.type,
        c->physics.vx, c->physics.vy,
        c->physics.ax, c->physics.ay,
        c->physics.state, c->physics.facing_direction,
        c->meta.health, c->meta.is_enemy ? "YES" : "NO",
        c->meta.coyote_frames, c->meta.jump_buffer_frames,
        c->meta.invincibility_frames, c->meta.ai_jump_cooldown,
        c->meta.current_anim, c->meta.current_frame, c->meta.current_anim_frame_index,
        c->meta.anim_timer
    );

    // Modern rdpq_text rendering cleanly parses multi-line (\n) text strings
    rdpq_text_print(NULL, 1, x, y, msg);
}

// Update your signature to accept the pre-calculated floored camera parameters
void draw_single_character(const character *chr, int cam_x_floor, int cam_y_floor, int off_x, int off_y, int view_w, int view_h, int character_index) {
    if (!chr) return;
        
    // --- MULTI-VIEWPORT INDEPENDENT INVINCIBILITY FLICKER ---
    if (!chr->meta.is_enemy && chr->meta.invincibility_frames > 0) {
        if (chr->meta.invincibility_frames % 4 < 2) {
            return; 
        }
    }

    // Convert coordinates using pre-calculated parent floor positions
    int chr_x_floor = (int)floorf(chr->x);
    int chr_y_floor = (int)floorf(chr->y);

    int screen_x = chr_x_floor - cam_x_floor + off_x;
    int screen_y = chr_y_floor - cam_y_floor + off_y;

    const visual_layout_t *vis = &character_visuals[chr->meta.type][chr->meta.current_anim];

    sprite_t *sheet = vis->sprite_sheet;
    if (!sheet) return;

    // FIX: Decouple width and height tracking entirely using your visual configuration layout
    int frame_w = vis->frame_width; 
    int frame_h = sheet->height; 

    if (frame_w <= 0) {
        frame_w = frame_h; // Fall back to a perfect square assumption if uninitialized
    }

    // Apply offset vectors depending on facing direction to fix tracking drift
    if (chr->physics.facing_direction == FACING_LEFT) {
        // Adjust mirroring screen coordinate translation anchors safely
        screen_x += vis->offset_x + (int)vis->flip_offset_correction;
    } else {
        screen_x += vis->offset_x;
    }
    screen_y += vis->offset_y;

    // Viewport Window Culling (Updated to track the true width and height properties)
    if (screen_x + frame_w < off_x  || screen_x > off_x + view_w ||
        screen_y + frame_h < off_y || screen_y > off_y + view_h) {
        return; 
    }

    // FIX: Slice texture blocks horizontally using the actual frame width stride
    int max_sheet_frames = sheet->width / frame_w;
    int visual_frame = chr->meta.current_anim_frame_index;
    if (visual_frame >= max_sheet_frames || visual_frame < 0) {
        visual_frame = 0;
    }

    int tex_src_x = visual_frame * frame_w;

    // Reconfigure parameters to use native hardware blit structures instead of negative scale matrices
    rdpq_blitparms_t parms = {
        .s0 = tex_src_x,              
        .t0 = 0,                      
        .width  = frame_w,  // True horizontal pixel dimension         
        .height = frame_h,  // True vertical pixel dimension         
        .flip_x = (chr->physics.facing_direction == FACING_LEFT) // Native HW flip!
    };
    
    // Direct hardware asset draw call
    rdpq_sprite_blit(sheet, (float)screen_x, (float)screen_y, &parms);
}

void debug_draw_projectiles_hitbox(int cam_x_floor, int cam_y_floor, int off_x, int off_y)
{
    int proj_count = get_projectile_count();
    if (proj_count <= 0) return;

    // Track active state to minimize RDP pipeline flush stalls
    int current_rdp_mode = -1; // -1 = unassigned, 0 = player, 1 = enemy

    for (int i = 0; i < proj_count; i++) {
        projectile_t *proj = get_projectile_at(i);
        if (!proj) continue;

        // Fast conversion without floating-point expansion logic
        int x1 = (int)floorf(proj->x) - cam_x_floor + off_x;
        int y1 = (int)floorf(proj->y) - cam_y_floor + off_y;
        int x2 = x1 + proj->meta.width;
        int y2 = y1 + proj->meta.height;

        // Stateful check: Only modify RDP fill register when ownership changes!
        if (proj->meta.is_enemy) {
            if (current_rdp_mode != 1) {
                rdpq_set_mode_fill(RGBA32(255, 0, 0, 255)); // Red
                current_rdp_mode = 1;
            }
        } else {
            if (current_rdp_mode != 0) {
                rdpq_set_mode_fill(RGBA32(0, 255, 255, 255)); // Cyan
                current_rdp_mode = 0;
            }
        }

        // Fast Draw
        rdpq_fill_rectangle(x1, y1, x2, y1 + 1);       
        rdpq_fill_rectangle(x1, y2 - 1, x2, y2);       
        rdpq_fill_rectangle(x1, y1, x1 + 1, y2);       
        rdpq_fill_rectangle(x2 - 1, y1, x2, y2);       
    }
}

void draw_map_tiles(const level_t *level, int cam_x_floor, int cam_y_floor, int off_x, int off_y, int view_w, int view_h) {
    if (!level || !level_tilesheet) return;

    // 1. Establish the RDP state ONCE
    rdpq_set_mode_copy(true); 
    rdpq_mode_tlut(TLUT_RGBA16);
    rdpq_tex_upload_tlut(sprite_get_palette(level_tilesheet), 0, 16);

    // Get raw pointer to surface details
    surface_t surf = sprite_get_pixels(level_tilesheet);

    // 💡 FIX: Track the last loaded tile ID to create a TMEM cache
    int last_tile_id = -1;

    int start_x = cam_x_floor / TILE_SIZE;
    int start_y = cam_y_floor / TILE_SIZE;
    
    int end_x = (cam_x_floor + view_w) / TILE_SIZE + 1;
    int end_y = (cam_y_floor + view_h) / TILE_SIZE + 1;

    // Safety clamps on map constraints
    if (start_x < 0) start_x = 0;
    if (start_y < 0) start_y = 0;
    if (end_x > MAP_WIDTH)  end_x = MAP_WIDTH;
    if (end_y > MAP_HEIGHT) end_y = MAP_HEIGHT;

    int base_screen_x = off_x - cam_x_floor;
    int base_screen_y = off_y - cam_y_floor;
    int hslices = level_tilesheet->hslices;

    for (int y = start_y; y < end_y; y++) {
        int screen_y = (y * TILE_SIZE) + base_screen_y;
        int map_row_offset = y * MAP_WIDTH;

        for (int x = start_x; x < end_x; x++) {
            uint8_t tile_id = level->map_data[map_row_offset + x];
            
            if (tile_id == 0 || tile_id == PLAYER_SPAWN || tile_id == ENEMY_SPAWN || tile_id == BOSS_SPAWN) {
                continue; 
            }

            int tile_index = tile_id - 1;
            
            // Source coordinates mapping out of the texture sheet
            int tile_x = (tile_index % hslices) * TILE_SIZE;
            int tile_y = (tile_index / hslices) * TILE_SIZE;
            int screen_x = (x * TILE_SIZE) + base_screen_x;

            // 🏎️ TMEM CACHE CHECK: Only reload TMEM if the tile type changes!
            if (tile_id != last_tile_id) {
                rdpq_tex_upload_sub(TILE0, &surf, NULL, tile_x, tile_y, tile_x + TILE_SIZE, tile_y + TILE_SIZE);
                last_tile_id = tile_id;
            }

            // Inner loop remains incredibly lightweight
            rdpq_texture_rectangle(TILE0, 
                                   screen_x, screen_y, 
                                   screen_x + TILE_SIZE, screen_y + TILE_SIZE, 
                                   tile_x, tile_y);
        }
    }
    
    rdpq_sync_pipe();
}

void draw_hud(const game_state_t *state) {
    // 1. Render the top 20px dark banner background spanning the full screen width
    rdpq_set_mode_fill(RGBA32(0x00, 0x00, 0x00, 180)); 
    rdpq_fill_rectangle(0, 0, SCREEN_WIDTH, 20);

    // 2. Prepare standard mode for text blitting
    rdpq_set_mode_standard();

    int player_count = get_player_count();
    for(int i = 0; i < player_count; i++) {
        character *player = get_player_at(i);

        if (player->meta.id <= 1) {
            // P1 & P2: Anchored to the left side of the screen
            int left_offset = 8;
    
            if (player->meta.id == 1) {
                left_offset += 45; // P1 at 8px, P2 at 53px
            }

            char player_string[16]; 
            sprintf(player_string, "P%d:%d", player->meta.id + 1, player->meta.health);

            rdpq_text_printf(NULL, 1, left_offset, 14, player_string);
        } 
        else if (player->meta.id > 1) 
        {
            // P3 & P4: Dynamically anchored to the right edge of the screen
            int right_offset = SCREEN_WIDTH - 96; // Equivalent to 224px when SCREEN_WIDTH is 320
            if (player->meta.id == 3) {
                right_offset += 45; // P3 at (Width - 96px), P4 at (Width - 51px)
            }

            char player_string[16]; 
            sprintf(player_string, "P%d:%d", player->meta.id + 1, player->meta.health);

            rdpq_text_printf(NULL, 1, right_offset, 14, player_string);
        }
    }

    // =========================================================================
    // B. CENTER SCREEN: Enemy Counter & Countdown Timer (Dynamic Center Offset)
    // =========================================================================
    char center_string[32];
    int time_int = (int)state->level_timer;
    if (time_int < 0) time_int = 0;

    int num_enemies = get_enemy_count();

    sprintf(center_string, "FOES:%02d | %03d", num_enemies, time_int);
    
    // Calculates the horizontal middle of the viewport and shifts left by 
    // the text boundary offset (55px) to keep it perfectly centered.
    int center_x = (SCREEN_WIDTH / 2) - 55; 
    rdpq_text_printf(NULL, 1, center_x, 14, center_string);
}
