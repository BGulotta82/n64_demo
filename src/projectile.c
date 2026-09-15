#include "projectile.h"

static projectile_registry_t g_projectiles = { NULL, 0, 0 };
extern anim_config_t character_anims[CHAR_TYPE_MAX][NUMBER_OF_ANIMATION_STATES];

void init_projectile_registry(int initial_capacity) {
    g_projectiles.capacity = initial_capacity;
    g_projectiles.count = 0;
    g_projectiles.data = (projectile_t *)malloc(initial_capacity * sizeof(projectile_t));
}

void spawn_projectile(projectile_t new_proj) {
    // If we hit capacity limits, dynamically grow the array (doubling strategy)
    if (g_projectiles.count >= g_projectiles.capacity) {
        g_projectiles.capacity = (g_projectiles.capacity == 0) ? 16 : g_projectiles.capacity * 2;
        g_projectiles.data = (projectile_t *)realloc(g_projectiles.data, g_projectiles.capacity * sizeof(projectile_t));
    }

    // Push the new projectile to the back of the active list
    g_projectiles.data[g_projectiles.count] = new_proj;
    g_projectiles.count++;
}

void destroy_projectile(int index) {
    if (index < 0 || index >= g_projectiles.count) return;

    // Fast Unordered Deletion: 
    // Swap the dead element with the absolute last element in the array, then decrement count.
    g_projectiles.data[index] = g_projectiles.data[g_projectiles.count - 1];
    g_projectiles.count--;
}

void cleanup_projectile_registry(void) {
    if (g_projectiles.data) {
        free(g_projectiles.data);
        g_projectiles.data = NULL;
    }
    g_projectiles.count = 0;
    g_projectiles.capacity = 0;
}

void update_projectiles(float dt) {
    // Standard global gravity constant (pixels per frame squared)
    // Adjust this value to make everything fall faster or slower globally

    // Loop backward to safely handle structural modifications (deletions)
    for (int i = g_projectiles.count - 1; i >= 0; i--) {
        projectile_t *p = &g_projectiles.data[i];

        // 1. Update Physics
        p->x += p->physics.vx;
        p->physics.vy += p->physics.gravity_scale * dt;
        if (p->physics.vy > p->physics.terminal_velocity)
        {
            p->physics.vy = p->physics.terminal_velocity;
        }
        p->y += p->physics.vy;
      
        // 2. Manage Lifespans
        p->meta.lifetime_frames--;
        if (p->meta.lifetime_frames <= 0) {
            destroy_projectile(i);
            continue; // Move immediately to next element
        }
    }
}

void spawn_projectile_from_character(const character *self) {
    if (!self)
    return;

    const anim_config_t *atk_cfg = &character_anims[self->meta.type][ANIM_ATTACK];
    if (atk_cfg->hitbox.style != HITBOX_STYLE_PROJECTILE)
    return;

    projectile_t p = {0};
    
    p.meta.owner = self;

    // Determine direction based on flip flags or velocity (e.g., facing left vs right)
    float direction = (self->physics.facing_direction == FACING_LEFT) ? -1.0f : 1.0f;
    float spawn_offset_x = (direction > 0) ? self->meta.width : -16.0f; // Shift spawn past bounding box

    p.x = self->x + spawn_offset_x;
    p.y = self->y + (self->meta.height / 2.5f); // Spawn near hand/chest level
    
    p.meta.is_enemy = self->meta.is_enemy; 
    p.meta.damage_source_id = self->meta.current_attack_id; // Tag with your character tracking loop index

    switch(self->meta.type){
        case WIZARD:
        {
            p.meta.type = MAGIC;
            p.physics.vx = direction * 8.0f;
            p.physics.vy = 0.0f;
            p.meta.width = 8;
            p.meta.height = 8;
            p.meta.damage = 15;
            p.meta.lifetime_frames = 120; // 2 seconds at 60 FPS
            break;
        }
        case ELF: {
            p.meta.type = ARROWS;
            p.physics.vx = direction * 8.0f;
            p.physics.vy = -2.0f; // Slight upward arc
            p.physics.gravity_scale = 10.0f;    // A reasonable per-frame gravity acceleration
            p.physics.terminal_velocity = 8.0f; // Safe falling cap
            p.meta.width = 12;
            p.meta.height = 4;
            p.meta.damage = 10;
            p.meta.lifetime_frames = 180;            
            break;
        }
        default:
            break;
    }

    spawn_projectile(p);
}

int get_projectile_count(void) {
    return g_projectiles.count;
}

projectile_t* get_projectile_at(int index) {
    if (index < 0 || index >= g_projectiles.count) return NULL;
    return &g_projectiles.data[index];
}