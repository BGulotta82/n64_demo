#include "projectile.h"


void projectile_init(character* character, projectile* projectile) {
    if (character->physics.facing_direction == FACING_LEFT) {
        projectile->x = character->x;
    } else {
        projectile->x = character->x + character->meta.width;
    }

    projectile->y = character->y + (character->meta.height / 2);
}

void projectile_update(projectile* projectile) {
    // update projectile position based on its properties    
}