#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "structs.h"
#include <stdlib.h>
void init_projectile_registry(int initial_capacity);
void spawn_projectile(projectile_t new_proj);
void spawn_projectile_from_character(const character *self);
void destroy_projectile(int index);
void cleanup_projectile_registry(void);
void update_projectiles(float dt);
int get_projectile_count(void);
projectile_t* get_projectile_at(int index);
#endif
