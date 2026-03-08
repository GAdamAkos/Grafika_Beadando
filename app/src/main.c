#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdbool.h>

#include "camera.h"
#include "help.h"

// Tanári OBJ loader (static lib)
#include "model.h"
#include "load.h"
#include "draw.h"

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

static void apply_lighting(float intensity) {
    if (intensity < 0.0f) intensity = 0.0f;
    if (intensity > 2.0f) intensity = 2.0f;

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    GLfloat specular[] = { 0.25f * intensity, 0.25f * intensity, 0.25f * intensity, 1.0f };
    GLfloat shininess[] = { 24.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

    GLfloat ambient[] = { 0.12f * intensity, 0.12f * intensity, 0.12f * intensity, 1.0f };
    GLfloat diffuse[] = { 0.90f * intensity, 0.90f * intensity, 0.90f * intensity, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
}

static void draw_grid(void) {
    glDisable(GL_LIGHTING);

    glBegin(GL_LINES);
    for (int i = -20; i <= 20; i++) {
        glColor3f(0.40f, 0.40f, 0.40f);
        glVertex3f((float)i, 0.0f, -20.0f); glVertex3f((float)i, 0.0f, 20.0f);
        glVertex3f(-20.0f, 0.0f, (float)i); glVertex3f(20.0f, 0.0f, (float)i);
    }
    glEnd();

    glEnable(GL_LIGHTING);
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    SDL_Window* window = NULL;
    SDL_GLContext gl_context = NULL;
    SDL_Event event;

    bool is_running = true;

    int window_w = INITIAL_WIDTH;
    int window_h = INITIAL_HEIGHT;

    Camera camera;
    float light_intensity = 1.0f;

    bool show_help = false;
    HelpOverlay* help = NULL;

    // OBJ model
    Model obj_model;
    init_model(&obj_model);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

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

    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        printf("OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_GL_SetSwapInterval(1);

    SDL_GetWindowSize(window, &window_w, &window_h);

    init_camera(&camera);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    glEnable(GL_DEPTH_TEST);

    setup_projection(window_w, window_h);

    // Help overlay BMP
    if (!help_init(&help, "../assets/textures/help.bmp")) {
        printf("WARNING: help overlay not loaded (missing ../assets/textures/help.bmp)\n");
    }

    // Load OBJ model (from assets)
    if (load_model(&obj_model, "../assets/models/cube.obj") != TRUE) {
        printf("WARNING: failed to load OBJ model: ../assets/models/cube.obj\n");
    } else {
        printf("OBJ loaded: %d triangles\n", obj_model.n_triangles);
    }

    Uint32 last_time = SDL_GetTicks();

    while (is_running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                is_running = false;
            } else if (event.type == SDL_KEYDOWN) {
                SDL_Keycode key = event.key.keysym.sym;

                if (key == SDLK_ESCAPE) {
                    is_running = false;
                }

                if (key == SDLK_F1) {
                    show_help = !show_help;
                    SDL_SetRelativeMouseMode(show_help ? SDL_FALSE : SDL_TRUE);
                }

                if (key == SDLK_PLUS || key == SDLK_KP_PLUS || key == SDLK_EQUALS) {
                    light_intensity += 0.1f;
                    if (light_intensity > 2.0f) light_intensity = 2.0f;
                } else if (key == SDLK_MINUS || key == SDLK_KP_MINUS) {
                    light_intensity -= 0.1f;
                    if (light_intensity < 0.0f) light_intensity = 0.0f;
                }

            } else if (event.type == SDL_MOUSEMOTION) {
                if (!show_help) {
                    camera.yaw += event.motion.xrel * camera.sensitivity;
                    camera.pitch -= event.motion.yrel * camera.sensitivity;

                    if (camera.pitch > 89.0f) camera.pitch = 89.0f;
                    if (camera.pitch < -89.0f) camera.pitch = -89.0f;
                }

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

        if (!show_help) {
            update_camera(&camera, delta_time);
        }

        glClearColor(0.08f, 0.08f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLoadIdentity();
        apply_camera(&camera);

        apply_lighting(light_intensity);
        GLfloat light_pos[] = { 8.0f, 12.0f, 6.0f, 1.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

        draw_grid();

        // Draw OBJ model (lit)
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 0.0f);
        glScalef(1.0f, 1.0f, 1.0f);

        // Give it a neutral color (until textures come next milestone)
        glColor3f(0.85f, 0.85f, 0.90f);
        draw_model(&obj_model);

        glPopMatrix();

        // Help overlay on top
        if (show_help && help) {
            help_draw(help, window_w, window_h);
        }

        SDL_GL_SwapWindow(window);
    }

    help_destroy(help);
    free_model(&obj_model);

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}