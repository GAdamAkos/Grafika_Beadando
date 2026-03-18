#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

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
            0.22f * intensity,
            0.22f * intensity,
            0.22f * intensity,
            1.0f
        };
        GLfloat material_shininess[] = { 24.0f };

        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_specular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, material_shininess);
    }

    {
        GLfloat ambient[] = {
            0.10f * intensity,
            0.10f * intensity,
            0.12f * intensity,
            1.0f
        };

        GLfloat diffuse[] = {
            0.42f * intensity,
            0.42f * intensity,
            0.40f * intensity,
            1.0f
        };

        GLfloat specular[] = {
            0.18f * intensity,
            0.18f * intensity,
            0.18f * intensity,
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
                float strength = pulse;

                GLfloat position[] = { x, y, z, 1.0f };

                GLfloat ambient[] = {
                    0.03f * master_intensity * strength,
                    0.025f * master_intensity * strength,
                    0.015f * master_intensity * strength,
                    1.0f
                };

                GLfloat diffuse[] = {
                    1.35f * master_intensity * strength,
                    1.05f * master_intensity * strength,
                    0.45f * master_intensity * strength,
                    1.0f
                };

                GLfloat specular[] = {
                    0.55f * master_intensity * strength,
                    0.40f * master_intensity * strength,
                    0.18f * master_intensity * strength,
                    1.0f
                };

                glEnable(light_ids[i]);
                glLightfv(light_ids[i], GL_POSITION, position);
                glLightfv(light_ids[i], GL_AMBIENT, ambient);
                glLightfv(light_ids[i], GL_DIFFUSE, diffuse);
                glLightfv(light_ids[i], GL_SPECULAR, specular);

                glLightf(light_ids[i], GL_CONSTANT_ATTENUATION, 0.55f);
                glLightf(light_ids[i], GL_LINEAR_ATTENUATION, 0.08f);
                glLightf(light_ids[i], GL_QUADRATIC_ATTENUATION, 0.015f);
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

static bool is_near_victory_terminal(const Camera* cam) {
    const float terminal_x = 0.0f;
    const float terminal_z = -31.0f;
    const float max_dx = 2.4f;
    const float max_dz = 2.8f;

    if (!cam) {
        return false;
    }

    return (
        cam->x > terminal_x - max_dx && cam->x < terminal_x + max_dx &&
        cam->z > terminal_z - max_dz && cam->z < terminal_z + max_dz
    );
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

static void draw_rect_2d(float x0, float y0, float x1, float y1) {
    glBegin(GL_QUADS);
    glVertex2f(x0, y0);
    glVertex2f(x1, y0);
    glVertex2f(x1, y1);
    glVertex2f(x0, y1);
    glEnd();
}

static unsigned char get_digit_mask(int digit) {
    switch (digit) {
        case 0: return 0x3F;
        case 1: return 0x06;
        case 2: return 0x5B;
        case 3: return 0x4F;
        case 4: return 0x66;
        case 5: return 0x6D;
        case 6: return 0x7D;
        case 7: return 0x07;
        case 8: return 0x7F;
        case 9: return 0x6F;
        default: return 0x00;
    }
}

static void draw_digit_7seg(float x, float y, float scale, int digit) {
    unsigned char mask = get_digit_mask(digit);

    float w = 30.0f * scale;
    float h = 54.0f * scale;
    float t = 5.0f * scale;
    float mid_y = y + h * 0.5f;

    glColor4f(0.08f, 0.16f, 0.14f, 0.28f);
    draw_rect_2d(x + t, y, x + w - t, y + t);
    draw_rect_2d(x + w - t, y + t, x + w, mid_y - t * 0.5f);
    draw_rect_2d(x + w - t, mid_y + t * 0.5f, x + w, y + h - t);
    draw_rect_2d(x + t, y + h - t, x + w - t, y + h);
    draw_rect_2d(x, mid_y + t * 0.5f, x + t, y + h - t);
    draw_rect_2d(x, y + t, x + t, mid_y - t * 0.5f);
    draw_rect_2d(x + t, mid_y - t * 0.5f, x + w - t, mid_y + t * 0.5f);

    glColor4f(0.35f, 1.0f, 0.72f, 0.95f);

    if (mask & 0x01) draw_rect_2d(x + t, y, x + w - t, y + t);
    if (mask & 0x02) draw_rect_2d(x + w - t, y + t, x + w, mid_y - t * 0.5f);
    if (mask & 0x04) draw_rect_2d(x + w - t, mid_y + t * 0.5f, x + w, y + h - t);
    if (mask & 0x08) draw_rect_2d(x + t, y + h - t, x + w - t, y + h);
    if (mask & 0x10) draw_rect_2d(x, mid_y + t * 0.5f, x + t, y + h - t);
    if (mask & 0x20) draw_rect_2d(x, y + t, x + t, mid_y - t * 0.5f);
    if (mask & 0x40) draw_rect_2d(x + t, mid_y - t * 0.5f, x + w - t, mid_y + t * 0.5f);
}

static void draw_colon_7seg(float x, float y, float scale) {
    float s = 4.0f * scale;

    glColor4f(0.35f, 1.0f, 0.72f, 0.95f);
    draw_rect_2d(x, y + 16.0f * scale, x + s, y + 16.0f * scale + s);
    draw_rect_2d(x, y + 34.0f * scale, x + s, y + 34.0f * scale + s);
}

static void draw_dot_7seg(float x, float y, float scale) {
    float s = 4.0f * scale;

    glColor4f(0.35f, 1.0f, 0.72f, 0.95f);
    draw_rect_2d(x, y + 50.0f * scale, x + s, y + 50.0f * scale + s);
}

static void draw_time_display(float x, float y, float scale, double time_sec) {
    int total_hundredths = (int)(time_sec * 100.0 + 0.5);
    int minutes = total_hundredths / 6000;
    int seconds = (total_hundredths / 100) % 60;
    int hundredths = total_hundredths % 100;

    int m1 = (minutes / 10) % 10;
    int m2 = minutes % 10;
    int s1 = seconds / 10;
    int s2 = seconds % 10;
    int h1 = hundredths / 10;
    int h2 = hundredths % 10;

    float dx = 0.0f;
    float digit_w = 30.0f * scale;
    float gap = 8.0f * scale;

    draw_digit_7seg(x + dx, y, scale, m1); dx += digit_w + gap;
    draw_digit_7seg(x + dx, y, scale, m2); dx += digit_w + gap;

    draw_colon_7seg(x + dx, y, scale); dx += 10.0f * scale + gap;

    draw_digit_7seg(x + dx, y, scale, s1); dx += digit_w + gap;
    draw_digit_7seg(x + dx, y, scale, s2); dx += digit_w + gap;

    draw_dot_7seg(x + dx, y, scale); dx += 10.0f * scale + gap;

    draw_digit_7seg(x + dx, y, scale, h1); dx += digit_w + gap;
    draw_digit_7seg(x + dx, y, scale, h2);
}

static void draw_victory_overlay(int w, int h, double final_time_sec) {
    float scale = 1.45f;
    float digit_w = 30.0f * scale;
    float gap = 8.0f * scale;
    float colon_w = 10.0f * scale;
    float dot_w = 10.0f * scale;
    float display_w = digit_w * 6.0f + colon_w + dot_w + gap * 7.0f;
    float display_h = 54.0f * scale;

    float padding_x = 34.0f;
    float padding_y = 24.0f;

    float box_w = display_w + padding_x * 2.0f;
    float box_h = display_h + padding_y * 2.0f;

    float x0 = (w - box_w) * 0.5f;
    float y0 = h - box_h - 42.0f;
    float x1 = x0 + box_w;
    float y1 = y0 + box_h;

    float time_x = x0 + (box_w - display_w) * 0.5f;
    float time_y = y0 + (box_h - display_h) * 0.5f;

    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    begin_2d(w, h);

    glColor4f(0.02f, 0.07f, 0.08f, 0.82f);
    draw_rect_2d(x0, y0, x1, y1);

    glColor4f(0.25f, 0.95f, 0.70f, 0.16f);
    draw_rect_2d(x0 + 10.0f, y0 + 10.0f, x1 - 10.0f, y0 + 24.0f);

    glColor4f(0.25f, 0.95f, 0.70f, 0.85f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x0, y0);
    glVertex2f(x1, y0);
    glVertex2f(x1, y1);
    glVertex2f(x0, y1);
    glEnd();

    glColor4f(0.25f, 0.95f, 0.70f, 0.14f);
    glBegin(GL_LINES);
    glVertex2f(x0 + 16.0f, y0 + 32.0f);
    glVertex2f(x1 - 16.0f, y0 + 32.0f);

    glVertex2f(x0 + 16.0f, y1 - 14.0f);
    glVertex2f(x1 - 16.0f, y1 - 14.0f);
    glEnd();

    draw_time_display(time_x, time_y, scale, final_time_sec);

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
    HelpOverlay* mission_overlay = NULL;

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

    if (!help_init(&mission_overlay, "../assets/textures/mission_complete.bmp")) {
        printf("WARNING: mission overlay not loaded (missing ../assets/textures/mission_complete.bmp)\n");
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

                victory_terminal_picked = false;

                if (scene && picked >= 0) {
                    const SceneEntity* picked_entity = scene_get_entity(scene, picked);
                    if (picked_entity && strcmp(picked_entity->id, "victory_core") == 0) {
                        victory_terminal_picked = true;
                    }
                }

                if (!mission_complete && scene && scene_get_dynamic_light_count(scene) >= 3) {
                    victory_ready = true;
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

                if (mission_complete && mission_overlay) {
                    help_draw(mission_overlay, window_w, window_h);
                }

                if (mission_complete) {
                    draw_victory_overlay(window_w, window_h, final_time_sec);
                }
            }

            SDL_GL_SwapWindow(window);
        }
    }

    scene_destroy(scene);
    help_destroy(help);
    help_destroy(mission_overlay);

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}