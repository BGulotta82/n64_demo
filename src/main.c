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
        camera_t *camera = get_camera_at(0);
        viewport_layout_t layout = viewport_configs[0][0];
        if (camera != NULL)
            camera_update(camera, (float)layout.width, (float)layout.height, dt);
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