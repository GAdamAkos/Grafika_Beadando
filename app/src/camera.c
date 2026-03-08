#include "camera.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

static float deg_to_rad(float deg) {
    return deg * (float)M_PI / 180.0f;
}

void init_camera(Camera* camera) {
    camera->x = 0.0f;
    camera->y = 1.7f;
    camera->z = 8.0f;

    camera->M = 1.7f;

    camera->pitch = 0.0f;
    camera->yaw = 180.0f;

    camera->speed = 6.0f;
    camera->sensitivity = 0.12f;
}

void update_camera(Camera* camera, double delta_time) {
    const Uint8* keys = SDL_GetKeyboardState(NULL);

    float dt = (float)delta_time;
    float move = camera->speed * dt;

    float yaw_rad = deg_to_rad(camera->yaw);

    // Forward vector on XZ plane
    float fx = cosf(yaw_rad);
    float fz = sinf(yaw_rad);

    // Right vector on XZ plane (90° rotated)
    float rx = cosf(yaw_rad + (float)M_PI_2);
    float rz = sinf(yaw_rad + (float)M_PI_2);

    float dx = 0.0f;
    float dz = 0.0f;

    if (keys[SDL_SCANCODE_W]) { dx += fx * move; dz += fz * move; }
    if (keys[SDL_SCANCODE_S]) { dx -= fx * move; dz -= fz * move; }
    if (keys[SDL_SCANCODE_D]) { dx += rx * move; dz += rz * move; }
    if (keys[SDL_SCANCODE_A]) { dx -= rx * move; dz -= rz * move; }

    camera->x += dx;
    camera->z += dz;
}

void apply_camera(Camera* camera) {
    float yaw_rad = deg_to_rad(camera->yaw);
    float pitch_rad = deg_to_rad(camera->pitch);

    float dir_x = cosf(pitch_rad) * cosf(yaw_rad);
    float dir_y = sinf(pitch_rad);
    float dir_z = cosf(pitch_rad) * sinf(yaw_rad);

    float ex = camera->x;
    float ey = camera->y;
    float ez = camera->z;

    float tx = ex + dir_x;
    float ty = ey + dir_y;
    float tz = ez + dir_z;

    gluLookAt(ex, ey, ez, tx, ty, tz, 0.0f, 1.0f, 0.0f);
}