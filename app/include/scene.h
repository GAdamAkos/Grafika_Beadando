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
void scene_update(Scene* scene, double delta_time);
void scene_draw(Scene* scene, int picked_index, float master_light);
void scene_destroy(Scene* scene);
bool scene_collides(Scene* scene, const AABB* player_box);
int scene_pick(Scene* scene, const Camera* cam);
void scene_interact(Scene* scene, int picked_index);

int scene_get_dynamic_light_count(Scene* scene);

bool scene_get_dynamic_light(
    Scene* scene,
    int active_index,
    float* x, float* y, float* z,
    float* intensity
);

const SceneEntity* scene_get_entity(const Scene* scene, int index);

#endif