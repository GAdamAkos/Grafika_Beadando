#ifndef SCENE_INTERNAL_H
#define SCENE_INTERNAL_H

#include <stdbool.h>

#include "scene.h"
#include "texture.h"
#include "model.h"

typedef struct ModelEntry {
    char path[260];
    Model model;
    bool loaded;
} ModelEntry;

typedef struct TextureEntry {
    char path[260];
    Texture tex;
    bool loaded;
} TextureEntry;

struct Scene {
    ModelEntry* models;
    int model_count;
    int model_cap;

    TextureEntry* textures;
    int tex_count;
    int tex_cap;

    SceneEntity* objects;
    int obj_count;
    int obj_cap;

    int power_on;

    float gate_open_angle;
    float gate_target_angle;

    float animation_time;
};

float deg_to_rad(float deg);
void trim_line(char* s);
int str_ieq(const char* a, const char* b);
int is_interactable_type(const char* type);

int find_or_add_model(Scene* sc, const char* path);
int find_or_add_texture(Scene* sc, const char* path);

int is_gate_object(const SceneEntity* o);
int gate_hinge_sign(const SceneEntity* o);
void compute_gate_aabb(SceneEntity* o);
void compute_object_aabb(SceneEntity* o);
void add_object(Scene* sc, SceneEntity obj);

bool parse_csv_line_v2(
    const char* line,
    char* out_id, char* out_type,
    char* out_model_path, char* out_tex_path,
    float* px, float* py, float* pz,
    float* rx, float* ry, float* rz,
    float* sx, float* sy, float* sz
);

int extract_trailing_number(const char* id);
SceneEntity* find_switch_for_lamp(Scene* sc, const SceneEntity* lamp);
int is_lamp_active(Scene* sc, const SceneEntity* lamp);
float lamp_pulse_value(Scene* sc, const SceneEntity* lamp);
void get_object_forward_from_ry(float ry_deg, float* fx, float* fz);

float broken_switch_pulse(Scene* sc, const SceneEntity* sw);
int count_active_switches(Scene* sc);
void scene_recompute_power(Scene* sc);
void update_gate_transforms(Scene* sc);

bool ray_aabb_hit(
    const float ox, const float oy, const float oz,
    const float dx, const float dy, const float dz,
    const AABB* b, float* out_t
);

void draw_ground_plane(TextureEntry* t, const SceneEntity* o);
int is_fence_like_object(const SceneEntity* o, const TextureEntry* t);
float compute_fence_brightness(float master_light);

void draw_wall_lamp_glow(const SceneEntity* o, float pulse, int active);
void draw_switch_status_light(int repaired, float pulse);
void draw_switch_sparks(Scene* sc, const SceneEntity* sw);

void apply_entity_transform(const SceneEntity* o);
void draw_scene_entity(Scene* sc, SceneEntity* o, float master_light);

#endif