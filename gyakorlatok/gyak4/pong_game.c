#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 700

#define PADDLE_WIDTH 20
#define PADDLE_HEIGHT 140
#define PADDLE_SPEED 8

#define BALL_MIN_RADIUS 8
#define BALL_MAX_RADIUS 40
#define BALL_DEFAULT_RADIUS 18

#define BALL_SPEED_X 7.0f
#define BALL_SPEED_Y 4.0f

#define PI 3.14159265358979323846

typedef struct {
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
} Color;

typedef struct {
    float x;
    float y;
    int w;
    int h;
    Color color;
} Paddle;

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    int radius;
    float angle;
    float angular_velocity;
    Color color;
} Ball;

typedef struct {
    int left;
    int right;
} Score;

void set_render_color(SDL_Renderer* renderer, Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

void draw_filled_circle(SDL_Renderer* renderer, int cx, int cy, int radius, Color color)
{
    int dx, dy;

    set_render_color(renderer, color);

    for (dy = -radius; dy <= radius; dy++) {
        for (dx = -radius; dx <= radius; dx++) {
            if (dx * dx + dy * dy <= radius * radius) {
                SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
            }
        }
    }
}

void draw_ball_rotation_mark(SDL_Renderer* renderer, Ball ball)
{
    int x2 = (int)(ball.x + cos(ball.angle) * ball.radius * 0.9f);
    int y2 = (int)(ball.y + sin(ball.angle) * ball.radius * 0.9f);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawLine(renderer, (int)ball.x, (int)ball.y, x2, y2);
}

void draw_paddle(SDL_Renderer* renderer, Paddle paddle)
{
    SDL_Rect rect;
    rect.x = (int)paddle.x;
    rect.y = (int)paddle.y;
    rect.w = paddle.w;
    rect.h = paddle.h;

    set_render_color(renderer, paddle.color);
    SDL_RenderFillRect(renderer, &rect);
}

void draw_center_line(SDL_Renderer* renderer)
{
    int y;
    SDL_Rect dash;

    SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);

    dash.x = WINDOW_WIDTH / 2 - 3;
    dash.w = 6;
    dash.h = 20;

    for (y = 10; y < WINDOW_HEIGHT; y += 35) {
        dash.y = y;
        SDL_RenderFillRect(renderer, &dash);
    }
}

void reset_ball(Ball* ball, int direction)
{
    if (ball == NULL) {
        return;
    }

    ball->x = WINDOW_WIDTH / 2.0f;
    ball->y = WINDOW_HEIGHT / 2.0f;
    ball->vx = BALL_SPEED_X * direction;
    ball->vy = BALL_SPEED_Y;
    ball->angle = 0.0f;
    ball->angular_velocity = 0.08f * direction;
}

bool circle_intersects_paddle(Ball ball, Paddle paddle)
{
    float closest_x;
    float closest_y;
    float dx;
    float dy;

    if (ball.x < paddle.x) {
        closest_x = paddle.x;
    }
    else if (ball.x > paddle.x + paddle.w) {
        closest_x = paddle.x + paddle.w;
    }
    else {
        closest_x = ball.x;
    }

    if (ball.y < paddle.y) {
        closest_y = paddle.y;
    }
    else if (ball.y > paddle.y + paddle.h) {
        closest_y = paddle.y + paddle.h;
    }
    else {
        closest_y = ball.y;
    }

    dx = ball.x - closest_x;
    dy = ball.y - closest_y;

    return (dx * dx + dy * dy) <= (float)(ball.radius * ball.radius);
}

void keep_paddle_in_window(Paddle* paddle)
{
    if (paddle->y < 0) {
        paddle->y = 0;
    }

    if (paddle->y + paddle->h > WINDOW_HEIGHT) {
        paddle->y = WINDOW_HEIGHT - paddle->h;
    }
}

void draw_segment(SDL_Renderer* renderer, int x, int y, int w, int h)
{
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(renderer, &rect);
}

void draw_digit(SDL_Renderer* renderer, int digit, int x, int y, int scale, Color color)
{
    bool a = false, b = false, c = false, d = false, e = false, f = false, g = false;
    int t = scale;
    int len = scale * 4;

    switch (digit) {
        case 0: a = b = c = d = e = f = true; break;
        case 1: b = c = true; break;
        case 2: a = b = d = e = g = true; break;
        case 3: a = b = c = d = g = true; break;
        case 4: b = c = f = g = true; break;
        case 5: a = c = d = f = g = true; break;
        case 6: a = c = d = e = f = g = true; break;
        case 7: a = b = c = true; break;
        case 8: a = b = c = d = e = f = g = true; break;
        case 9: a = b = c = d = f = g = true; break;
        default: return;
    }

    set_render_color(renderer, color);

    if (a) draw_segment(renderer, x + t,         y,             len, t);
    if (b) draw_segment(renderer, x + t + len,   y + t,         t,   len);
    if (c) draw_segment(renderer, x + t + len,   y + 2*t + len, t,   len);
    if (d) draw_segment(renderer, x + t,         y + 2*(t+len), len, t);
    if (e) draw_segment(renderer, x,             y + 2*t + len, t,   len);
    if (f) draw_segment(renderer, x,             y + t,         t,   len);
    if (g) draw_segment(renderer, x + t,         y + t + len,   len, t);
}

void draw_score(SDL_Renderer* renderer, Score score)
{
    Color score_color = {255, 255, 255, 255};

    draw_digit(renderer, score.left / 10,  WINDOW_WIDTH / 2 - 140, 30, 6, score_color);
    draw_digit(renderer, score.left % 10,  WINDOW_WIDTH / 2 - 90,  30, 6, score_color);

    draw_digit(renderer, score.right / 10, WINDOW_WIDTH / 2 + 40,  30, 6, score_color);
    draw_digit(renderer, score.right % 10, WINDOW_WIDTH / 2 + 90,  30, 6, score_color);
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Event event;

    bool running = true;

    bool w_down = false;
    bool s_down = false;
    bool up_down = false;
    bool down_down = false;

    Paddle left_paddle;
    Paddle right_paddle;
    Ball ball;
    Score score = {0, 0};

    Color bg = {20, 20, 24, 255};
    Color paddle_color = {220, 220, 220, 255};
    Color ball_color = {255, 120, 0, 255};

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL init hiba: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Pong",
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

    left_paddle.x = 30.0f;
    left_paddle.y = WINDOW_HEIGHT / 2.0f - PADDLE_HEIGHT / 2.0f;
    left_paddle.w = PADDLE_WIDTH;
    left_paddle.h = PADDLE_HEIGHT;
    left_paddle.color = paddle_color;

    right_paddle.x = WINDOW_WIDTH - 30.0f - PADDLE_WIDTH;
    right_paddle.y = WINDOW_HEIGHT / 2.0f - PADDLE_HEIGHT / 2.0f;
    right_paddle.w = PADDLE_WIDTH;
    right_paddle.h = PADDLE_HEIGHT;
    right_paddle.color = paddle_color;

    ball.x = WINDOW_WIDTH / 2.0f;
    ball.y = WINDOW_HEIGHT / 2.0f;
    ball.vx = BALL_SPEED_X;
    ball.vy = BALL_SPEED_Y;
    ball.radius = BALL_DEFAULT_RADIUS;
    ball.angle = 0.0f;
    ball.angular_velocity = 0.08f;
    ball.color = ball_color;

    printf("Vezerles:\n");
    printf("Bal uto: W / S\n");
    printf("Jobb uto: FEL / LE\n");
    printf("Labda athelyezese: bal egérgomb\n");
    printf("Labda merete: Q kisebb, E nagyobb\n");
    printf("Ujrainditas: R\n");
    printf("Pontok nullazasa: C\n");

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_w: w_down = true; break;
                        case SDLK_s: s_down = true; break;
                        case SDLK_UP: up_down = true; break;
                        case SDLK_DOWN: down_down = true; break;

                        case SDLK_q:
                            if (ball.radius > BALL_MIN_RADIUS) {
                                ball.radius -= 2;
                            }
                            break;

                        case SDLK_e:
                            if (ball.radius < BALL_MAX_RADIUS) {
                                ball.radius += 2;
                            }
                            break;

                        case SDLK_r:
                            reset_ball(&ball, (ball.vx >= 0) ? 1 : -1);
                            break;

                        case SDLK_c:
                            score.left = 0;
                            score.right = 0;
                            reset_ball(&ball, 1);
                            break;
                    }
                    break;

                case SDL_KEYUP:
                    switch (event.key.keysym.sym) {
                        case SDLK_w: w_down = false; break;
                        case SDLK_s: s_down = false; break;
                        case SDLK_UP: up_down = false; break;
                        case SDLK_DOWN: down_down = false; break;
                    }
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        ball.x = (float)event.button.x;
                        ball.y = (float)event.button.y;

                        if (ball.x < ball.radius) {
                            ball.x = ball.radius;
                        }
                        if (ball.x > WINDOW_WIDTH - ball.radius) {
                            ball.x = WINDOW_WIDTH - ball.radius;
                        }
                        if (ball.y < ball.radius) {
                            ball.y = ball.radius;
                        }
                        if (ball.y > WINDOW_HEIGHT - ball.radius) {
                            ball.y = WINDOW_HEIGHT - ball.radius;
                        }
                    }
                    break;
            }
        }

        if (w_down) {
            left_paddle.y -= PADDLE_SPEED;
        }
        if (s_down) {
            left_paddle.y += PADDLE_SPEED;
        }
        if (up_down) {
            right_paddle.y -= PADDLE_SPEED;
        }
        if (down_down) {
            right_paddle.y += PADDLE_SPEED;
        }

        keep_paddle_in_window(&left_paddle);
        keep_paddle_in_window(&right_paddle);

        ball.x += ball.vx;
        ball.y += ball.vy;
        ball.angle += ball.angular_velocity;

        if (ball.y - ball.radius <= 0) {
            ball.y = ball.radius;
            ball.vy = -ball.vy;
            ball.angular_velocity += 0.01f;
        }

        if (ball.y + ball.radius >= WINDOW_HEIGHT) {
            ball.y = WINDOW_HEIGHT - ball.radius;
            ball.vy = -ball.vy;
            ball.angular_velocity -= 0.01f;
        }

        if (ball.vx < 0 && circle_intersects_paddle(ball, left_paddle)) {
            float paddle_center = left_paddle.y + left_paddle.h / 2.0f;
            float offset = (ball.y - paddle_center) / (left_paddle.h / 2.0f);

            ball.x = left_paddle.x + left_paddle.w + ball.radius;
            ball.vx = fabs(ball.vx);
            ball.vy += offset * 2.5f;
            ball.angular_velocity = -0.08f - offset * 0.08f;
        }

        if (ball.vx > 0 && circle_intersects_paddle(ball, right_paddle)) {
            float paddle_center = right_paddle.y + right_paddle.h / 2.0f;
            float offset = (ball.y - paddle_center) / (right_paddle.h / 2.0f);

            ball.x = right_paddle.x - ball.radius;
            ball.vx = -fabs(ball.vx);
            ball.vy += offset * 2.5f;
            ball.angular_velocity = 0.08f + offset * 0.08f;
        }

        if (ball.x + ball.radius < 0) {
            score.right++;
            reset_ball(&ball, -1);
        }

        if (ball.x - ball.radius > WINDOW_WIDTH) {
            score.left++;
            reset_ball(&ball, 1);
        }

        SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
        SDL_RenderClear(renderer);

        draw_center_line(renderer);
        draw_score(renderer, score);

        draw_paddle(renderer, left_paddle);
        draw_paddle(renderer, right_paddle);

        draw_filled_circle(renderer, (int)ball.x, (int)ball.y, ball.radius, ball.color);
        draw_ball_rotation_mark(renderer, ball);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}