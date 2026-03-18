#ifndef SCENE_H
#define SCENE_H

#include <stdbool.h>
#include "collision.h"
#include "camera.h"

typedef struct Scene Scene;

typedef struct SceneEntity {
    char id[64];
    char type[32];

    int model_idx;
    int tex_idx;

    float px, py, pz;
    float rx, ry, rz;
    float sx, sy, sz;

    float base_px, base_py, base_pz;

    int state;
    AABB box;
} SceneEntity;

bool scene_init(Scene** out_scene, const char* csv_path);

/**
 * Per-frame scene update (animations).
 */
void scene_update(Scene* scene, double delta_time);

/**
 * Draw scene, and optionally highlight the picked object (index, -1 = none).
 */
void scene_draw(Scene* scene, int picked_index, float master_light);

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

/**
 * Returns the number of currently active lamp lights.
 */
int scene_get_dynamic_light_count(Scene* scene);

/**
 * Query active lamp light data by active index.
 * Returns true if the given active light exists.
 */
bool scene_get_dynamic_light(
    Scene* scene,
    int active_index,
    float* x, float* y, float* z,
    float* intensity
);

/**
 * Get read-only access to a scene entity by index.
 * Returns NULL if index is invalid.
 */
const SceneEntity* scene_get_entity(const Scene* scene, int index);

#endif