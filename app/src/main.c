#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "camera.h"
#include "help.h"
#include "scene.h"
#include "collision.h"
#include "app_render.h"
#include "ui_hud.h"

static const int INITIAL_WIDTH = 800;
static const int INITIAL_HEIGHT = 600;

static AABB make_player_aabb(const Camera* cam)
{
    const float half_w = 0.75f;
    const float half_d = 0.75f;
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

static bool is_near_victory_terminal(const Camera* cam)
{
    const float terminal_x = 0.0f;
    const float terminal_z = -23.10f;
    const float max_dx = 3.0f;
    const float max_dz = 3.8f;

    if (!cam) {
        return false;
    }

    return (
        cam->x > terminal_x - max_dx && cam->x < terminal_x + max_dx &&
        cam->z > terminal_z - max_dz && cam->z < terminal_z + max_dz
    );
}

int main(int argc, char* argv[])
{
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
    bool mission_complete = false;
    bool victory_ready = false;
    bool victory_terminal_picked = false;

    Uint32 run_start_ticks = 0;
    double final_time_sec = 0.0;

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
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

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
    glEnable(GL_NORMALIZE);
    setup_projection(window_w, window_h);
    setup_fog(fog_density);

    if (!help_init(&help, "../assets/textures/help.bmp")) {
        printf("WARNING: help overlay not loaded (missing ../assets/textures/help.bmp)\n");
    }

    if (!scene_init(&scene, "../assets/scene.csv")) {
        printf("WARNING: scene not loaded (missing ../assets/scene.csv)\n");
    }

    run_start_ticks = SDL_GetTicks();

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

                        if (victory_ready &&
                            !mission_complete &&
                            victory_terminal_picked &&
                            is_near_victory_terminal(&camera)) {
                            int total_hundredths;
                            int minutes;
                            int seconds;
                            int hundredths;
                            char title[128];

                            mission_complete = true;
                            final_time_sec = (SDL_GetTicks() - run_start_ticks) / 1000.0;

                            total_hundredths = (int)(final_time_sec * 100.0 + 0.5);
                            minutes = total_hundredths / 6000;
                            seconds = (total_hundredths / 100) % 60;
                            hundredths = total_hundredths % 100;

                            snprintf(
                                title,
                                sizeof(title),
                                "Substation Night Patrol - MISSION COMPLETE - %02d:%02d.%02d",
                                minutes, seconds, hundredths
                            );
                            SDL_SetWindowTitle(window, title);

                            printf(
                                "MISSION COMPLETE: Terminal activated. Substation restored. Time: %02d:%02d.%02d\n",
                                minutes, seconds, hundredths
                            );
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

                if (!mission_complete && scene && scene_get_dynamic_light_count(scene) >= 3) {
                    victory_ready = true;
                }

                victory_terminal_picked = false;

                if (scene && picked >= 0) {
                    const SceneEntity* picked_entity = scene_get_entity(scene, picked);

                    if (picked_entity && strcmp(picked_entity->id, "victory_core") == 0) {
                        if (victory_ready && is_near_victory_terminal(&camera)) {
                            victory_terminal_picked = true;
                        } else {
                            picked = -1;
                        }
                    }
                }
            }

            glClearColor(0.08f, 0.08f, 0.12f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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
                double hud_time_sec = mission_complete
                    ? final_time_sec
                    : (SDL_GetTicks() - run_start_ticks) / 1000.0;

                draw_crosshair(window_w, window_h);
                draw_bottom_hud(window_w, window_h, hud_time_sec);

                if (mission_complete) {
                    draw_victory_overlay(window_w, window_h, final_time_sec);
                }
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