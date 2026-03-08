#ifndef SCENE_H
#define SCENE_H

#include <stdbool.h>
#include "collision.h"

typedef struct Scene Scene;

bool scene_init(Scene** out_scene, const char* csv_path);
void scene_draw(Scene* scene);
void scene_destroy(Scene* scene);

/**
 * Check whether a player's AABB collides with any scene obstacle AABB.
 * Returns true if collision detected.
 */
bool scene_collides(Scene* scene, const AABB* player_box);

#endif