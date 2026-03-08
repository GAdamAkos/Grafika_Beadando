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
 * Load a BMP texture with optional color keying.
 * - path: relative/absolute path to BMP
 * - use_color_key: if true, pixels matching (r,g,b) become fully transparent (only for 32-bit upload)
 * - r,g,b: color key target
 */
bool load_texture_bmp(Texture* out, const char* path, bool use_color_key, Uint8 r, Uint8 g, Uint8 b);

void destroy_texture(Texture* tex);

#endif