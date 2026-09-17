#include "engine.h"
#include "renderer.h"

float calculate_delta_time(unsigned long long *last_ticks);

int main(void) {
    game_state_t state;

    dfs_init(DFS_DEFAULT_LOCATION);
    renderer_init();
    timer_init();
    engine_init(&state);    

    unsigned long long last_ticks = timer_ticks();
   
    while (1) {
        // 1. SAFELY Lock the backbuffer. 
        // If the RDP is completely busy or the TV isn't ready, this returns NULL
        surface_t *disp = display_get();
        
        // 2. THE FIXED VS-YNC PACER:
        // If it returns NULL, we skip the rest of the frame calculations. 
        // This naturally throttles your CPU loops directly to the N64's video interrupts!
        if (!disp) {
            continue; 
        }

        // 3. NOW calculate delta time. Because the loop is throttled by the display lock, 
        // dt will be beautifully stable (approx 0.0166s / 60 FPS).
        float dt = calculate_delta_time(&last_ticks);
        if (dt > 0.1f) dt = 0.1f; 

        // 4. Run your game logic updates
        engine_update(&state, dt);

        // =========================================================================
        // 5. GATHER TRACKING DATA & UPDATE DECOUPLED VIEWP_CONFIG CAMERAS
        // =========================================================================
        
        // Count how many players are currently alive in the match
        int player_count = get_player_count(); 
        int joined_players = state.joined_players; 
        
        int config_idx = joined_players - 1; 

        for (int i = 0; i < player_count; i++) {
            character *player = get_player_at(i);
            camera_t *camera = get_camera_at(player->meta.id);
                
            // Look up what screen dimensions this specific quadrant/split should look like
            viewport_layout_t layout = viewport_configs[config_idx][player->meta.id];
            
            // Track this player's camera completely independently of the other slots!
            camera_update_split(
                camera, // Pass this specific player id camera instance
                player->x,     // Target player exact position vectors
                player->y,
                (float)player->meta.width, 
                (float)player->meta.height,
                (float)layout.width,               // Pass dynamic viewport screen constraints
                (float)layout.height, 
                player->physics.facing_direction,
                dt
            );
        }

        // 6. Draw your scene passing down the valid locked pointer
        renderer_draw(disp, &state); 
    }
}

float calculate_delta_time(unsigned long long *last_ticks) {
    unsigned long long current_ticks = timer_ticks();
    unsigned long long elapsed_ticks = current_ticks - *last_ticks;
    *last_ticks = current_ticks;

    float dt = (float)elapsed_ticks / TICKS_PER_SECOND;

    if (dt > 0.1f)   dt = 0.1f;
    if (dt <= 0.0f)  dt = 0.0166f;

    return dt;
}