#include "camera.h"
#include <SDL2/SDL_opengl.h>
#include <math.h>

void init_camera(Camera* camera) {
    camera->x = 0.0f;
    camera->y = 0.0f;
    camera->z = 5.0f;
    camera->M = 1.7f; // Standard human eye height
    camera->pitch = 0.0f;
    camera->yaw = -90.0f;
    camera->speed = 5.0f;
    camera->sensitivity = 0.1f;
}

void apply_camera(Camera* camera) {
    glRotatef(-camera->pitch, 1.0f, 0.0f, 0.0f);
    glRotatef(-camera->yaw - 90.0f, 0.0f, 1.0f, 0.0f);
    glTranslatef(-camera->x, -camera->M, -camera->z);
}