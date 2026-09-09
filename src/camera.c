#include "camera.h"

void camera_init(camera_t *cam, int world_width, int world_height, int screen_width, int screen_height) {
    cam->x = 0;
    cam->y = 0;
    cam->width = screen_width;
    cam->height = screen_height;
    cam->world_width = world_width;
    cam->world_height = world_height;
}

void camera_update(camera_t *cam, int *player_x, int *player_y, bool *player_active, int player_count) {
    int active_count = 0;

    for (int i = 0; i < player_count; i++) {
        if (player_active[i]) {
            active_count++;
        }
    }

    if (active_count == 0) {
        return;
    }

    // Single active player: always center on them.
    if (active_count == 1) {
        int idx = -1;
        for (int i = 0; i < player_count; i++) {
            if (player_active[i]) {
                idx = i;
                break;
            }
        }

        if (idx < 0) {
            return;
        }

        int target_x = player_x[idx] - (cam->width / 2);
        int target_y = player_y[idx] - (cam->height / 2);

        if (target_x < 0) target_x = 0;
        if (target_y < 0) target_y = 0;
        if (target_x + cam->width > cam->world_width) {
            target_x = cam->world_width - cam->width;
        }
        if (target_y + cam->height > cam->world_height) {
            target_y = cam->world_height - cam->height;
        }

        cam->x = target_x;
        cam->y = target_y;
        return;
    }

    // Compute current group bounds.
    int min_x = 999999;
    int min_y = 999999;
    int max_x = -999999;
    int max_y = -999999;

    for (int i = 0; i < player_count; i++) {
        if (!player_active[i]) {
            continue;
        }

        int left = player_x[i];
        int right = player_x[i] + PLAYER_WIDTH;
        int top = player_y[i];
        int bottom = player_y[i] + PLAYER_HEIGHT;

        if (left < min_x) min_x = left;
        if (top < min_y) min_y = top;
        if (right > max_x) max_x = right;
        if (bottom > max_y) max_y = bottom;
    }

    // Proposed camera center based on the active group.
    int proposed_x = ((min_x + max_x) / 2) - (cam->width / 2);
    int proposed_y = ((min_y + max_y) / 2) - (cam->height / 2);

    // Clamp candidate camera to world bounds.
    if (proposed_x < 0) proposed_x = 0;
    if (proposed_y < 0) proposed_y = 0;
    if (proposed_x + cam->width > cam->world_width) {
        proposed_x = cam->world_width - cam->width;
    }
    if (proposed_y + cam->height > cam->world_height) {
        proposed_y = cam->world_height - cam->height;
    }

    // Reject the move if it would hide any active player.
    for (int i = 0; i < player_count; i++) {
        if (!player_active[i]) {
            continue;
        }

        int left = player_x[i];
        int right = player_x[i] + PLAYER_WIDTH;
        int top = player_y[i];
        int bottom = player_y[i] + PLAYER_HEIGHT;

        if (left < proposed_x || right > proposed_x + cam->width ||
            top < proposed_y || bottom > proposed_y + cam->height) {
            // Keep current camera; do not allow a leader to drag others off-screen.
            return;
        }
    }

    cam->x = proposed_x;
    cam->y = proposed_y;
}