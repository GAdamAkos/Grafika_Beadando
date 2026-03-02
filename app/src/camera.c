#include "camera.h"
#include <SDL2/SDL.h>
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

void update_camera(Camera* camera, double delta_time) {
    const Uint8* state = SDL_GetKeyboardState(NULL);
    float speed = camera->speed * (float)delta_time;
    
    // Előre-hátra számítása a yaw szög alapján (radiánban)
    float rad_yaw = camera->yaw * M_PI / 180.0f;

    if (state[SDL_SCANCODE_W]) {
        camera->x += cos(rad_yaw) * speed;
        camera->z += sin(rad_yaw) * speed;
    }
    if (state[SDL_SCANCODE_S]) {
        camera->x -= cos(rad_yaw) * speed;
        camera->z -= sin(rad_yaw) * speed;
    }
    if (state[SDL_SCANCODE_A]) {
        camera->x += sin(rad_yaw) * speed;
        camera->z -= cos(rad_yaw) * speed;
    }
    if (state[SDL_SCANCODE_D]) {
        camera->x -= sin(rad_yaw) * speed;
        camera->z += cos(rad_yaw) * speed;
    }
}