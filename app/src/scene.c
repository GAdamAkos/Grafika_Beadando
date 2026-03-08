#include "scene.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <SDL2/SDL_opengl.h>

#include "texture.h"

// Course OBJ loader
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
};

static float deg_to_rad(float deg) { return deg * (float)M_PI / 180.0f; }

static void trim_line(char* s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r' || s[n-1] == ' ' || s[n-1] == '\t'))
        s[--n] = '\0';
}

static int str_ieq(const char* a, const char* b) {
    while (*a && *b) {
        char ca = *a, cb = *b;
        if (ca >= 'A' && ca <= 'Z') ca = (char)(ca - 'A' + 'a');
        if (cb >= 'A' && cb <= 'Z') cb = (char)(cb - 'A' + 'a');
        if (ca != cb) return 0;
        a++; b++;
    }
    return (*a == '\0' && *b == '\0');
}

static int is_interactable_type(const char* type) {
    return str_ieq(type, "switch");
}

static int find_or_add_model(Scene* sc, const char* path) {
    for (int i = 0; i < sc->model_count; i++) {
        if (strcmp(sc->models[i].path, path) == 0) return i;
    }
    if (sc->model_count == sc->model_cap) {
        sc->model_cap = (sc->model_cap == 0) ? 8 : sc->model_cap * 2;
        sc->models = (ModelEntry*)realloc(sc->models, sizeof(ModelEntry) * sc->model_cap);
    }
    ModelEntry* e = &sc->models[sc->model_count];
    memset(e, 0, sizeof(*e));
    strncpy(e->path, path, sizeof(e->path)-1);
    init_model(&e->model);
    e->loaded = false;
    return sc->model_count++;
}

static int find_or_add_texture(Scene* sc, const char* path) {
    for (int i = 0; i < sc->tex_count; i++) {
        if (strcmp(sc->textures[i].path, path) == 0) return i;
    }
    if (sc->tex_count == sc->tex_cap) {
        sc->tex_cap = (sc->tex_cap == 0) ? 8 : sc->tex_cap * 2;
        sc->textures = (TextureEntry*)realloc(sc->textures, sizeof(TextureEntry) * sc->tex_cap);
    }
    TextureEntry* t = &sc->textures[sc->tex_count];
    memset(t, 0, sizeof(*t));
    strncpy(t->path, path, sizeof(t->path)-1);
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
    float* px,float* py,float* pz,
    float* rx,float* ry,float* rz,
    float* sx,float* sy,float* sz
) {
    return (sscanf(line,
        " %63[^,] , %31[^,] , %259[^,] , %259[^,] , %f , %f , %f , %f , %f , %f , %f , %f , %f ",
        out_id, out_type, out_model_path, out_tex_path,
        px, py, pz,
        rx, ry, rz,
        sx, sy, sz
    ) == 13);
}

static void scene_recompute_power(Scene* sc) {
    int on = 0;
    for (int i = 0; i < sc->obj_count; i++) {
        if (str_ieq(sc->objects[i].type, "switch") && sc->objects[i].state == 1) {
            on = 1;
            break;
        }
    }
    sc->power_on = on;
}

/* -------- Ray vs AABB (slabs) -------- */

static bool ray_aabb_hit(const float ox, const float oy, const float oz,
                         const float dx, const float dy, const float dz,
                         const AABB* b, float* out_t) {
    const float EPS = 1e-6f;

    float tmin = -1e30f;
    float tmax =  1e30f;

    if (fabsf(dx) < EPS) {
        if (ox < b->min_x || ox > b->max_x) return false;
    } else {
        float inv = 1.0f / dx;
        float t1 = (b->min_x - ox) * inv;
        float t2 = (b->max_x - ox) * inv;
        if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
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
        if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
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
        if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
        if (t1 > tmin) tmin = t1;
        if (t2 < tmax) tmax = t2;
        if (tmin > tmax) return false;
    }

    float t = (tmin >= 0.0f) ? tmin : tmax;
    if (t < 0.0f) return false;

    if (out_t) *out_t = t;
    return true;
}

/* -------- Ground plane draw (tileable) -------- */

static void draw_ground_plane(TextureEntry* t, const SceneObject* o) {
    if (!t || !t->loaded || t->tex.id == 0) return;

    // Use repeat for ground
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, t->tex.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    // Ground extents from scale (sx,sz)
    float half_x = 0.5f * o->sx;
    float half_z = 0.5f * o->sz;


    float x0 = o->px - half_x;
    float x1 = o->px + half_x;
    float z0 = o->pz - half_z;
    float z1 = o->pz + half_z;
    float y  = o->py;

    float tile = 10.0f;     // több ismétlés -> kevésbé pixeles, szebb
    float eps  = 0.01f;     // nagyobb bias -> kevésbé mintázza a széleket
    float off  = 0.37f;     // offset -> ne pont a "szél" essen a varratra

    glBegin(GL_QUADS);
    glTexCoord2f(off + eps,          off + eps);          glVertex3f(x0, y, z0);
    glTexCoord2f(off + tile - eps,   off + eps);          glVertex3f(x1, y, z0);
    glTexCoord2f(off + tile - eps,   off + tile - eps);   glVertex3f(x1, y, z1);
    glTexCoord2f(off + eps,          off + tile - eps);   glVertex3f(x0, y, z1);
    glEnd();

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glDisable(GL_TEXTURE_2D);
}

/* -------- Public API -------- */

bool scene_init(Scene** out_scene, const char* csv_path) {
    if (!out_scene) return false;

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

        char id[64], type[32], model_path[260], tex_path[260];
        float px,py,pz,rx,ry,rz,sx,sy,sz;

        if (!parse_csv_line_v2(line, id, type, model_path, tex_path, &px,&py,&pz,&rx,&ry,&rz,&sx,&sy,&sz)) {
            printf("scene_init: CSV parse error at line %d: %s\n", line_no, line);
            continue;
        }

        int midx = find_or_add_model(sc, model_path);
        int tidx = find_or_add_texture(sc, tex_path);

        SceneObject obj;
        memset(&obj, 0, sizeof(obj));

        strncpy(obj.id, id, sizeof(obj.id)-1);
        strncpy(obj.type, type, sizeof(obj.type)-1);

        obj.model_idx = midx;
        obj.tex_idx = tidx;

        obj.px = px; obj.py = py; obj.pz = pz;
        obj.rx = rx; obj.ry = ry; obj.rz = rz;
        obj.sx = sx; obj.sy = sy; obj.sz = sz;

        obj.state = 0;

        add_object(sc, obj);
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

    *out_scene = sc;
    printf("scene_init: objects=%d models=%d textures=%d power_on=%d\n",
           sc->obj_count, sc->model_count, sc->tex_count, sc->power_on);
    return true;
}

bool scene_collides(Scene* sc, const AABB* player_box) {
    if (!sc || !player_box) return false;

    for (int i = 0; i < sc->obj_count; i++) {
        if (str_ieq(sc->objects[i].type, "lamp")) continue;
        if (str_ieq(sc->objects[i].type, "ground")) continue;

        if (aabb_intersects(&sc->objects[i].box, player_box)) return true;
    }
    return false;
}

int scene_pick(Scene* sc, const Camera* cam) {
    if (!sc || !cam) return -1;

    float ox = cam->x, oy = cam->y, oz = cam->z;

    float yaw = deg_to_rad(cam->yaw);
    float pitch = deg_to_rad(cam->pitch);

    float dx = cosf(pitch) * cosf(yaw);
    float dy = sinf(pitch);
    float dz = cosf(pitch) * sinf(yaw);

    int best_i = -1;
    float best_t = 1e30f;

    for (int i = 0; i < sc->obj_count; i++) {
        if (!is_interactable_type(sc->objects[i].type)) continue;

        float t = 0.0f;
        if (ray_aabb_hit(ox, oy, oz, dx, dy, dz, &sc->objects[i].box, &t)) {
            if (t < best_t) { best_t = t; best_i = i; }
        }
    }

    if (best_i != -1 && best_t > 8.0f) return -1;
    return best_i;
}

void scene_interact(Scene* sc, int picked_index) {
    if (!sc) return;
    if (picked_index < 0 || picked_index >= sc->obj_count) return;

    SceneObject* o = &sc->objects[picked_index];

    if (str_ieq(o->type, "switch")) {
        o->state = (o->state == 0) ? 1 : 0;
        scene_recompute_power(sc);
        printf("INTERACT: %s toggled -> %d (power_on=%d)\n", o->id, o->state, sc->power_on);
    }
}

static void draw_aabb_lines(const AABB* b) {
    float x0 = b->min_x, x1 = b->max_x;
    float y0 = b->min_y, y1 = b->max_y;
    float z0 = b->min_z, z1 = b->max_z;

    glBegin(GL_LINES);

    glVertex3f(x0,y0,z0); glVertex3f(x1,y0,z0);
    glVertex3f(x1,y0,z0); glVertex3f(x1,y0,z1);
    glVertex3f(x1,y0,z1); glVertex3f(x0,y0,z1);
    glVertex3f(x0,y0,z1); glVertex3f(x0,y0,z0);

    glVertex3f(x0,y1,z0); glVertex3f(x1,y1,z0);
    glVertex3f(x1,y1,z0); glVertex3f(x1,y1,z1);
    glVertex3f(x1,y1,z1); glVertex3f(x0,y1,z1);
    glVertex3f(x0,y1,z1); glVertex3f(x0,y1,z0);

    glVertex3f(x0,y0,z0); glVertex3f(x0,y1,z0);
    glVertex3f(x1,y0,z0); glVertex3f(x1,y1,z0);
    glVertex3f(x1,y0,z1); glVertex3f(x1,y1,z1);
    glVertex3f(x0,y0,z1); glVertex3f(x0,y1,z1);

    glEnd();
}

void scene_draw(Scene* sc, int picked_index) {
    if (!sc) return;

    for (int i = 0; i < sc->obj_count; i++) {
        SceneObject* o = &sc->objects[i];
        ModelEntry* m = &sc->models[o->model_idx];
        TextureEntry* t = &sc->textures[o->tex_idx];

        // Special: ground is a real plane with tiling UVs
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

        if (str_ieq(o->type, "lamp") && sc->power_on) {
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            glColor3f(1.0f, 0.95f, 0.60f);
            if (m->loaded) draw_model(&m->model);
            glEnable(GL_LIGHTING);
        } else {
            if (t->loaded && t->tex.id != 0) {
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, t->tex.id);
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            }

            if (m->loaded) draw_model(&m->model);

            if (t->loaded && t->tex.id != 0) {
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
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
    if (!sc) return;

    for (int i = 0; i < sc->model_count; i++) free_model(&sc->models[i].model);
    for (int i = 0; i < sc->tex_count; i++) destroy_texture(&sc->textures[i].tex);

    free(sc->models);
    free(sc->textures);
    free(sc->objects);
    free(sc);
}