#include "scene.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <SDL2/SDL_opengl.h>

#include "texture.h"
#include "model.h"
#include "load.h"
#include "draw.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

typedef struct SceneObject {
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
} SceneObject;

struct Scene {
    ModelEntry* models;
    int model_count;
    int model_cap;

    TextureEntry* textures;
    int tex_count;
    int tex_cap;

    SceneObject* objects;
    int obj_count;
    int obj_cap;

    int power_on;

    float gate_open_amount;
    float gate_target_amount;

    float animation_time;
};

static float deg_to_rad(float deg) {
    return deg * (float)M_PI / 180.0f;
}

static void trim_line(char* s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r' || s[n - 1] == ' ' || s[n - 1] == '\t')) {
        s[--n] = '\0';
    }
}

static int str_ieq(const char* a, const char* b) {
    while (*a && *b) {
        char ca = *a;
        char cb = *b;

        if (ca >= 'A' && ca <= 'Z') ca = (char)(ca - 'A' + 'a');
        if (cb >= 'A' && cb <= 'Z') cb = (char)(cb - 'A' + 'a');

        if (ca != cb) {
            return 0;
        }

        a++;
        b++;
    }

    return (*a == '\0' && *b == '\0');
}

static int is_interactable_type(const char* type) {
    return str_ieq(type, "switch");
}

static int find_or_add_model(Scene* sc, const char* path) {
    for (int i = 0; i < sc->model_count; i++) {
        if (strcmp(sc->models[i].path, path) == 0) {
            return i;
        }
    }

    if (sc->model_count == sc->model_cap) {
        sc->model_cap = (sc->model_cap == 0) ? 8 : sc->model_cap * 2;
        sc->models = (ModelEntry*)realloc(sc->models, sizeof(ModelEntry) * sc->model_cap);
    }

    ModelEntry* e = &sc->models[sc->model_count];
    memset(e, 0, sizeof(*e));
    strncpy(e->path, path, sizeof(e->path) - 1);
    init_model(&e->model);
    e->loaded = false;

    return sc->model_count++;
}

static int find_or_add_texture(Scene* sc, const char* path) {
    for (int i = 0; i < sc->tex_count; i++) {
        if (strcmp(sc->textures[i].path, path) == 0) {
            return i;
        }
    }

    if (sc->tex_count == sc->tex_cap) {
        sc->tex_cap = (sc->tex_cap == 0) ? 8 : sc->tex_cap * 2;
        sc->textures = (TextureEntry*)realloc(sc->textures, sizeof(TextureEntry) * sc->tex_cap);
    }

    TextureEntry* t = &sc->textures[sc->tex_count];
    memset(t, 0, sizeof(*t));
    strncpy(t->path, path, sizeof(t->path) - 1);
    t->tex.id = 0;
    t->tex.width = 0;
    t->tex.height = 0;
    t->loaded = false;

    return sc->tex_count++;
}

static void compute_object_aabb(SceneObject* o) {
    float hx = 0.5f * o->sx;
    float hy = 0.5f * o->sy;
    float hz = 0.5f * o->sz;

    o->box.min_x = o->px - hx;
    o->box.max_x = o->px + hx;
    o->box.min_y = o->py - hy;
    o->box.max_y = o->py + hy;
    o->box.min_z = o->pz - hz;
    o->box.max_z = o->pz + hz;
}

static void add_object(Scene* sc, SceneObject obj) {
    compute_object_aabb(&obj);

    if (sc->obj_count == sc->obj_cap) {
        sc->obj_cap = (sc->obj_cap == 0) ? 16 : sc->obj_cap * 2;
        sc->objects = (SceneObject*)realloc(sc->objects, sizeof(SceneObject) * sc->obj_cap);
    }

    sc->objects[sc->obj_count++] = obj;
}

static bool parse_csv_line_v2(
    const char* line,
    char* out_id, char* out_type,
    char* out_model_path, char* out_tex_path,
    float* px, float* py, float* pz,
    float* rx, float* ry, float* rz,
    float* sx, float* sy, float* sz
) {
    return (sscanf(
        line,
        " %63[^,] , %31[^,] , %259[^,] , %259[^,] , %f , %f , %f , %f , %f , %f , %f , %f , %f ",
        out_id, out_type, out_model_path, out_tex_path,
        px, py, pz,
        rx, ry, rz,
        sx, sy, sz
    ) == 13);
}

static int extract_trailing_number(const char* id) {
    int len = (int)strlen(id);
    int start = len;

    while (start > 0 && id[start - 1] >= '0' && id[start - 1] <= '9') {
        start--;
    }

    if (start == len) {
        return -1;
    }

    return atoi(id + start);
}

static SceneObject* find_switch_for_lamp(Scene* sc, const SceneObject* lamp) {
    int lamp_num = extract_trailing_number(lamp->id);
    if (lamp_num < 0) {
        return NULL;
    }

    for (int i = 0; i < sc->obj_count; i++) {
        SceneObject* obj = &sc->objects[i];

        if (!str_ieq(obj->type, "switch")) {
            continue;
        }

        if (extract_trailing_number(obj->id) == lamp_num) {
            return obj;
        }
    }

    return NULL;
}

static int is_lamp_active(Scene* sc, const SceneObject* lamp) {
    SceneObject* linked_switch = find_switch_for_lamp(sc, lamp);
    if (!linked_switch) {
        return 0;
    }

    return (linked_switch->state == 1);
}

static float lamp_pulse_value(Scene* sc, const SceneObject* lamp) {
    int n = extract_trailing_number(lamp->id);
    float phase = (n > 0) ? (float)n * 1.35f : 0.0f;
    return 0.55f + 0.45f * (0.5f + 0.5f * sinf(sc->animation_time * 4.0f + phase));
}

static int count_active_switches(Scene* sc) {
    int count = 0;

    for (int i = 0; i < sc->obj_count; i++) {
        if (str_ieq(sc->objects[i].type, "switch") && sc->objects[i].state == 1) {
            count++;
        }
    }

    return count;
}

static void scene_recompute_power(Scene* sc) {
    int active_switches = count_active_switches(sc);
    sc->power_on = (active_switches > 0) ? 1 : 0;
    sc->gate_target_amount = (active_switches >= 3) ? 2.2f : 0.0f;
}

static void update_gate_positions(Scene* sc) {
    for (int i = 0; i < sc->obj_count; i++) {
        SceneObject* o = &sc->objects[i];

        if (!str_ieq(o->type, "gate")) {
            continue;
        }

        o->px = o->base_px;
        o->py = o->base_py;
        o->pz = o->base_pz;

        if (strcmp(o->id, "gate_left") == 0) {
            o->px = o->base_px - sc->gate_open_amount;
        }
        else if (strcmp(o->id, "gate_right") == 0) {
            o->px = o->base_px + sc->gate_open_amount;
        }

        compute_object_aabb(o);
    }
}

static bool ray_aabb_hit(
    const float ox, const float oy, const float oz,
    const float dx, const float dy, const float dz,
    const AABB* b, float* out_t
) {
    const float EPS = 1e-6f;

    float tmin = -1e30f;
    float tmax =  1e30f;

    if (fabsf(dx) < EPS) {
        if (ox < b->min_x || ox > b->max_x) return false;
    } else {
        float inv = 1.0f / dx;
        float t1 = (b->min_x - ox) * inv;
        float t2 = (b->max_x - ox) * inv;
        if (t1 > t2) {
            float tmp = t1;
            t1 = t2;
            t2 = tmp;
        }
        if (t1 > tmin) tmin = t1;
        if (t2 < tmax) tmax = t2;
        if (tmin > tmax) return false;
    }

    if (fabsf(dy) < EPS) {
        if (oy < b->min_y || oy > b->max_y) return false;
    } else {
        float inv = 1.0f / dy;
        float t1 = (b->min_y - oy) * inv;
        float t2 = (b->max_y - oy) * inv;
        if (t1 > t2) {
            float tmp = t1;
            t1 = t2;
            t2 = tmp;
        }
        if (t1 > tmin) tmin = t1;
        if (t2 < tmax) tmax = t2;
        if (tmin > tmax) return false;
    }

    if (fabsf(dz) < EPS) {
        if (oz < b->min_z || oz > b->max_z) return false;
    } else {
        float inv = 1.0f / dz;
        float t1 = (b->min_z - oz) * inv;
        float t2 = (b->max_z - oz) * inv;
        if (t1 > t2) {
            float tmp = t1;
            t1 = t2;
            t2 = tmp;
        }
        if (t1 > tmin) tmin = t1;
        if (t2 < tmax) tmax = t2;
        if (tmin > tmax) return false;
    }

    {
        float t = (tmin >= 0.0f) ? tmin : tmax;
        if (t < 0.0f) return false;
        if (out_t) *out_t = t;
        return true;
    }
}

static void draw_ground_plane(TextureEntry* t, const SceneObject* o) {
    if (!t || !t->loaded || t->tex.id == 0) {
        return;
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, t->tex.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glColor3f(1.0f, 1.0f, 1.0f);

    {
        float half_x = 0.5f * o->sx;
        float half_z = 0.5f * o->sz;

        float x0 = o->px - half_x;
        float x1 = o->px + half_x;
        float z0 = o->pz - half_z;
        float z1 = o->pz + half_z;
        float y = o->py;

        float tile = 10.0f;
        float eps = 0.01f;
        float off = 0.37f;

        glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(off + eps,        off + eps);        glVertex3f(x0, y, z0);
        glTexCoord2f(off + tile - eps, off + eps);        glVertex3f(x1, y, z0);
        glTexCoord2f(off + tile - eps, off + tile - eps); glVertex3f(x1, y, z1);
        glTexCoord2f(off + eps,        off + tile - eps); glVertex3f(x0, y, z1);
        glEnd();
    }

    glDisable(GL_TEXTURE_2D);
}

static int is_fence_like_object(const SceneObject* o, const TextureEntry* t) {
    if (o != NULL) {
        if (strncmp(o->id, "fence_", 6) == 0) {
            return 1;
        }
        if (strstr(o->id, "fence") != NULL) {
            return 1;
        }
    }

    if (t != NULL) {
        if (strstr(t->path, "fence.bmp") != NULL) {
            return 1;
        }
    }

    return 0;
}

static float compute_fence_brightness(float master_light) {
    if (master_light < 0.2f) {
        master_light = 0.2f;
    }
    if (master_light > 2.0f) {
        master_light = 2.0f;
    }

    /* 0.2 -> 0.12, 1.0 -> 0.55, 2.0 -> 1.00 */
    return 0.022222f + 0.488889f * master_light;
}

bool scene_init(Scene** out_scene, const char* csv_path) {
    if (!out_scene) {
        return false;
    }

    FILE* f = fopen(csv_path, "r");
    if (!f) {
        printf("scene_init: failed to open CSV: %s\n", csv_path);
        return false;
    }

    Scene* sc = (Scene*)calloc(1, sizeof(Scene));
    if (!sc) {
        fclose(f);
        return false;
    }

    sc->gate_open_amount = 0.0f;
    sc->gate_target_amount = 0.0f;
    sc->animation_time = 0.0f;

    {
        char line[1024];
        int line_no = 0;

        while (fgets(line, sizeof(line), f)) {
            line_no++;
            trim_line(line);

            if (line[0] == '\0') continue;
            if (line[0] == '#') continue;

            if (line_no == 1 && strstr(line, "id") && strstr(line, "type") && strstr(line, "model")) {
                continue;
            }

            {
                char id[64], type[32], model_path[260], tex_path[260];
                float px, py, pz, rx, ry, rz, sx, sy, sz;

                if (!parse_csv_line_v2(line, id, type, model_path, tex_path, &px, &py, &pz, &rx, &ry, &rz, &sx, &sy, &sz)) {
                    printf("scene_init: CSV parse error at line %d: %s\n", line_no, line);
                    continue;
                }

                {
                    int midx = find_or_add_model(sc, model_path);
                    int tidx = find_or_add_texture(sc, tex_path);

                    SceneObject obj;
                    memset(&obj, 0, sizeof(obj));

                    strncpy(obj.id, id, sizeof(obj.id) - 1);
                    strncpy(obj.type, type, sizeof(obj.type) - 1);

                    obj.model_idx = midx;
                    obj.tex_idx = tidx;

                    obj.px = px;
                    obj.py = py;
                    obj.pz = pz;

                    obj.rx = rx;
                    obj.ry = ry;
                    obj.rz = rz;

                    obj.sx = sx;
                    obj.sy = sy;
                    obj.sz = sz;

                    obj.base_px = px;
                    obj.base_py = py;
                    obj.base_pz = pz;

                    obj.state = 0;

                    add_object(sc, obj);
                }
            }
        }
    }

    fclose(f);

    for (int i = 0; i < sc->model_count; i++) {
        if (load_model(&sc->models[i].model, sc->models[i].path) != TRUE) {
            printf("scene_init: failed to load model: %s\n", sc->models[i].path);
        } else {
            sc->models[i].loaded = true;
        }
    }

    for (int i = 0; i < sc->tex_count; i++) {
        if (!load_texture_bmp(&sc->textures[i].tex, sc->textures[i].path, false, 0, 0, 0)) {
            printf("scene_init: failed to load texture: %s\n", sc->textures[i].path);
        } else {
            sc->textures[i].loaded = true;
        }
    }

    scene_recompute_power(sc);
    update_gate_positions(sc);

    *out_scene = sc;

    printf(
        "scene_init: objects=%d models=%d textures=%d power_on=%d\n",
        sc->obj_count, sc->model_count, sc->tex_count, sc->power_on
    );

    return true;
}

void scene_update(Scene* sc, double delta_time) {
    if (!sc) {
        return;
    }

    sc->animation_time += (float)delta_time;

    {
        float speed = 2.4f * (float)delta_time;

        if (sc->gate_open_amount < sc->gate_target_amount) {
            sc->gate_open_amount += speed;
            if (sc->gate_open_amount > sc->gate_target_amount) {
                sc->gate_open_amount = sc->gate_target_amount;
            }
        }
        else if (sc->gate_open_amount > sc->gate_target_amount) {
            sc->gate_open_amount -= speed;
            if (sc->gate_open_amount < sc->gate_target_amount) {
                sc->gate_open_amount = sc->gate_target_amount;
            }
        }
    }

    update_gate_positions(sc);
}

bool scene_collides(Scene* sc, const AABB* player_box) {
    if (!sc || !player_box) {
        return false;
    }

    for (int i = 0; i < sc->obj_count; i++) {
        if (str_ieq(sc->objects[i].type, "lamp")) continue;
        if (str_ieq(sc->objects[i].type, "ground")) continue;

        if (aabb_intersects(&sc->objects[i].box, player_box)) {
            return true;
        }
    }

    return false;
}

int scene_pick(Scene* sc, const Camera* cam) {
    if (!sc || !cam) {
        return -1;
    }

    {
        float ox = cam->x;
        float oy = cam->y;
        float oz = cam->z;

        float yaw = deg_to_rad(cam->yaw);
        float pitch = deg_to_rad(cam->pitch);

        float dx = cosf(pitch) * cosf(yaw);
        float dy = sinf(pitch);
        float dz = cosf(pitch) * sinf(yaw);

        int best_i = -1;
        float best_t = 1e30f;

        for (int i = 0; i < sc->obj_count; i++) {
            if (!is_interactable_type(sc->objects[i].type)) {
                continue;
            }

            {
                float t = 0.0f;
                if (ray_aabb_hit(ox, oy, oz, dx, dy, dz, &sc->objects[i].box, &t)) {
                    if (t < best_t) {
                        best_t = t;
                        best_i = i;
                    }
                }
            }
        }

        if (best_i != -1 && best_t > 8.0f) {
            return -1;
        }

        return best_i;
    }
}

void scene_interact(Scene* sc, int picked_index) {
    if (!sc) return;
    if (picked_index < 0 || picked_index >= sc->obj_count) return;

    {
        SceneObject* o = &sc->objects[picked_index];

        if (str_ieq(o->type, "switch")) {
            o->state = (o->state == 0) ? 1 : 0;
            scene_recompute_power(sc);

            printf(
                "INTERACT: %s toggled -> %d | active_switches=%d | gate_target=%.2f\n",
                o->id,
                o->state,
                count_active_switches(sc),
                sc->gate_target_amount
            );
        }
    }
}

int scene_get_dynamic_light_count(Scene* sc) {
    int count = 0;

    if (!sc) {
        return 0;
    }

    for (int i = 0; i < sc->obj_count; i++) {
        SceneObject* o = &sc->objects[i];

        if (!str_ieq(o->type, "lamp")) continue;
        if (is_lamp_active(sc, o)) count++;
    }

    return count;
}

bool scene_get_dynamic_light(
    Scene* sc,
    int active_index,
    float* x, float* y, float* z,
    float* intensity
) {
    int current = 0;

    if (!sc) {
        return false;
    }

    for (int i = 0; i < sc->obj_count; i++) {
        SceneObject* o = &sc->objects[i];

        if (!str_ieq(o->type, "lamp")) continue;
        if (!is_lamp_active(sc, o)) continue;

        if (current == active_index) {
            if (x) *x = o->px;
            if (y) *y = o->py + 0.15f;
            if (z) *z = o->pz;
            if (intensity) *intensity = lamp_pulse_value(sc, o);
            return true;
        }

        current++;
    }

    return false;
}

static void draw_aabb_lines(const AABB* b) {
    float x0 = b->min_x, x1 = b->max_x;
    float y0 = b->min_y, y1 = b->max_y;
    float z0 = b->min_z, z1 = b->max_z;

    glBegin(GL_LINES);

    glVertex3f(x0, y0, z0); glVertex3f(x1, y0, z0);
    glVertex3f(x1, y0, z0); glVertex3f(x1, y0, z1);
    glVertex3f(x1, y0, z1); glVertex3f(x0, y0, z1);
    glVertex3f(x0, y0, z1); glVertex3f(x0, y0, z0);

    glVertex3f(x0, y1, z0); glVertex3f(x1, y1, z0);
    glVertex3f(x1, y1, z0); glVertex3f(x1, y1, z1);
    glVertex3f(x1, y1, z1); glVertex3f(x0, y1, z1);
    glVertex3f(x0, y1, z1); glVertex3f(x0, y1, z0);

    glVertex3f(x0, y0, z0); glVertex3f(x0, y1, z0);
    glVertex3f(x1, y0, z0); glVertex3f(x1, y1, z0);
    glVertex3f(x1, y0, z1); glVertex3f(x1, y1, z1);
    glVertex3f(x0, y0, z1); glVertex3f(x0, y1, z1);

    glEnd();
}

void scene_draw(Scene* sc, int picked_index, float master_light) {
    if (!sc) {
        return;
    }

    for (int i = 0; i < sc->obj_count; i++) {
        SceneObject* o = &sc->objects[i];
        ModelEntry* m = &sc->models[o->model_idx];
        TextureEntry* t = &sc->textures[o->tex_idx];

        if (str_ieq(o->type, "ground")) {
            draw_ground_plane(t, o);
            continue;
        }

        glPushMatrix();

        glTranslatef(o->px, o->py, o->pz);
        glRotatef(o->rx, 1.f, 0.f, 0.f);
        glRotatef(o->ry, 0.f, 1.f, 0.f);
        glRotatef(o->rz, 0.f, 0.f, 1.f);
        glScalef(o->sx, o->sy, o->sz);

        if (str_ieq(o->type, "lamp")) {
            if (is_lamp_active(sc, o)) {
                float pulse = lamp_pulse_value(sc, o);

                glDisable(GL_TEXTURE_2D);
                glDisable(GL_LIGHTING);
                glColor3f(1.00f * pulse, 0.92f * pulse, 0.55f * pulse);

                if (m->loaded) {
                    draw_model(&m->model);
                }

                glEnable(GL_LIGHTING);
            } else {
                glDisable(GL_TEXTURE_2D);
                glDisable(GL_LIGHTING);
                glColor3f(0.15f, 0.15f, 0.18f);

                if (m->loaded) {
                    draw_model(&m->model);
                }

                glEnable(GL_LIGHTING);
            }
        }
        else if (str_ieq(o->type, "switch")) {
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);

            if (o->state == 1) {
                glColor3f(0.20f, 1.00f, 0.20f);
            } else {
                glColor3f(0.85f, 0.15f, 0.15f);
            }

            if (m->loaded) {
                draw_model(&m->model);
            }

            glEnable(GL_LIGHTING);
        }
        else if (is_fence_like_object(o, t)) {
            float fence_brightness = compute_fence_brightness(master_light);

            glDisable(GL_LIGHTING);
            glColor3f(fence_brightness, fence_brightness, fence_brightness);

            if (t->loaded && t->tex.id != 0) {
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, t->tex.id);
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            }

            if (m->loaded) {
                draw_model(&m->model);
            }

            if (t->loaded && t->tex.id != 0) {
                glDisable(GL_TEXTURE_2D);
            }

            glColor3f(1.0f, 1.0f, 1.0f);
            glEnable(GL_LIGHTING);
        }
        else {
            glColor3f(1.0f, 1.0f, 1.0f);

            if (t->loaded && t->tex.id != 0) {
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, t->tex.id);
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            }

            if (m->loaded) {
                draw_model(&m->model);
            }

            if (t->loaded && t->tex.id != 0) {
                glDisable(GL_TEXTURE_2D);
            }
        }

        glPopMatrix();

        if (i == picked_index && is_interactable_type(o->type)) {
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            glColor3f(1.f, 1.f, 1.f);
            draw_aabb_lines(&o->box);
            glEnable(GL_LIGHTING);
        }
    }
}

void scene_destroy(Scene* sc) {
    if (!sc) {
        return;
    }

    for (int i = 0; i < sc->model_count; i++) {
        free_model(&sc->models[i].model);
    }

    for (int i = 0; i < sc->tex_count; i++) {
        destroy_texture(&sc->textures[i].tex);
    }

    free(sc->models);
    free(sc->textures);
    free(sc->objects);
    free(sc);
}