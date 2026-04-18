#ifndef APP_RENDER_H
#define APP_RENDER_H

#include "scene.h"

void setup_projection(int window_w, int window_h);
void apply_lighting(float intensity);
void apply_dynamic_lights(Scene* scene, float master_intensity);
void draw_grid(void);
void setup_fog(float density);

#endif