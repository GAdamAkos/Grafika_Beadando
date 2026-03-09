#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdbool.h>

#include "camera.h"
#include "help.h"
#include "scene.h"
#include "collision.h"

static const int INITIAL_WIDTH = 800;
static const int INITIAL_HEIGHT = 600;

static void setup_projection(int window_w, int window_h) {
    if (window_h <= 0) {
        window_h = 1;
    }

    glViewport(0, 0, window_w, window_h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    {
        float aspect = (float)window_w / (float)window_h;
        gluPerspective(60.0, aspect, 0.5, 1000.0);
    }

    glMatrixMode(GL_MODELVIEW);
}

static void apply_lighting(float intensity) {
    if (intensity < 0.2f) {
        intensity = 0.2f;
    }
    if (intensity > 2.0f) {
        intensity = 2.0f;
    }

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    {
        GLfloat material_specular[] = {
            0.30f * intensity,
            0.30f * intensity,
            0.30f * intensity,
            1.0f
        };
        GLfloat material_shininess[] = { 24.0f };

        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_specular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, material_shininess);
    }

    {
        GLfloat ambient[] = {
            0.18f * intensity,
            0.18f * intensity,
            0.20f * intensity,
            1.0f
        };

        GLfloat diffuse[] = {
            0.85f * intensity,
            0.85f * intensity,
            0.80f * intensity,
            1.0f
        };

        GLfloat specular[] = {
            0.30f * intensity,
            0.30f * intensity,
            0.30f * intensity,
            1.0f
        };

        glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    }
}

static void apply_dynamic_lights(Scene* scene, float master_intensity) {
    GLenum light_ids[3] = { GL_LIGHT1, GL_LIGHT2, GL_LIGHT3 };

    for (int i = 0; i < 3; i++) {
        glDisable(light_ids[i]);
    }

    if (!scene) {
        return;
    }

    {
        int active_count = scene_get_dynamic_light_count(scene);
        if (active_count > 3) {
            active_count = 3;
        }

        for (int i = 0; i < active_count; i++) {
            float x, y, z, pulse;

            if (!scene_get_dynamic_light(scene, i, &x, &y, &z, &pulse)) {
                continue;
            }

            {
                GLfloat position[] = { x, y, z, 1.0f };

                GLfloat ambient[] = {
                    0.04f * master_intensity * pulse,
                    0.04f * master_intensity * pulse,
                    0.03f * master_intensity * pulse,
                    1.0f
                };

                GLfloat diffuse[] = {
                    0.90f * master_intensity * pulse,
                    0.78f * master_intensity * pulse,
                    0.42f * master_intensity * pulse,
                    1.0f
                };

                GLfloat specular[] = {
                    0.28f * master_intensity * pulse,
                    0.22f * master_intensity * pulse,
                    0.12f * master_intensity * pulse,
                    1.0f
                };

                glEnable(light_ids[i]);
                glLightfv(light_ids[i], GL_POSITION, position);
                glLightfv(light_ids[i], GL_AMBIENT, ambient);
                glLightfv(light_ids[i], GL_DIFFUSE, diffuse);
                glLightfv(light_ids[i], GL_SPECULAR, specular);

                glLightf(light_ids[i], GL_CONSTANT_ATTENUATION, 1.0f);
                glLightf(light_ids[i], GL_LINEAR_ATTENUATION, 0.04f);
                glLightf(light_ids[i], GL_QUADRATIC_ATTENUATION, 0.01f);
            }
        }
    }
}

static void draw_grid(void) {
    glDisable(GL_LIGHTING);

    glBegin(GL_LINES);
    for (int i = -40; i <= 40; i++) {
        glColor3f(0.40f, 0.40f, 0.40f);
        glVertex3f((float)i, 0.0f, -40.0f);
        glVertex3f((float)i, 0.0f, 40.0f);

        glVertex3f(-40.0f, 0.0f, (float)i);
        glVertex3f(40.0f, 0.0f, (float)i);
    }
    glEnd();

    glEnable(GL_LIGHTING);
}

static AABB make_player_aabb(const Camera* cam) {
    const float half_w = 0.25f;
    const float half_d = 0.25f;
    const float height = 1.7f;

    float feet_y = cam->y - cam->M;

    AABB a;
    a.min_x = cam->x - half_w;
    a.max_x = cam->x + half_w;

    a.min_y = feet_y;
    a.max_y = feet_y + height;

    a.min_z = cam->z - half_d;
    a.max_z = cam->z + half_d;

    return a;
}

static void begin_2d(int w, int h) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, w, h, 0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

static void end_2d(void) {
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
}

static void draw_crosshair(int w, int h) {
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    begin_2d(w, h);

    {
        float cx = w * 0.5f;
        float cy = h * 0.5f;
        float len = 6.0f;
        float gap = 3.0f;

        glColor4f(1.f, 1.f, 1.f, 0.75f);
        glLineWidth(2.0f);

        glBegin(GL_LINES);
        glVertex2f(cx - gap - len, cy);
        glVertex2f(cx - gap,       cy);

        glVertex2f(cx + gap,       cy);
        glVertex2f(cx + gap + len, cy);

        glVertex2f(cx, cy - gap - len);
        glVertex2f(cx, cy - gap);

        glVertex2f(cx, cy + gap);
        glVertex2f(cx, cy + gap + len);
        glEnd();

        glLineWidth(1.0f);
    }

    end_2d();

    glPopAttrib();
}

static void setup_fog(float density) {
    if (density < 0.0f) {
        density = 0.0f;
    }

    glEnable(GL_FOG);

    {
        GLfloat fog_color[4] = { 0.08f, 0.08f, 0.12f, 1.0f };
        glFogfv(GL_FOG_COLOR, fog_color);
    }

    glFogi(GL_FOG_MODE, GL_EXP2);
    glFogf(GL_FOG_DENSITY, density);

    glHint(GL_FOG_HINT, GL_NICEST);
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

    Scene* scene = NULL;
    int picked = -1;

    float fog_density = 0.05f;
    bool show_grid = false;

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
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        INITIAL_WIDTH,
        INITIAL_HEIGHT,
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
    setup_fog(fog_density);

    if (!help_init(&help, "../assets/textures/help.bmp")) {
        printf("WARNING: help overlay not loaded (missing ../assets/textures/help.bmp)\n");
    }

    if (!scene_init(&scene, "../assets/scene.csv")) {
        printf("WARNING: scene not loaded (missing ../assets/scene.csv)\n");
    }

    {
        Uint32 last_time = SDL_GetTicks();

        while (is_running) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    is_running = false;
                }
                else if (event.type == SDL_KEYDOWN) {
                    SDL_Keycode key = event.key.keysym.sym;

                    if (key == SDLK_ESCAPE) {
                        is_running = false;
                    }

                    if (key == SDLK_F1) {
                        show_help = !show_help;
                        SDL_SetRelativeMouseMode(show_help ? SDL_FALSE : SDL_TRUE);
                    }

                    if (key == SDLK_g && !show_help) {
                        show_grid = !show_grid;
                        printf("Debug grid: %s\n", show_grid ? "ON" : "OFF");
                    }

                    if (key == SDLK_e && !show_help) {
                        if (scene) {
                            scene_interact(scene, picked);
                        }
                    }

                    if (key == SDLK_EQUALS || key == SDLK_PLUS || key == SDLK_KP_PLUS) {
                        light_intensity += 0.1f;
                        if (light_intensity > 2.0f) {
                            light_intensity = 2.0f;
                        }
                        printf("Light intensity: %.2f\n", light_intensity);
                    }
                    else if (key == SDLK_MINUS || key == SDLK_KP_MINUS) {
                        light_intensity -= 0.1f;
                        if (light_intensity < 0.2f) {
                            light_intensity = 0.2f;
                        }
                        printf("Light intensity: %.2f\n", light_intensity);
                    }

                    if (key == SDLK_COMMA) {
                        fog_density -= 0.002f;
                        if (fog_density < 0.0f) {
                            fog_density = 0.0f;
                        }
                        setup_fog(fog_density);
                        printf("Fog density: %.3f\n", fog_density);
                    }
                    else if (key == SDLK_PERIOD) {
                        fog_density += 0.002f;
                        if (fog_density > 0.15f) {
                            fog_density = 0.15f;
                        }
                        setup_fog(fog_density);
                        printf("Fog density: %.3f\n", fog_density);
                    }
                }
                else if (event.type == SDL_MOUSEMOTION) {
                    if (!show_help) {
                        camera.yaw += event.motion.xrel * camera.sensitivity;
                        camera.pitch -= event.motion.yrel * camera.sensitivity;

                        if (camera.pitch > 89.0f) {
                            camera.pitch = 89.0f;
                        }
                        if (camera.pitch < -89.0f) {
                            camera.pitch = -89.0f;
                        }
                    }
                }
                else if (event.type == SDL_WINDOWEVENT) {
                    if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
                        event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        window_w = event.window.data1;
                        window_h = event.window.data2;
                        setup_projection(window_w, window_h);
                    }
                }
            }

            {
                Uint32 current_time = SDL_GetTicks();
                double delta_time = (current_time - last_time) / 1000.0;
                last_time = current_time;

                if (scene) {
                    scene_update(scene, delta_time);
                }

                if (!show_help) {
                    float old_x = camera.x;
                    float old_y = camera.y;
                    float old_z = camera.z;

                    update_camera(&camera, delta_time);

                    if (scene) {
                        AABB player = make_player_aabb(&camera);
                        if (scene_collides(scene, &player)) {
                            camera.x = old_x;
                            camera.y = old_y;
                            camera.z = old_z;
                        }
                    }
                }

                if (scene && !show_help) {
                    picked = scene_pick(scene, &camera);
                } else {
                    picked = -1;
                }
            }

            glClearColor(0.08f, 0.08f, 0.12f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glLoadIdentity();
            apply_camera(&camera);

            apply_lighting(light_intensity);

            {
                GLfloat light_pos[] = { 8.0f, 12.0f, 6.0f, 1.0f };
                glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
            }

            apply_dynamic_lights(scene, light_intensity);

            if (show_grid) {
                draw_grid();
            }

            if (scene) {
                scene_draw(scene, picked, light_intensity);
            }

            if (show_help && help) {
                help_draw(help, window_w, window_h);
            } else {
                draw_crosshair(window_w, window_h);
            }

            SDL_GL_SwapWindow(window);
        }
    }

    scene_destroy(scene);
    help_destroy(help);

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}