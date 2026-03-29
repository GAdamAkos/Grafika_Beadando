#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 700

#define PALETTE_HEIGHT 80
#define PALETTE_COLOR_COUNT 8
#define MAX_CIRCLE_COUNT 200

#define PI 3.14159265358979323846

typedef struct {
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
} Color;

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    int x;
    int y;
    int radius;
    Color color;
} Circle;

typedef enum {
    APPROX_BY_STEP_COUNT,
    APPROX_BY_ANGLE,
    APPROX_BY_MAX_SEGMENT
} ApproxMode;

Color palette[PALETTE_COLOR_COUNT] = {
    {255,   0,   0, 255},
    {  0, 255,   0, 255},
    {  0,   0, 255, 255},
    {255, 255,   0, 255},
    {255, 128,   0, 255},
    {255,   0, 255, 255},
    {  0, 255, 255, 255},
    {255, 255, 255, 255}
};

void set_render_color(SDL_Renderer* renderer, Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

bool point_in_rect(int x, int y, SDL_Rect rect)
{
    return (x >= rect.x && x < rect.x + rect.w &&
            y >= rect.y && y < rect.y + rect.h);
}

bool get_palette_color_at(int x, int y, Color* selected_color)
{
    int i;
    int cell_width = WINDOW_WIDTH / PALETTE_COLOR_COUNT;
    SDL_Rect cell;

    for (i = 0; i < PALETTE_COLOR_COUNT; i++) {
        cell.x = i * cell_width;
        cell.y = 0;
        cell.w = cell_width;
        cell.h = PALETTE_HEIGHT;

        if (point_in_rect(x, y, cell)) {
            *selected_color = palette[i];
            return true;
        }
    }

    return false;
}

void draw_palette(SDL_Renderer* renderer, Color selected_color)
{
    int i;
    int cell_width = WINDOW_WIDTH / PALETTE_COLOR_COUNT;
    SDL_Rect cell;

    for (i = 0; i < PALETTE_COLOR_COUNT; i++) {
        cell.x = i * cell_width;
        cell.y = 0;
        cell.w = cell_width;
        cell.h = PALETTE_HEIGHT;

        set_render_color(renderer, palette[i]);
        SDL_RenderFillRect(renderer, &cell);

        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_RenderDrawRect(renderer, &cell);

        if (palette[i].r == selected_color.r &&
            palette[i].g == selected_color.g &&
            palette[i].b == selected_color.b) {
            SDL_Rect highlight = {cell.x + 4, cell.y + 4, cell.w - 8, cell.h - 8};
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &highlight);
        }
    }
}

void draw_line(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, Color color)
{
    set_render_color(renderer, color);
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

int compute_segment_count(Circle circle, ApproxMode mode, int step_count, double angle_step_deg, double max_segment_length)
{
    double circumference = 2.0 * PI * circle.radius;
    int count = 3;

    if (circle.radius <= 0) {
        return 0;
    }

    switch (mode) {
        case APPROX_BY_STEP_COUNT:
            count = step_count;
            break;

        case APPROX_BY_ANGLE:
            count = (int)ceil(360.0 / angle_step_deg);
            break;

        case APPROX_BY_MAX_SEGMENT:
            count = (int)ceil(circumference / max_segment_length);
            break;
    }

    if (count < 3) {
        count = 3;
    }

    return count;
}

void draw_circle_approx(SDL_Renderer* renderer,
                        Circle circle,
                        ApproxMode mode,
                        int step_count,
                        double angle_step_deg,
                        double max_segment_length)
{
    int i;
    int segment_count = compute_segment_count(circle, mode, step_count, angle_step_deg, max_segment_length);

    if (segment_count < 3) {
        return;
    }

    for (i = 0; i < segment_count; i++) {
        double a1 = 2.0 * PI * i / segment_count;
        double a2 = 2.0 * PI * (i + 1) / segment_count;

        int x1 = circle.x + (int)(circle.radius * cos(a1));
        int y1 = circle.y + (int)(circle.radius * sin(a1));
        int x2 = circle.x + (int)(circle.radius * cos(a2));
        int y2 = circle.y + (int)(circle.radius * sin(a2));

        draw_line(renderer, x1, y1, x2, y2, circle.color);
    }
}

bool is_point_inside_circle(int px, int py, Circle circle)
{
    int dx = px - circle.x;
    int dy = py - circle.y;
    return dx * dx + dy * dy <= circle.radius * circle.radius;
}

void draw_plus_mark(SDL_Renderer* renderer, Circle circle)
{
    int size = circle.radius / 2;
    if (size < 8) {
        size = 8;
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawLine(renderer, circle.x - size, circle.y, circle.x + size, circle.y);
    SDL_RenderDrawLine(renderer, circle.x, circle.y - size, circle.x, circle.y + size);
}

int find_circle_at(Circle circles[], int circle_count, int x, int y)
{
    int i;

    for (i = circle_count - 1; i >= 0; i--) {
        if (is_point_inside_circle(x, y, circles[i])) {
            return i;
        }
    }

    return -1;
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Event event;

    Circle circles[MAX_CIRCLE_COUNT];
    int circle_count = 0;

    Color selected_color = palette[0];

    bool running = true;
    bool creating_circle = false;
    bool dragging_circle = false;

    Point center = {0, 0};
    Point mouse = {0, 0};

    int dragged_index = -1;
    int drag_offset_x = 0;
    int drag_offset_y = 0;

    ApproxMode approx_mode = APPROX_BY_STEP_COUNT;
    int step_count = 36;
    double angle_step_deg = 10.0;
    double max_segment_length = 12.0;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL init hiba: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Korok SDL-ben",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH,
                              WINDOW_HEIGHT,
                              0);
    if (!window) {
        printf("Ablak hiba: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("Renderer hiba: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    printf("Billentyuk:\n");
    printf("1 - kozelites fix lepesszammal\n");
    printf("2 - kozelites fix szoglepessel\n");
    printf("3 - kozelites maximalis szakaszhosszal\n");
    printf("C - minden torlese\n");

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;

                case SDL_MOUSEMOTION:
                    mouse.x = event.motion.x;
                    mouse.y = event.motion.y;
                    printf("Eger pozicio: (%d, %d)\n", mouse.x, mouse.y);

                    if (dragging_circle && dragged_index >= 0) {
                        circles[dragged_index].x = mouse.x - drag_offset_x;
                        circles[dragged_index].y = mouse.y - drag_offset_y;
                    }
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        int mx = event.button.x;
                        int my = event.button.y;
                        Color palette_color;
                        int found_index;

                        if (get_palette_color_at(mx, my, &palette_color)) {
                            selected_color = palette_color;
                        }
                        else if (my >= PALETTE_HEIGHT) {
                            found_index = find_circle_at(circles, circle_count, mx, my);

                            if (found_index >= 0) {
                                dragging_circle = true;
                                dragged_index = found_index;
                                drag_offset_x = mx - circles[found_index].x;
                                drag_offset_y = my - circles[found_index].y;
                            }
                            else {
                                creating_circle = true;
                                center.x = mx;
                                center.y = my;
                            }
                        }
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        if (dragging_circle) {
                            dragging_circle = false;
                            dragged_index = -1;
                        }
                        else if (creating_circle) {
                            int dx = event.button.x - center.x;
                            int dy = event.button.y - center.y;
                            int radius = (int)sqrt((double)(dx * dx + dy * dy));

                            if (radius > 0 && circle_count < MAX_CIRCLE_COUNT) {
                                circles[circle_count].x = center.x;
                                circles[circle_count].y = center.y;
                                circles[circle_count].radius = radius;
                                circles[circle_count].color = selected_color;
                                circle_count++;
                            }

                            creating_circle = false;
                        }
                    }
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_1:
                            approx_mode = APPROX_BY_STEP_COUNT;
                            printf("Mod: fix lepesszam\n");
                            break;

                        case SDLK_2:
                            approx_mode = APPROX_BY_ANGLE;
                            printf("Mod: fix szoglepes\n");
                            break;

                        case SDLK_3:
                            approx_mode = APPROX_BY_MAX_SEGMENT;
                            printf("Mod: maximalis szakaszhossz\n");
                            break;

                        case SDLK_c:
                            circle_count = 0;
                            creating_circle = false;
                            dragging_circle = false;
                            dragged_index = -1;
                            printf("Korok torolve\n");
                            break;
                    }
                    break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
        SDL_RenderClear(renderer);

        draw_palette(renderer, selected_color);

        for (int i = 0; i < circle_count; i++) {
            draw_circle_approx(renderer, circles[i], approx_mode, step_count, angle_step_deg, max_segment_length);

            if (is_point_inside_circle(mouse.x, mouse.y, circles[i])) {
                draw_plus_mark(renderer, circles[i]);
            }
        }

        if (creating_circle) {
            int dx = mouse.x - center.x;
            int dy = mouse.y - center.y;
            int preview_radius = (int)sqrt((double)(dx * dx + dy * dy));

            Circle preview = {center.x, center.y, preview_radius, selected_color};
            draw_circle_approx(renderer, preview, approx_mode, step_count, angle_step_deg, max_segment_length);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}