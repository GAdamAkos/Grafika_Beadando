#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>

#include "app_render.h"

void setup_projection(int window_w, int window_h)
{
    if (window_h <= 0) {
        window_h = 1;
    }

    glViewport(0, 0, window_w, window_h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    {
        float aspect = (float)window_w / (float)window_h;
        gluPerspective(60.0, aspect, 0.5, 1000.0);
    }

    glMatrixMode(GL_MODELVIEW);
}

void apply_lighting(float intensity)
{
    if (intensity < 0.2f) {
        intensity = 0.2f;
    }
    if (intensity > 2.0f) {
        intensity = 2.0f;
    }

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    {
        GLfloat material_specular[] = {
            0.22f * intensity,
            0.22f * intensity,
            0.22f * intensity,
            1.0f
        };
        GLfloat material_shininess[] = { 24.0f };

        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_specular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, material_shininess);
    }

    {
        GLfloat ambient[] = {
            0.10f * intensity,
            0.10f * intensity,
            0.12f * intensity,
            1.0f
        };

        GLfloat diffuse[] = {
            0.42f * intensity,
            0.42f * intensity,
            0.40f * intensity,
            1.0f
        };

        GLfloat specular[] = {
            0.18f * intensity,
            0.18f * intensity,
            0.18f * intensity,
            1.0f
        };

        glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    }
}

void apply_dynamic_lights(Scene* scene, float master_intensity)
{
    GLenum light_ids[3] = { GL_LIGHT1, GL_LIGHT2, GL_LIGHT3 };

    for (int i = 0; i < 3; i++) {
        glDisable(light_ids[i]);
    }

    if (!scene) {
        return;
    }

    {
        int active_count = scene_get_dynamic_light_count(scene);
        if (active_count > 3) {
            active_count = 3;
        }

        for (int i = 0; i < active_count; i++) {
            float x, y, z, pulse;

            if (!scene_get_dynamic_light(scene, i, &x, &y, &z, &pulse)) {
                continue;
            }

            {
                float strength = pulse;

                GLfloat position[] = { x, y, z, 1.0f };

                GLfloat ambient[] = {
                    0.03f * master_intensity * strength,
                    0.025f * master_intensity * strength,
                    0.015f * master_intensity * strength,
                    1.0f
                };

                GLfloat diffuse[] = {
                    1.35f * master_intensity * strength,
                    1.05f * master_intensity * strength,
                    0.45f * master_intensity * strength,
                    1.0f
                };

                GLfloat specular[] = {
                    0.55f * master_intensity * strength,
                    0.40f * master_intensity * strength,
                    0.18f * master_intensity * strength,
                    1.0f
                };

                glEnable(light_ids[i]);
                glLightfv(light_ids[i], GL_POSITION, position);
                glLightfv(light_ids[i], GL_AMBIENT, ambient);
                glLightfv(light_ids[i], GL_DIFFUSE, diffuse);
                glLightfv(light_ids[i], GL_SPECULAR, specular);

                glLightf(light_ids[i], GL_CONSTANT_ATTENUATION, 0.55f);
                glLightf(light_ids[i], GL_LINEAR_ATTENUATION, 0.08f);
                glLightf(light_ids[i], GL_QUADRATIC_ATTENUATION, 0.015f);
            }
        }
    }
}

void draw_grid(void)
{
    glDisable(GL_LIGHTING);

    glBegin(GL_LINES);
    for (int i = -40; i <= 40; i++) {
        glColor3f(0.40f, 0.40f, 0.40f);
        glVertex3f((float)i, 0.0f, -40.0f);
        glVertex3f((float)i, 0.0f, 40.0f);

        glVertex3f(-40.0f, 0.0f, (float)i);
        glVertex3f(40.0f, 0.0f, (float)i);
    }
    glEnd();

    glEnable(GL_LIGHTING);
}

void setup_fog(float density)
{
    if (density < 0.0f) {
        density = 0.0f;
    }

    glEnable(GL_FOG);

    {
        GLfloat fog_color[4] = { 0.08f, 0.08f, 0.12f, 1.0f };
        glFogfv(GL_FOG_COLOR, fog_color);
    }

    glFogi(GL_FOG_MODE, GL_EXP2);
    glFogf(GL_FOG_DENSITY, density);

    glHint(GL_FOG_HINT, GL_NICEST);
}