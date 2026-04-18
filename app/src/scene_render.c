#include "scene_internal.h"

#include <string.h>
#include <SDL2/SDL_opengl.h>

#include "draw.h"

void apply_entity_transform(const SceneEntity* o)
{
    glTranslatef(o->px, o->py, o->pz);

    if (is_gate_object(o)) {
        float hinge_offset = 0.5f * o->sx * (float)gate_hinge_sign(o);

        glTranslatef(-hinge_offset, 0.f, 0.f);
        glRotatef(o->ry, 0.f, 1.f, 0.f);
        glTranslatef(hinge_offset, 0.f, 0.f);
        glScalef(o->sx, o->sy, o->sz);
    }
    else {
        glRotatef(o->rx, 1.f, 0.f, 0.f);
        glRotatef(o->ry, 0.f, 1.f, 0.f);
        glRotatef(o->rz, 0.f, 0.f, 1.f);
        glScalef(o->sx, o->sy, o->sz);
    }
}

void draw_scene_entity(Scene* sc, SceneEntity* o, float master_light)
{
    ModelEntry* m = &sc->models[o->model_idx];
    TextureEntry* t = &sc->textures[o->tex_idx];

    if (str_ieq(o->type, "ground")) {
        draw_ground_plane(t, o);
        return;
    }

    glPushMatrix();
    apply_entity_transform(o);

    if (str_ieq(o->type, "lamp")) {
        int active = is_lamp_active(sc, o);
        float pulse = lamp_pulse_value(sc, o);

        glColor3f(1.0f, 1.0f, 1.0f);

        if (t->loaded && t->tex.id != 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, t->tex.id);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        }

        if (active) {
            glColor3f(0.90f, 0.90f, 0.90f);
        } else {
            glColor3f(0.30f, 0.30f, 0.32f);
        }

        if (m->loaded) {
            draw_model(&m->model);
        }

        if (t->loaded && t->tex.id != 0) {
            glDisable(GL_TEXTURE_2D);
        }

        draw_wall_lamp_glow(o, active ? pulse : 0.0f, active);
    }
    else if (str_ieq(o->type, "switch")) {
        float pulse = broken_switch_pulse(sc, o);

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

        glDisable(GL_LIGHTING);

        draw_switch_status_light(o->state == 1, pulse);

        if (o->state == 0) {
            draw_switch_sparks(sc, o);
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
}

void scene_draw(Scene* sc, int picked_index, float master_light)
{
    if (!sc) {
        return;
    }

    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    for (int i = 0; i < sc->obj_count; i++) {
        if (i == picked_index) {
            continue;
        }

        if (strstr(sc->objects[i].id, "_collision") != NULL) {
            continue;
        }

        draw_scene_entity(sc, &sc->objects[i], master_light);
    }

    if (picked_index >= 0 && picked_index < sc->obj_count) {
        SceneEntity* o = &sc->objects[picked_index];

        if (strstr(o->id, "_collision") != NULL) {
            glStencilMask(0xFF);
            glDisable(GL_STENCIL_TEST);
            return;
        }

        {
            ModelEntry* m = &sc->models[o->model_idx];

            glStencilMask(0xFF);
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

            draw_scene_entity(sc, o, master_light);

            if (is_interactable_type(o->type) && m->loaded) {
                glStencilMask(0x00);
                glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
                glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

                glDisable(GL_TEXTURE_2D);
                glDisable(GL_LIGHTING);

                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(-1.0f, -1.0f);

                glPushMatrix();
                apply_entity_transform(o);

                if (str_ieq(o->type, "switch")) {
                    glScalef(1.04f, 1.04f, 1.04f);
                } else {
                    glScalef(1.03f, 1.03f, 1.03f);
                }

                glColor3f(1.0f, 0.85f, 0.20f);
                draw_model(&m->model);

                glPopMatrix();

                glDisable(GL_POLYGON_OFFSET_FILL);
                glEnable(GL_LIGHTING);
                glEnable(GL_TEXTURE_2D);
                glColor3f(1.0f, 1.0f, 1.0f);
            }
        }
    }

    glStencilMask(0xFF);
    glDisable(GL_STENCIL_TEST);
}

void scene_destroy(Scene* sc)
{
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