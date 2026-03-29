#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 700

#define PALETTE_HEIGHT 80
#define PALETTE_COLOR_COUNT 8

#define MAX_LINE_COUNT 200
#define MAX_RECT_COUNT 200

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
    Point start;
    Point end;
    Color color;
} Line;

typedef struct {
    int x;
    int y;
    int w;
    int h;
    Color color;
} FilledRect;

typedef enum {
    MODE_LINE,
    MODE_RECT
} DrawMode;

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
    SDL_Rect border;

    for (i = 0; i < PALETTE_COLOR_COUNT; i++) {
        cell.x = i * cell_width;
        cell.y = 0;
        cell.w = cell_width;
        cell.h = PALETTE_HEIGHT;

        set_render_color(renderer, palette[i]);
        SDL_RenderFillRect(renderer, &cell);

        border = cell;
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_RenderDrawRect(renderer, &border);

        if (palette[i].r == selected_color.r &&
            palette[i].g == selected_color.g &&
            palette[i].b == selected_color.b) {
            SDL_Rect highlight = {cell.x + 4, cell.y + 4, cell.w - 8, cell.h - 8};
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &highlight);
        }
    }
}

void draw_lines(SDL_Renderer* renderer, Line lines[], int line_count)
{
    int i;

    for (i = 0; i < line_count; i++) {
        set_render_color(renderer, lines[i].color);
        SDL_RenderDrawLine(renderer,
                           lines[i].start.x, lines[i].start.y,
                           lines[i].end.x, lines[i].end.y);
    }
}

void draw_filled_rects(SDL_Renderer* renderer, FilledRect rects[], int rect_count)
{
    int i;
    SDL_Rect rect;

    for (i = 0; i < rect_count; i++) {
        rect.x = rects[i].x;
        rect.y = rects[i].y;
        rect.w = rects[i].w;
        rect.h = rects[i].h;

        set_render_color(renderer, rects[i].color);
        SDL_RenderFillRect(renderer, &rect);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &rect);
    }
}

FilledRect make_rect_from_points(Point a, Point b, Color color)
{
    FilledRect rect;
    int min_x = (a.x < b.x) ? a.x : b.x;
    int min_y = (a.y < b.y) ? a.y : b.y;
    int max_x = (a.x > b.x) ? a.x : b.x;
    int max_y = (a.y > b.y) ? a.y : b.y;

    rect.x = min_x;
    rect.y = min_y;
    rect.w = max_x - min_x;
    rect.h = max_y - min_y;
    rect.color = color;

    return rect;
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Event event;

    bool running = true;
    bool first_point_selected = false;

    Point first_point = {0, 0};
    Point mouse_point = {0, 0};

    Line lines[MAX_LINE_COUNT];
    int line_count = 0;

    FilledRect rects[MAX_RECT_COUNT];
    int rect_count = 0;

    DrawMode mode = MODE_LINE;
    Color selected_color = palette[0];

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL init hiba: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Szakaszok es teglalapok",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH,
                              WINDOW_HEIGHT,
                              0);
    if (!window) {
        printf("Ablak letrehozas hiba: %s\n", SDL_GetError());
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
    printf("L - szakasz mod\n");
    printf("R - teglalap mod\n");
    printf("C - minden torlese\n");

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;

                case SDL_MOUSEMOTION:
                    mouse_point.x = event.motion.x;
                    mouse_point.y = event.motion.y;
                    printf("Eger pozicio: (%d, %d)\n", mouse_point.x, mouse_point.y);
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        int mx = event.button.x;
                        int my = event.button.y;
                        Color palette_color;

                        if (get_palette_color_at(mx, my, &palette_color)) {
                            selected_color = palette_color;
                            first_point_selected = false;
                        }
                        else if (my >= PALETTE_HEIGHT) {
                            if (!first_point_selected) {
                                first_point.x = mx;
                                first_point.y = my;
                                first_point_selected = true;
                            }
                            else {
                                Point second_point = {mx, my};

                                if (mode == MODE_LINE) {
                                    if (line_count < MAX_LINE_COUNT) {
                                        lines[line_count].start = first_point;
                                        lines[line_count].end = second_point;
                                        lines[line_count].color = selected_color;
                                        line_count++;
                                    }
                                }
                                else if (mode == MODE_RECT) {
                                    if (rect_count < MAX_RECT_COUNT) {
                                        rects[rect_count] = make_rect_from_points(first_point, second_point, selected_color);
                                        rect_count++;
                                    }
                                }

                                first_point_selected = false;
                            }
                        }
                    }
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_l:
                            mode = MODE_LINE;
                            first_point_selected = false;
                            printf("Mod: szakasz\n");
                            break;

                        case SDLK_r:
                            mode = MODE_RECT;
                            first_point_selected = false;
                            printf("Mod: teglalap\n");
                            break;

                        case SDLK_c:
                            line_count = 0;
                            rect_count = 0;
                            first_point_selected = false;
                            printf("Torles\n");
                            break;
                    }
                    break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        draw_palette(renderer, selected_color);
        draw_filled_rects(renderer, rects, rect_count);
        draw_lines(renderer, lines, line_count);

        if (first_point_selected) {
            set_render_color(renderer, selected_color);

            if (mode == MODE_LINE) {
                SDL_RenderDrawLine(renderer,
                                   first_point.x, first_point.y,
                                   mouse_point.x, mouse_point.y);
            }
            else {
                FilledRect preview = make_rect_from_points(first_point, mouse_point, selected_color);
                SDL_Rect r = {preview.x, preview.y, preview.w, preview.h};
                SDL_RenderDrawRect(renderer, &r);
            }
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}