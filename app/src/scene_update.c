#include "scene_internal.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

int extract_trailing_number(const char* id)
{
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

SceneEntity* find_switch_for_lamp(Scene* sc, const SceneEntity* lamp)
{
    int lamp_num = extract_trailing_number(lamp->id);
    if (lamp_num < 0) {
        return NULL;
    }

    for (int i = 0; i < sc->obj_count; i++) {
        SceneEntity* obj = &sc->objects[i];

        if (!str_ieq(obj->type, "switch")) {
            continue;
        }

        if (extract_trailing_number(obj->id) == lamp_num) {
            return obj;
        }
    }

    return NULL;
}

int is_lamp_active(Scene* sc, const SceneEntity* lamp)
{
    SceneEntity* linked_switch = find_switch_for_lamp(sc, lamp);
    if (!linked_switch) {
        return 0;
    }

    return (linked_switch->state == 1);
}

float lamp_pulse_value(Scene* sc, const SceneEntity* lamp)
{
    int n = extract_trailing_number(lamp->id);
    float phase = (n > 0) ? (float)n * 1.35f : 0.0f;
    return 0.55f + 0.45f * (0.5f + 0.5f * sinf(sc->animation_time * 4.0f + phase));
}

void get_object_forward_from_ry(float ry_deg, float* fx, float* fz)
{
    float a = deg_to_rad(ry_deg);

    if (fx) *fx = sinf(a);
    if (fz) *fz = cosf(a);
}

float broken_switch_pulse(Scene* sc, const SceneEntity* sw)
{
    int n = extract_trailing_number(sw->id);
    float phase = (n > 0) ? (float)n * 0.9f : 0.0f;
    return 0.5f
        + 0.30f * sinf(sc->animation_time * 9.0f + phase)
        + 0.20f * sinf(sc->animation_time * 23.0f + phase * 1.7f);
}

int count_active_switches(Scene* sc)
{
    int count = 0;

    for (int i = 0; i < sc->obj_count; i++) {
        if (str_ieq(sc->objects[i].type, "switch") && sc->objects[i].state == 1) {
            count++;
        }
    }

    return count;
}

void scene_recompute_power(Scene* sc)
{
    int active_switches = count_active_switches(sc);
    sc->power_on = (active_switches > 0) ? 1 : 0;
    sc->gate_target_angle = (active_switches >= 3) ? 90.0f : 0.0f;
}

void update_gate_transforms(Scene* sc)
{
    for (int i = 0; i < sc->obj_count; i++) {
        SceneEntity* o = &sc->objects[i];

        if (!is_gate_object(o)) {
            continue;
        }

        o->px = o->base_px;
        o->py = o->base_py;
        o->pz = o->base_pz;
        o->rx = 0.0f;
        o->rz = 0.0f;
        o->ry = 0.0f;

        if (strcmp(o->id, "gate_left") == 0) {
            o->ry = -sc->gate_open_angle;
        }
        else if (strcmp(o->id, "gate_right") == 0) {
            o->ry = sc->gate_open_angle;
        }

        compute_object_aabb(o);
    }
}

void scene_update(Scene* sc, double delta_time)
{
    if (!sc) {
        return;
    }

    sc->animation_time += (float)delta_time;

    {
        float speed = 180.0f * (float)delta_time;

        if (sc->gate_open_angle < sc->gate_target_angle) {
            sc->gate_open_angle += speed;
            if (sc->gate_open_angle > sc->gate_target_angle) {
                sc->gate_open_angle = sc->gate_target_angle;
            }
        }
        else if (sc->gate_open_angle > sc->gate_target_angle) {
            sc->gate_open_angle -= speed;
            if (sc->gate_open_angle < sc->gate_target_angle) {
                sc->gate_open_angle = sc->gate_target_angle;
            }
        }
    }

    update_gate_transforms(sc);
}

void scene_interact(Scene* sc, int picked_index)
{
    if (!sc) return;
    if (picked_index < 0 || picked_index >= sc->obj_count) return;

    {
        SceneEntity* o = &sc->objects[picked_index];

        if (str_ieq(o->type, "switch")) {
            if (o->state == 0) {
                o->state = 1;
                scene_recompute_power(sc);

                printf(
                    "REPAIR: %s fixed -> %d | fixed_switches=%d | gate_target_angle=%.2f\n",
                    o->id,
                    o->state,
                    count_active_switches(sc),
                    sc->gate_target_angle
                );
            } else {
                printf("REPAIR: %s is already fixed\n", o->id);
            }
        }
        else if (str_ieq(o->type, "terminal")) {
            printf("TERMINAL: %s ready for activation\n", o->id);
        }
    }
}