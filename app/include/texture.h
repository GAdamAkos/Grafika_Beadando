#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

typedef struct Texture {
    GLuint id;
    int width;
    int height;
} Texture;

/**
 * Load a BMP as an OpenGL RGBA texture.
 * Supports optional color-key transparency (e.g., magenta 255,0,255).
 */
bool load_texture_bmp(Texture* out_tex, const char* path, bool use_color_key, Uint8 key_r, Uint8 key_g, Uint8 key_b);

void destroy_texture(Texture* tex);

#endif