#include "scene_internal.h"

#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

float deg_to_rad(float deg)
{
    return deg * (float)M_PI / 180.0f;
}

int is_gate_object(const SceneEntity* o)
{
    return (o != NULL && str_ieq(o->type, "gate"));
}

int gate_hinge_sign(const SceneEntity* o)
{
    if (o == NULL) {
        return 0;
    }

    if (strcmp(o->id, "gate_left") == 0) {
        return 1;
    }

    if (strcmp(o->id, "gate_right") == 0) {
        return -1;
    }

    return 0;
}

void compute_gate_aabb(SceneEntity* o)
{
    float half_w;
    float half_h;
    float half_d;
    float angle;
    float c;
    float s;
    float dir;
    float hinge_x;
    float hinge_z;
    float center_x;
    float center_z;
    float hx;
    float hz;

    if (o == NULL) {
        return;
    }

    half_w = 0.5f * o->sx;
    half_h = 0.5f * o->sy;
    half_d = 0.5f * o->sz;
    angle = deg_to_rad(o->ry);
    c = cosf(angle);
    s = sinf(angle);
    dir = (float)gate_hinge_sign(o);

    hinge_x = o->px - dir * half_w;
    hinge_z = o->pz;

    center_x = hinge_x + dir * half_w * c;
    center_z = hinge_z - dir * half_w * s;

    hx = fabsf(c) * half_w + fabsf(s) * half_d;
    hz = fabsf(s) * half_w + fabsf(c) * half_d;

    o->box.min_x = center_x - hx;
    o->box.max_x = center_x + hx;
    o->box.min_y = o->py - half_h;
    o->box.max_y = o->py + half_h;
    o->box.min_z = center_z - hz;
    o->box.max_z = center_z + hz;
}

void compute_object_aabb(SceneEntity* o)
{
    float hx;
    float hy;
    float hz;

    if (is_gate_object(o)) {
        compute_gate_aabb(o);
        return;
    }

    hx = 0.5f * o->sx;
    hy = 0.5f * o->sy;
    hz = 0.5f * o->sz;

    if (fabsf(o->ry) > 0.001f) {
        float angle = deg_to_rad(o->ry);
        float c = fabsf(cosf(angle));
        float s = fabsf(sinf(angle));

        float rotated_hx = c * hx + s * hz;
        float rotated_hz = s * hx + c * hz;

        hx = rotated_hx;
        hz = rotated_hz;
    }

    o->box.min_x = o->px - hx;
    o->box.max_x = o->px + hx;
    o->box.min_y = o->py - hy;
    o->box.max_y = o->py + hy;
    o->box.min_z = o->pz - hz;
    o->box.max_z = o->pz + hz;
}

bool ray_aabb_hit(
    const float ox, const float oy, const float oz,
    const float dx, const float dy, const float dz,
    const AABB* b, float* out_t
)
{
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

bool scene_collides(Scene* sc, const AABB* player_box)
{
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

int scene_pick(Scene* sc, const Camera* cam)
{
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

int scene_get_dynamic_light_count(Scene* sc)
{
    int count = 0;

    if (!sc) {
        return 0;
    }

    for (int i = 0; i < sc->obj_count; i++) {
        SceneEntity* o = &sc->objects[i];

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
)
{
    int current = 0;

    if (!sc) {
        return false;
    }

    for (int i = 0; i < sc->obj_count; i++) {
        SceneEntity* o = &sc->objects[i];

        if (!str_ieq(o->type, "lamp")) continue;
        if (!is_lamp_active(sc, o)) continue;

        if (current == active_index) {
            float fx = 0.0f;
            float fz = 1.0f;
            float pulse = lamp_pulse_value(sc, o);

            get_object_forward_from_ry(o->ry, &fx, &fz);

            if (x) *x = o->px + fx * 0.42f;
            if (y) *y = o->py;
            if (z) *z = o->pz + fz * 0.42f;
            if (intensity) *intensity = 0.45f + 1.8f * pulse;

            return true;
        }

        current++;
    }

    return false;
}

const SceneEntity* scene_get_entity(const Scene* sc, int index)
{
    if (!sc) {
        return NULL;
    }

    if (index < 0 || index >= sc->obj_count) {
        return NULL;
    }

    return &sc->objects[index];
}