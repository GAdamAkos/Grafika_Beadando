#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <stdbool.h>
#include "camera.h" // 1. Az új fejléc beemelése

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int argc, char* argv[]) {
    SDL_Window* window = NULL;
    SDL_GLContext gl_context;
    bool is_running = true;
    SDL_Event event;

    Camera camera; // 2. Kamera példányosítása
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;

    window = SDL_CreateWindow("Substation Night Patrol", 
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                              SCREEN_WIDTH, SCREEN_HEIGHT, 
                              SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

    gl_context = SDL_GL_CreateContext(window);
    
    init_camera(&camera); // 3. Kamera inicializálása
    SDL_SetRelativeMouseMode(SDL_TRUE); // 4. Egér elfogása

    while (is_running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) is_running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) is_running = false;

            // 5. Egérmozgás kezelése a forgatáshoz
            if (event.type == SDL_MOUSEMOTION) {
                camera.yaw += event.motion.xrel * camera.sensitivity;
                camera.pitch -= event.motion.yrel * camera.sensitivity;
                
                if (camera.pitch > 89.0f) camera.pitch = 89.0f;
                if (camera.pitch < -89.0f) camera.pitch = -89.0f;
            }
        }

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW); // 6. Kamera transzformációk alkalmazása
        glLoadIdentity();
        apply_camera(&camera);

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}