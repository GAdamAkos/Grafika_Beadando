#include "texture.h"
#include <stdio.h>

bool load_texture_bmp(Texture* out_tex, const char* path, bool use_color_key, Uint8 key_r, Uint8 key_g, Uint8 key_b) {
    (void)use_color_key;
    (void)key_r; (void)key_g; (void)key_b;

    if (!out_tex) return false;

    SDL_Surface* bmp = SDL_LoadBMP(path);
    if (!bmp) {
        printf("SDL_LoadBMP failed for '%s': %s\n", path, SDL_GetError());
        return false;
    }

    // Convert to 24-bit BGR (no alpha, very reliable for BMP)
    SDL_Surface* bgr = SDL_ConvertSurfaceFormat(bmp, SDL_PIXELFORMAT_BGR24, 0);
    SDL_FreeSurface(bmp);

    if (!bgr) {
        printf("SDL_ConvertSurfaceFormat failed: %s\n", SDL_GetError());
        return false;
    }

    GLuint tex_id = 0;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Important for 24-bit data: ensure alignment
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB,
        bgr->w, bgr->h, 0,
        GL_BGR, GL_UNSIGNED_BYTE, bgr->pixels
    );

    out_tex->id = tex_id;
    out_tex->width = bgr->w;
    out_tex->height = bgr->h;

    SDL_FreeSurface(bgr);
    return true;
}

void destroy_texture(Texture* tex) {
    if (!tex) return;
    if (tex->id != 0) {
        glDeleteTextures(1, &tex->id);
        tex->id = 0;
    }
    tex->width = 0;
    tex->height = 0;
}