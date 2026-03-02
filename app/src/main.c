#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdbool.h>
#include "camera.h"

// Window constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int argc, char* argv[]) {
    SDL_Window* window = NULL;
    SDL_GLContext gl_context;
    bool is_running = true;
    SDL_Event event;

    // Initialize camera structure
    Camera camera;
    
    // Initialize SDL video subsystem
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Set OpenGL attributes (using compatibility profile for GLU and legacy GL)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Create Window
    window = SDL_CreateWindow("Substation Night Patrol", 
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                              SCREEN_WIDTH, SCREEN_HEIGHT, 
                              SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create OpenGL Context
    gl_context = SDL_GL_CreateContext(window);
    if (gl_context == NULL) {
        printf("OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Initialize our camera and capture mouse
    init_camera(&camera);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    // Enable Depth Testing for 3D
    glEnable(GL_DEPTH_TEST);

    // Timing variables
    Uint32 last_time = SDL_GetTicks();

    // Main Loop
    while (is_running) {
        // 1. Handle Events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                is_running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    is_running = false;
                }
            }
            // Mouse look logic
            if (event.type == SDL_MOUSEMOTION) {
                camera.yaw += event.motion.xrel * camera.sensitivity;
                camera.pitch -= event.motion.yrel * camera.sensitivity;
                
                // Clamp pitch to avoid flipping over
                if (camera.pitch > 89.0f) camera.pitch = 89.0f;
                if (camera.pitch < -89.0f) camera.pitch = -89.0f;
            }
        }

        // 2. Calculate Delta Time for smooth movement
        Uint32 current_time = SDL_GetTicks();
        float delta_time = (current_time - last_time) / 1000.0f;
        last_time = current_time;

        // 3. Update Logic
        update_camera(&camera, delta_time);

        // 4. Rendering
        // Clear screen and depth buffer
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set Projection Matrix (Perspective)
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float aspect = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
        gluPerspective(60.0, aspect, 0.1, 1000.0); 

        // Set Modelview Matrix (Camera/Objects)
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        apply_camera(&camera);

        // Draw a simple floor grid to visualize movement
        glBegin(GL_LINES);
        for(int i = -20; i <= 20; i++) {
            glColor3f(0.4f, 0.4f, 0.4f);
            glVertex3f((float)i, 0.0f, -20.0f); glVertex3f((float)i, 0.0f, 20.0f);
            glVertex3f(-20.0f, 0.0f, (float)i); glVertex3f(20.0f, 0.0f, (float)i);
        }
        glEnd();

        // 5. Swap buffers to show the frame
        SDL_GL_SwapWindow(window);
    }

    // --- Cleanup ---
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}