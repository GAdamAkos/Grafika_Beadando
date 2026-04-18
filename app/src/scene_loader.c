#include "scene_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "load.h"

void trim_line(char* s)
{
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r' || s[n - 1] == ' ' || s[n - 1] == '\t')) {
        s[--n] = '\0';
    }
}

int str_ieq(const char* a, const char* b)
{
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

int is_interactable_type(const char* type)
{
    return str_ieq(type, "switch") || str_ieq(type, "terminal");
}

int find_or_add_model(Scene* sc, const char* path)
{
    for (int i = 0; i < sc->model_count; i++) {
        if (strcmp(sc->models[i].path, path) == 0) {
            return i;
        }
    }

    if (sc->model_count == sc->model_cap) {
        sc->model_cap = (sc->model_cap == 0) ? 8 : sc->model_cap * 2;
        sc->models = (ModelEntry*)realloc(sc->models, sizeof(ModelEntry) * sc->model_cap);
    }

    {
        ModelEntry* e = &sc->models[sc->model_count];
        memset(e, 0, sizeof(*e));
        strncpy(e->path, path, sizeof(e->path) - 1);
        init_model(&e->model);
        e->loaded = false;
    }

    return sc->model_count++;
}

int find_or_add_texture(Scene* sc, const char* path)
{
    for (int i = 0; i < sc->tex_count; i++) {
        if (strcmp(sc->textures[i].path, path) == 0) {
            return i;
        }
    }

    if (sc->tex_count == sc->tex_cap) {
        sc->tex_cap = (sc->tex_cap == 0) ? 8 : sc->tex_cap * 2;
        sc->textures = (TextureEntry*)realloc(sc->textures, sizeof(TextureEntry) * sc->tex_cap);
    }

    {
        TextureEntry* t = &sc->textures[sc->tex_count];
        memset(t, 0, sizeof(*t));
        strncpy(t->path, path, sizeof(t->path) - 1);
        t->tex.id = 0;
        t->tex.width = 0;
        t->tex.height = 0;
        t->loaded = false;
    }

    return sc->tex_count++;
}

void add_object(Scene* sc, SceneEntity obj)
{
    compute_object_aabb(&obj);

    if (sc->obj_count == sc->obj_cap) {
        sc->obj_cap = (sc->obj_cap == 0) ? 16 : sc->obj_cap * 2;
        sc->objects = (SceneEntity*)realloc(sc->objects, sizeof(SceneEntity) * sc->obj_cap);
    }

    sc->objects[sc->obj_count++] = obj;
}

bool parse_csv_line_v2(
    const char* line,
    char* out_id, char* out_type,
    char* out_model_path, char* out_tex_path,
    float* px, float* py, float* pz,
    float* rx, float* ry, float* rz,
    float* sx, float* sy, float* sz
)
{
    return (sscanf(
        line,
        " %63[^,] , %31[^,] , %259[^,] , %259[^,] , %f , %f , %f , %f , %f , %f , %f , %f , %f ",
        out_id, out_type, out_model_path, out_tex_path,
        px, py, pz,
        rx, ry, rz,
        sx, sy, sz
    ) == 13);
}

bool scene_init(Scene** out_scene, const char* csv_path)
{
    if (!out_scene) {
        return false;
    }

    {
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

        sc->gate_open_angle = 0.0f;
        sc->gate_target_angle = 0.0f;
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

                        SceneEntity obj;
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
        update_gate_transforms(sc);

        *out_scene = sc;

        printf(
            "scene_init: objects=%d models=%d textures=%d power_on=%d\n",
            sc->obj_count, sc->model_count, sc->tex_count, sc->power_on
        );
    }

    return true;
}