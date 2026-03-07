#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdbool.h>

#include "camera.h"

// Initial window size (only initial, not fixed!)
static const int INITIAL_WIDTH = 800;
static const int INITIAL_HEIGHT = 600;

static void setup_projection(int window_w, int window_h) {
    if (window_h <= 0) window_h = 1;

    glViewport(0, 0, window_w, window_h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float aspect = (float)window_w / (float)window_h;
    gluPerspective(60.0, aspect, 0.1, 1000.0);

    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    SDL_Window* window = NULL;
    SDL_GLContext gl_context = NULL;
    bool is_running = true;
    SDL_Event event;

    int window_w = INITIAL_WIDTH;
    int window_h = INITIAL_HEIGHT;

    Camera camera;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // OpenGL attributes (compat profile for legacy + GLU)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    window = SDL_CreateWindow(
        "Substation Night Patrol",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        INITIAL_WIDTH, INITIAL_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN
    );

    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    gl_context = SDL_GL_CreateContext(window);
    if (gl_context == NULL) {
        printf("OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Optional but recommended: vsync (less CPU burn)
    SDL_GL_SetSwapInterval(1);

    // Get actual window size (important on some setups)
    SDL_GetWindowSize(window, &window_w, &window_h);

    init_camera(&camera);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    glEnable(GL_DEPTH_TEST);

    // Setup initial viewport + projection
    setup_projection(window_w, window_h);

    Uint32 last_time = SDL_GetTicks();

    while (is_running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                is_running = false;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    is_running = false;
                }
            } else if (event.type == SDL_MOUSEMOTION) {
                camera.yaw += event.motion.xrel * camera.sensitivity;
                camera.pitch -= event.motion.yrel * camera.sensitivity;

                if (camera.pitch > 89.0f) camera.pitch = 89.0f;
                if (camera.pitch < -89.0f) camera.pitch = -89.0f;
            } else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
                    event.window.event == SDL_WINDOWEVENT_RESIZED) {

                    window_w = event.window.data1;
                    window_h = event.window.data2;
                    setup_projection(window_w, window_h);
                }
            }
        }

        Uint32 current_time = SDL_GetTicks();
        float delta_time = (current_time - last_time) / 1000.0f;
        last_time = current_time;

        update_camera(&camera, delta_time);

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLoadIdentity();
        apply_camera(&camera);

        // Floor grid
        glBegin(GL_LINES);
        for (int i = -20; i <= 20; i++) {
            glColor3f(0.4f, 0.4f, 0.4f);
            glVertex3f((float)i, 0.0f, -20.0f); glVertex3f((float)i, 0.0f, 20.0f);
            glVertex3f(-20.0f, 0.0f, (float)i); glVertex3f(20.0f, 0.0f, (float)i);
        }
        glEnd();

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}