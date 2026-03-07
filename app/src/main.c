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

static void apply_lighting(float intensity) {
    if (intensity < 0.0f) intensity = 0.0f;
    if (intensity > 2.0f) intensity = 2.0f;

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    // Specular highlight so the lighting is clearly visible
    GLfloat specular[] = { 0.25f * intensity, 0.25f * intensity, 0.25f * intensity, 1.0f };
    GLfloat shininess[] = { 24.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

    // Ambient + Diffuse
    GLfloat ambient[] = { 0.12f * intensity, 0.12f * intensity, 0.12f * intensity, 1.0f };
    GLfloat diffuse[] = { 0.90f * intensity, 0.90f * intensity, 0.90f * intensity, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
}

static void draw_lit_cube(void) {
    glPushMatrix();
    glTranslatef(0.0f, 1.0f, 0.0f);
    glScalef(2.0f, 2.0f, 2.0f);

    // With GL_COLOR_MATERIAL enabled, color acts as diffuse material
    glColor3f(0.80f, 0.85f, 0.90f);

    glBegin(GL_QUADS);

    // +Z
    glNormal3f(0.f, 0.f, 1.f);
    glVertex3f(-0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);

    // -Z
    glNormal3f(0.f, 0.f, -1.f);
    glVertex3f( 0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);

    // +X
    glNormal3f(1.f, 0.f, 0.f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);

    // -X
    glNormal3f(-1.f, 0.f, 0.f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f);

    // +Y
    glNormal3f(0.f, 1.f, 0.f);
    glVertex3f(-0.5f,  0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f);

    // -Y
    glNormal3f(0.f, -1.f, 0.f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f(-0.5f, -0.5f,  0.5f);

    glEnd();
    glPopMatrix();
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

    // Lighting control
    float light_intensity = 1.0f; // [0.0 .. 2.0]

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
                SDL_Keycode key = event.key.keysym.sym;

                if (key == SDLK_ESCAPE) {
                    is_running = false;
                }

                // Light intensity controls: + / - (also works on numpad)
                if (key == SDLK_PLUS || key == SDLK_KP_PLUS || key == SDLK_EQUALS) {
                    light_intensity += 0.1f;
                    if (light_intensity > 2.0f) light_intensity = 2.0f;
                } else if (key == SDLK_MINUS || key == SDLK_KP_MINUS) {
                    light_intensity -= 0.1f;
                    if (light_intensity < 0.0f) light_intensity = 0.0f;
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

        glClearColor(0.08f, 0.08f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLoadIdentity();
        apply_camera(&camera);

        // Lighting setup (after camera, so we can set light position in world space)
        apply_lighting(light_intensity);
        GLfloat light_pos[] = { 8.0f, 12.0f, 6.0f, 1.0f }; // point light
        glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

        // Draw grid WITHOUT lighting (always visible)
        glDisable(GL_LIGHTING);
        glBegin(GL_LINES);
        for (int i = -20; i <= 20; i++) {
            glColor3f(0.40f, 0.40f, 0.40f);
            glVertex3f((float)i, 0.0f, -20.0f); glVertex3f((float)i, 0.0f, 20.0f);
            glVertex3f(-20.0f, 0.0f, (float)i); glVertex3f(20.0f, 0.0f, (float)i);
        }
        glEnd();

        // Draw cube WITH lighting
        glEnable(GL_LIGHTING);
        draw_lit_cube();

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}