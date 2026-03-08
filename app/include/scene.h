#ifndef SCENE_H
#define SCENE_H

#include <stdbool.h>
#include "collision.h"
#include "camera.h"

typedef struct Scene Scene;

bool scene_init(Scene** out_scene, const char* csv_path);

/**
 * Draw scene, and optionally highlight the picked object (index, -1 = none).
 */
void scene_draw(Scene* scene, int picked_index);

void scene_destroy(Scene* scene);

/**
 * Collision test: does player AABB collide with any object AABB?
 */
bool scene_collides(Scene* scene, const AABB* player_box);

/**
 * Crosshair picking: ray from camera forward direction, find closest AABB hit.
 * Returns object index, or -1 if nothing hit.
 */
int scene_pick(Scene* scene, const Camera* cam);

/**
 * Interact with an object (E): toggle switch, update power state.
 */
void scene_interact(Scene* scene, int picked_index);

#endif