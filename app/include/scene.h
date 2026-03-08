#ifndef SCENE_H
#define SCENE_H

#include <stdbool.h>

typedef struct Scene Scene;

bool scene_init(Scene** out_scene, const char* csv_path);
void scene_draw(Scene* scene);
void scene_destroy(Scene* scene);

#endif