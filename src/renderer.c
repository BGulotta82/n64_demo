#include <libdragon.h>
#include "renderer.h"
#include "level.h"
#include "camera.h"

extern camera_t camera;
sprite_t* level_tilesheet;
sprite_t* character_sprites[NUMBER_OF_CHARACTER_TYPES];

void renderer_init(void) {
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
    rdpq_init();
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

    draw_level(&state->level);
    draw_characters(state);

    // Detach and flip cleanly at the next VSync interval
    rdpq_detach_show();
}

void draw_characters(const game_state_t *state) {
    // 1. Render all active human players
    for (int i = 0; i < MAX_PLAYERS; i++) {
        draw_single_character(&state->players[i]);
    }

    // 2. Render all active AI enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        draw_single_character(&state->enemies[i]);
    }
}

void draw_single_character(const character *chr) {
    if (!chr || !chr->active) return;

    // Calculate rounded screen positions
    int screen_x = (int)(chr->x + 0.5f) - camera.x;
    int screen_y = (int)(chr->y + 0.5f) - camera.y;

    // Culling: Skip drawing if the character is entirely off the screen layout
    if (screen_x + PLAYER_WIDTH < 0 || screen_x > SCREEN_WIDTH ||
        screen_y + PLAYER_HEIGHT < 0 || screen_y > SCREEN_HEIGHT) {
        return; 
    }

    // Safely pull the correct pre-loaded sheet based on this character's type enum
    sprite_t *sheet = character_sprites[chr->type];
    if (!sheet) return;

    // --- ANIMATION FRAME SELECTION ---
    // If your character sheets contain multiple animation frames, you can use Tiled parameters 
    // or frame indexes here. For now, we will draw the first frame (0,0) as a standalone asset:
    rdpq_blitparms_t parms = {
        .s0 = 0,
        .t0 = 0,
        .width = (int)PLAYER_WIDTH,
        .height = (int)PLAYER_HEIGHT,
    };

    // If your sprites are transparent (CI4/CI8 with palettes), configure transparency modes
    rdpq_set_mode_standard(); 
    rdpq_mode_alphacompare(1); // Enables transparency key passing

    // Render the sprite directly via RDP
    rdpq_sprite_blit(sheet, screen_x, screen_y, &parms);
}

void draw_level(level_t *level) {
    if (!level || !level_tilesheet) return;

    // 1. MUST use standard mode for CI4 (Copy mode cannot parse palettes)
    rdpq_set_mode_standard();
    
    // 2. Configure the Texture Lookup Table and upload the palette
    rdpq_mode_tlut(TLUT_RGBA16);
    rdpq_tex_upload_tlut(sprite_get_palette(level_tilesheet), 0, 16);

    int start_x = camera.x / TILE_SIZE;
    int start_y = camera.y / TILE_SIZE;
    int end_x = (camera.x + SCREEN_WIDTH) / TILE_SIZE + 1;
    int end_y = (camera.y + SCREEN_HEIGHT) / TILE_SIZE + 1;

    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {
            if (x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_HEIGHT) {
                continue;
            }

            uint8_t tile_id = level->map_data[y * MAP_WIDTH + x];
            if (tile_id == 0) continue; // Assuming 0 is empty/air

            int tile_index = tile_id - 1;
            int tile_x = (tile_index % level_tilesheet->hslices) * TILE_SIZE;
            int tile_y = (tile_index / level_tilesheet->hslices) * TILE_SIZE;

            int screen_x = x * TILE_SIZE - camera.x;
            int screen_y = y * TILE_SIZE - camera.y;

            // 3. Define the source rectangle coordinates within the level_tilesheet
            rdpq_blitparms_t parms = {
                .s0 = tile_x,
                .t0 = tile_y,
                .width = TILE_SIZE,
                .height = TILE_SIZE,
            };

            // 4. Blit directly from the main level_tilesheet sprite using the parameters
            rdpq_sprite_blit(level_tilesheet, screen_x, screen_y, &parms);
        }
    }
}
