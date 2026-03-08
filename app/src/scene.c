#include "scene.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL_opengl.h>

#include "texture.h"

// Course OBJ loader
#include "model.h"
#include "load.h"
#include "draw.h"

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
    int model_idx;
    int tex_idx;

    float px, py, pz;
    float rx, ry, rz;
    float sx, sy, sz;
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
};

static void trim_line(char* s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r' || s[n-1] == ' ' || s[n-1] == '\t'))
        s[--n] = '\0';
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

static void add_object(Scene* sc, SceneObject obj) {
    if (sc->obj_count == sc->obj_cap) {
        sc->obj_cap = (sc->obj_cap == 0) ? 16 : sc->obj_cap * 2;
        sc->objects = (SceneObject*)realloc(sc->objects, sizeof(SceneObject) * sc->obj_cap);
    }
    sc->objects[sc->obj_count++] = obj;
}

static bool parse_csv_line(const char* line, char* model_path, char* tex_path,
                           float* px,float* py,float* pz,
                           float* rx,float* ry,float* rz,
                           float* sx,float* sy,float* sz) {
    // CSV columns:
    // model,texture,px,py,pz,rx,ry,rz,sx,sy,sz
    // Example:
    // ../assets/models/cube.obj,../assets/textures/cube.bmp,0,0,0,0,0,0,1,1,1
    return (sscanf(line,
        " %259[^,] , %259[^,] , %f , %f , %f , %f , %f , %f , %f , %f , %f ",
        model_path, tex_path,
        px, py, pz,
        rx, ry, rz,
        sx, sy, sz
    ) == 11);
}

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

        // skip header if present
        if (strstr(line, "model") && strstr(line, "texture") && line_no == 1) {
            continue;
        }

        char model_path[260];
        char tex_path[260];
        float px,py,pz,rx,ry,rz,sx,sy,sz;

        if (!parse_csv_line(line, model_path, tex_path, &px,&py,&pz,&rx,&ry,&rz,&sx,&sy,&sz)) {
            printf("scene_init: CSV parse error at line %d: %s\n", line_no, line);
            continue;
        }

        int midx = find_or_add_model(sc, model_path);
        int tidx = find_or_add_texture(sc, tex_path);

        SceneObject obj;
        obj.model_idx = midx;
        obj.tex_idx = tidx;
        obj.px = px; obj.py = py; obj.pz = pz;
        obj.rx = rx; obj.ry = ry; obj.rz = rz;
        obj.sx = sx; obj.sy = sy; obj.sz = sz;

        add_object(sc, obj);
    }

    fclose(f);

    // Load unique models/textures once
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

    *out_scene = sc;
    printf("scene_init: objects=%d models=%d textures=%d\n", sc->obj_count, sc->model_count, sc->tex_count);
    return true;
}

void scene_draw(Scene* sc) {
    if (!sc) return;

    for (int i = 0; i < sc->obj_count; i++) {
        SceneObject* o = &sc->objects[i];
        ModelEntry* m = &sc->models[o->model_idx];
        TextureEntry* t = &sc->textures[o->tex_idx];

        glPushMatrix();

        glTranslatef(o->px, o->py, o->pz);
        glRotatef(o->rx, 1.f, 0.f, 0.f);
        glRotatef(o->ry, 0.f, 1.f, 0.f);
        glRotatef(o->rz, 0.f, 0.f, 1.f);
        glScalef(o->sx, o->sy, o->sz);

        if (t->loaded && t->tex.id != 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, t->tex.id);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // stable (no tint)
        }

        if (m->loaded) {
            draw_model(&m->model);
        }

        if (t->loaded && t->tex.id != 0) {
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            glDisable(GL_TEXTURE_2D);
        }

        glPopMatrix();
    }
}

void scene_destroy(Scene* sc) {
    if (!sc) return;

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