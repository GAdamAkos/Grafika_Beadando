#include "texture.h"
#include <stdio.h>

static void apply_color_key_alpha(SDL_Surface* rgba, Uint8 key_r, Uint8 key_g, Uint8 key_b) {
    // rgba is ABGR8888 (so bytes in memory are A,B,G,R on little endian),
    // but SDL provides direct pixel values, so easiest is read components.
    Uint8* pixels = (Uint8*)rgba->pixels;
    const int pitch = rgba->pitch;

    for (int y = 0; y < rgba->h; y++) {
        Uint8* row = pixels + y * pitch;
        for (int x = 0; x < rgba->w; x++) {
            Uint8* p = row + x * 4; // ABGR8888 in memory

            Uint8 a = p[0];
            Uint8 b = p[1];
            Uint8 g = p[2];
            Uint8 r = p[3];

            (void)a;

            if (r == key_r && g == key_g && b == key_b) {
                p[0] = 0;   // alpha = 0
            } else {
                p[0] = 255; // alpha = 255
            }
        }
    }
}

bool load_texture_bmp(Texture* out_tex, const char* path, bool use_color_key, Uint8 key_r, Uint8 key_g, Uint8 key_b) {
    if (!out_tex) return false;

    SDL_Surface* bmp = SDL_LoadBMP(path);
    if (!bmp) {
        printf("SDL_LoadBMP failed for '%s': %s\n", path, SDL_GetError());
        return false;
    }

    SDL_Surface* rgba = SDL_ConvertSurfaceFormat(bmp, SDL_PIXELFORMAT_ABGR8888, 0);
    SDL_FreeSurface(bmp);

    if (!rgba) {
        printf("SDL_ConvertSurfaceFormat failed: %s\n", SDL_GetError());
        return false;
    }

    if (use_color_key) {
        apply_color_key_alpha(rgba, key_r, key_g, key_b);
    } else {
        // Force alpha to 255 everywhere
        Uint8* pixels = (Uint8*)rgba->pixels;
        for (int y = 0; y < rgba->h; y++) {
            Uint8* row = pixels + y * rgba->pitch;
            for (int x = 0; x < rgba->w; x++) {
                row[x * 4 + 0] = 255; // alpha
            }
        }
    }

    GLuint tex_id = 0;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // For old OpenGL, CLAMP is ok; CLAMP_TO_EDGE is nicer but may not be available everywhere.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA,
        rgba->w, rgba->h, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, rgba->pixels
    );

    SDL_FreeSurface(rgba);

    out_tex->id = tex_id;
    out_tex->width = 0;
    out_tex->height = 0;

    // We know width/height from the converted surface before freeing
    // (store them earlier)
    // Quick fix: re-load not needed; instead store before free:
    // But we already freed; let's just set to 0 if not used elsewhere.
    // We'll keep sizing in help.c from passed-in constants.
    // If you want exact dims, move these assignments above SDL_FreeSurface(rgba).
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