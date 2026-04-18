#include "scene_internal.h"

#include <SDL2/SDL_opengl.h>
#include <string.h>
#include <math.h>

void draw_wall_lamp_glow(const SceneEntity* o, float pulse, int active)
{
    float z = 0.515f;

    (void)o;

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);

    if (active) {
        glColor3f(1.00f * pulse, 0.92f * pulse, 0.55f * pulse);
    } else {
        glColor3f(0.12f, 0.12f, 0.14f);
    }

    glBegin(GL_QUADS);
    glVertex3f(-0.30f, -0.18f, z);
    glVertex3f( 0.30f, -0.18f, z);
    glVertex3f( 0.30f,  0.18f, z);
    glVertex3f(-0.30f,  0.18f, z);
    glEnd();

    if (active) {
        glColor3f(1.00f * pulse, 0.85f * pulse, 0.35f * pulse);

        glBegin(GL_QUADS);
        glVertex3f(-0.16f, -0.08f, z + 0.01f);
        glVertex3f( 0.16f, -0.08f, z + 0.01f);
        glVertex3f( 0.16f,  0.08f, z + 0.01f);
        glVertex3f(-0.16f,  0.08f, z + 0.01f);
        glEnd();
    }

    glEnable(GL_LIGHTING);
}

void draw_switch_status_light(int repaired, float pulse)
{
    float z_outer = 0.516f;
    float z_inner = 0.522f;

    float outer_r, outer_g, outer_b;
    float inner_r, inner_g, inner_b;

    if (repaired) {
        outer_r = 0.05f;
        outer_g = 0.30f + 0.10f * pulse;
        outer_b = 0.05f;

        inner_r = 0.25f;
        inner_g = 0.95f;
        inner_b = 0.20f;
    } else {
        outer_r = 0.20f + 0.10f * pulse;
        outer_g = 0.03f;
        outer_b = 0.02f;

        inner_r = 0.95f;
        inner_g = 0.18f + 0.08f * pulse;
        inner_b = 0.08f;
    }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    glColor4f(outer_r, outer_g, outer_b, 0.85f);
    glBegin(GL_QUADS);
    glVertex3f(-0.14f, 0.14f, z_outer);
    glVertex3f( 0.14f, 0.14f, z_outer);
    glVertex3f( 0.14f, 0.36f, z_outer);
    glVertex3f(-0.14f, 0.36f, z_outer);
    glEnd();

    glColor3f(inner_r, inner_g, inner_b);
    glBegin(GL_QUADS);
    glVertex3f(-0.09f, 0.18f, z_inner);
    glVertex3f( 0.09f, 0.18f, z_inner);
    glVertex3f( 0.09f, 0.32f, z_inner);
    glVertex3f(-0.09f, 0.32f, z_inner);
    glEnd();

    glEnable(GL_LIGHTING);
}

void draw_switch_sparks(Scene* sc, const SceneEntity* sw)
{
    float pulse = broken_switch_pulse(sc, sw);
    float t = sc->animation_time;
    float z1 = 0.525f;
    float z2 = 0.80f;

    if (pulse < 0.82f) {
        return;
    }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    glLineWidth(2.0f);
    glColor3f(1.00f, 0.65f + 0.20f * pulse, 0.10f);

    glBegin(GL_LINES);

    glVertex3f( 0.04f, -0.03f, z1); glVertex3f( 0.16f + 0.03f * sinf(t * 17.0f),  0.09f, z2);
    glVertex3f(-0.01f,  0.02f, z1); glVertex3f(-0.14f + 0.02f * cosf(t * 13.0f),  0.11f, z2 - 0.03f);
    glVertex3f( 0.02f, -0.01f, z1); glVertex3f( 0.08f, -0.13f + 0.02f * sinf(t * 19.0f), z2 - 0.02f);

    if (pulse > 0.90f) {
        glVertex3f( 0.06f,  0.01f, z1); glVertex3f( 0.21f, -0.03f, z2);
        glVertex3f(-0.04f,  0.00f, z1); glVertex3f(-0.18f,  0.15f, z2 - 0.01f);
    }

    glEnd();

    glLineWidth(1.0f);
    glColor3f(1.00f, 0.95f, 0.75f);

    glBegin(GL_LINES);
    glVertex3f( 0.04f, -0.03f, z1 + 0.002f); glVertex3f( 0.11f,  0.05f, z2 - 0.08f);
    glVertex3f(-0.01f,  0.02f, z1 + 0.002f); glVertex3f(-0.09f,  0.07f, z2 - 0.08f);
    glEnd();

    glEnable(GL_LIGHTING);
}

void draw_ground_plane(TextureEntry* t, const SceneEntity* o)
{
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

int is_fence_like_object(const SceneEntity* o, const TextureEntry* t)
{
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

float compute_fence_brightness(float master_light)
{
    if (master_light < 0.2f) {
        master_light = 0.2f;
    }
    if (master_light > 2.0f) {
        master_light = 2.0f;
    }

    return 0.022222f + 0.488889f * master_light;
}