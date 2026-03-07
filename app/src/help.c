#include "help.h"
#include "texture.h"

#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <stdlib.h>

struct HelpOverlay {
    Texture tex;
    int img_w;
    int img_h;
};

static void begin_2d(int window_w, int window_h) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, window_w, window_h, 0);

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

bool help_init(HelpOverlay** out_help, const char* bmp_path) {
    if (!out_help) return false;

    HelpOverlay* h = (HelpOverlay*)calloc(1, sizeof(HelpOverlay));
    if (!h) return false;

    // If your help.bmp uses magenta background for transparency:
    // set use_color_key=true and key=(255,0,255).
    // If not, set use_color_key=false.
    bool ok = load_texture_bmp(&h->tex, bmp_path, false, 255, 0, 255);
    if (!ok) {
        free(h);
        return false;
    }

    // You can set these manually to the BMP dimensions you created (recommended).
    // This helps aspect-ratio scaling.
    // Example: 1280x720 or 1024x768 etc.
    h->img_w = 1536;
    h->img_h = 1024;

    *out_help = h;
    return true;
}

void help_draw(HelpOverlay* help, int window_w, int window_h) {
    if (!help || help->tex.id == 0) return;

    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    begin_2d(window_w, window_h);

    // Darken background a bit (so the help is readable)
    glDisable(GL_TEXTURE_2D);
    glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
    glBegin(GL_QUADS);
    glVertex2f(0.f, 0.f);
    glVertex2f((float)window_w, 0.f);
    glVertex2f((float)window_w, (float)window_h);
    glVertex2f(0.f, (float)window_h);
    glEnd();

    // Compute scaled size while keeping aspect ratio
    float pad = 40.0f;
    float max_w = (float)window_w - pad * 2.0f;
    float max_h = (float)window_h - pad * 2.0f;

    float img_w = (float)help->img_w;
    float img_h = (float)help->img_h;

    float scale_w = max_w / img_w;
    float scale_h = max_h / img_h;
    float scale = (scale_w < scale_h) ? scale_w : scale_h;

    float draw_w = img_w * scale;
    float draw_h = img_h * scale;

    float x = ((float)window_w - draw_w) * 0.5f;
    float y = ((float)window_h - draw_h) * 0.5f;

    // Draw textured quad
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, help->tex.id);

    glColor4f(1.f, 1.f, 1.f, 1.f);

    glBegin(GL_QUADS);
    glTexCoord2f(0.f, 0.f); glVertex2f(x,         y);
    glTexCoord2f(1.f, 0.f); glVertex2f(x + draw_w, y);
    glTexCoord2f(1.f, 1.f); glVertex2f(x + draw_w, y + draw_h);
    glTexCoord2f(0.f, 1.f); glVertex2f(x,         y + draw_h);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    end_2d();

    glPopAttrib();
}

void help_destroy(HelpOverlay* help) {
    if (!help) return;
    destroy_texture(&help->tex);
    free(help);
}