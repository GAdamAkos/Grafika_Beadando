#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <stdbool.h>
#include <string.h>

#include "ui_hud.h"

static void begin_2d(int w, int h)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, w, h, 0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

static void end_2d(void)
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
}

void draw_crosshair(int w, int h)
{
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

static void draw_rect_2d(float x0, float y0, float x1, float y1)
{
    glBegin(GL_QUADS);
    glVertex2f(x0, y0);
    glVertex2f(x1, y0);
    glVertex2f(x1, y1);
    glVertex2f(x0, y1);
    glEnd();
}

static unsigned char get_digit_mask(int digit)
{
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

static void draw_digit_7seg(float x, float y, float scale, int digit)
{
    unsigned char mask = get_digit_mask(digit);

    float w = 30.0f * scale;
    float h = 54.0f * scale;
    float t = 5.0f * scale;
    float mid_y = y + h * 0.5f;

    glColor4f(0.16f, 0.10f, 0.06f, 0.12f);
    draw_rect_2d(x + t, y, x + w - t, y + t);
    draw_rect_2d(x + w - t, y + t, x + w, mid_y - t * 0.5f);
    draw_rect_2d(x + w - t, mid_y + t * 0.5f, x + w, y + h - t);
    draw_rect_2d(x + t, y + h - t, x + w - t, y + h);
    draw_rect_2d(x, mid_y + t * 0.5f, x + t, y + h - t);
    draw_rect_2d(x, y + t, x + t, mid_y - t * 0.5f);
    draw_rect_2d(x + t, mid_y - t * 0.5f, x + w - t, mid_y + t * 0.5f);

    glColor4f(0.95f, 0.68f, 0.28f, 0.82f);

    if (mask & 0x01) draw_rect_2d(x + t, y, x + w - t, y + t);
    if (mask & 0x02) draw_rect_2d(x + w - t, y + t, x + w, mid_y - t * 0.5f);
    if (mask & 0x04) draw_rect_2d(x + w - t, mid_y + t * 0.5f, x + w, y + h - t);
    if (mask & 0x08) draw_rect_2d(x + t, y + h - t, x + w - t, y + h);
    if (mask & 0x10) draw_rect_2d(x, mid_y + t * 0.5f, x + t, y + h - t);
    if (mask & 0x20) draw_rect_2d(x, y + t, x + t, mid_y - t * 0.5f);
    if (mask & 0x40) draw_rect_2d(x + t, mid_y - t * 0.5f, x + w - t, mid_y + t * 0.5f);
}

static void draw_colon_7seg(float x, float y, float scale)
{
    float s = 6.0f * scale;
    glColor4f(0.95f, 0.68f, 0.28f, 0.82f);
    draw_rect_2d(x, y + 16.0f * scale, x + s, y + 22.0f * scale);
    draw_rect_2d(x, y + 34.0f * scale, x + s, y + 40.0f * scale);
}

static void draw_dot_7seg(float x, float y, float scale)
{
    float s = 6.0f * scale;
    glColor4f(0.95f, 0.68f, 0.28f, 0.82f);
    draw_rect_2d(x, y + 44.0f * scale, x + s, y + 50.0f * scale);
}

static void draw_time_display(float x, float y, float scale, double time_sec)
{
    int total_hundredths = (int)(time_sec * 100.0 + 0.5);
    int minutes = total_hundredths / 6000;
    int seconds = (total_hundredths / 100) % 60;
    int hundredths = total_hundredths % 100;

    int m1 = (minutes / 10) % 10;
    int m2 = minutes % 10;
    int s1 = (seconds / 10) % 10;
    int s2 = seconds % 10;
    int h1 = (hundredths / 10) % 10;
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

static bool get_glyph_5x7(char c, unsigned char out[7])
{
    memset(out, 0, 7);

    switch (c) {
        case 'F':
            out[0] = 0x1F; out[1] = 0x10; out[2] = 0x10; out[3] = 0x1E;
            out[4] = 0x10; out[5] = 0x10; out[6] = 0x10; return true;
        case 'H':
            out[0] = 0x11; out[1] = 0x11; out[2] = 0x11; out[3] = 0x1F;
            out[4] = 0x11; out[5] = 0x11; out[6] = 0x11; return true;
        case 'E':
            out[0] = 0x1F; out[1] = 0x10; out[2] = 0x10; out[3] = 0x1E;
            out[4] = 0x10; out[5] = 0x10; out[6] = 0x1F; return true;
        case 'L':
            out[0] = 0x10; out[1] = 0x10; out[2] = 0x10; out[3] = 0x10;
            out[4] = 0x10; out[5] = 0x10; out[6] = 0x1F; return true;
        case 'P':
            out[0] = 0x1E; out[1] = 0x11; out[2] = 0x11; out[3] = 0x1E;
            out[4] = 0x10; out[5] = 0x10; out[6] = 0x10; return true;
        case '1':
            out[0] = 0x04; out[1] = 0x0C; out[2] = 0x04; out[3] = 0x04;
            out[4] = 0x04; out[5] = 0x04; out[6] = 0x0E; return true;
        case ' ':
            return true;
        default:
            return false;
    }
}

static void draw_text_5x7(float x, float y, float scale, const char* text)
{
    float cell = 4.0f * scale;
    float advance = cell * 6.0f;

    for (size_t i = 0; text[i] != '\0'; i++) {
        unsigned char rows[7];
        if (get_glyph_5x7(text[i], rows)) {
            for (int row = 0; row < 7; row++) {
                for (int col = 0; col < 5; col++) {
                    if (rows[row] & (1 << (4 - col))) {
                        float x0 = x + col * cell;
                        float y0 = y + row * cell;
                        draw_rect_2d(x0, y0, x0 + cell, y0 + cell);
                    }
                }
            }
        }
        x += advance;
    }
}

void draw_bottom_hud(int w, int h, double elapsed_time_sec)
{
    float bar_h = 42.0f;
    float pad = 12.0f;
    float time_scale = 0.46f;
    float time_w = (30.0f * time_scale) * 6.0f + (10.0f * time_scale) * 2.0f + (8.0f * time_scale) * 7.0f;
    float time_x = w - time_w - 18.0f;
    float time_y = h - bar_h + 6.0f;

    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    begin_2d(w, h);

    glColor4f(0.10f, 0.08f, 0.07f, 0.42f);
    draw_rect_2d(0.0f, h - bar_h, (float)w, (float)h);

    glColor4f(0.85f, 0.45f, 0.12f, 0.10f);
    draw_rect_2d(0.0f, h - bar_h, (float)w, h - bar_h + 4.0f);

    glColor4f(0.95f, 0.68f, 0.28f, 0.78f);
    draw_text_5x7(pad, h - bar_h + 8.0f, 1.0f, "F1 HELP");

    draw_time_display(time_x, time_y, time_scale, elapsed_time_sec);

    end_2d();
    glPopAttrib();
}

void draw_victory_overlay(int w, int h, double final_time_sec)
{
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
    float y0 = 36.0f;
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

    glColor4f(0.10f, 0.08f, 0.07f, 0.52f);
    draw_rect_2d(x0, y0, x1, y1);

    glColor4f(0.95f, 0.68f, 0.28f, 0.10f);
    draw_rect_2d(x0 + 10.0f, y0 + 10.0f, x1 - 10.0f, y0 + 24.0f);

    glColor4f(0.95f, 0.68f, 0.28f, 0.85f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x0, y0);
    glVertex2f(x1, y0);
    glVertex2f(x1, y1);
    glVertex2f(x0, y1);
    glEnd();

    glColor4f(0.95f, 0.68f, 0.28f, 0.10f);
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