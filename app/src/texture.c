#include "texture.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glu.h>


bool load_texture_bmp(Texture* out, const char* path, bool use_color_key, Uint8 r, Uint8 g, Uint8 b) {
    if (!out || !path) return false;

    out->id = 0;
    out->width = 0;
    out->height = 0;

    SDL_Surface* surface = SDL_LoadBMP(path);
    if (!surface) {
        printf("SDL_LoadBMP failed for '%s': %s\n", path, SDL_GetError());
        return false;
    }

    // If color key requested, convert to 32-bit and set alpha=0 where pixel matches key.
    // (Mostly useful for UI sprites; not required for ground, but keeps API consistent.)
    SDL_Surface* upload_surface = surface;

    if (use_color_key) {
        SDL_Surface* converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
        if (converted) {
            Uint32* pixels = (Uint32*)converted->pixels;
            int count = converted->w * converted->h;

            // In ABGR8888, bytes in memory are: A B G R (little endian details aside)
            // We'll extract using SDL_GetRGBA for safety.
            SDL_PixelFormat* fmt = converted->format;

            for (int i = 0; i < count; i++) {
                Uint8 pr, pg, pb, pa;
                SDL_GetRGBA(pixels[i], fmt, &pr, &pg, &pb, &pa);
                if (pr == r && pg == g && pb == b) {
                    // set alpha = 0
                    pixels[i] = SDL_MapRGBA(fmt, pr, pg, pb, 0);
                }
            }

            upload_surface = converted;
        }
    }

    GLuint tex_id = 0;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    // Safe for tightly packed BMP data
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Decide format based on bytes per pixel
    GLenum src_format = GL_BGR;
    GLint internal_format = GL_RGB;

    if (upload_surface->format->BytesPerPixel == 4) {
        // ABGR8888 in memory can be treated as BGRA for OpenGL upload
        src_format = GL_BGRA;
        internal_format = GL_RGBA;
    } else {
        // 24-bit BMP: BGR
        src_format = GL_BGR;
        internal_format = GL_RGB;
    }

    // Build mipmaps using GLU (OpenGL 2.1 friendly)
    // This replaces glTexImage2D + glGenerateMipmap.
    int res = gluBuild2DMipmaps(
        GL_TEXTURE_2D,
        internal_format,
        upload_surface->w,
        upload_surface->h,
        src_format,
        GL_UNSIGNED_BYTE,
        upload_surface->pixels
    );

    if (res != 0) {
        // 0 means success in GLU
        printf("gluBuild2DMipmaps failed for '%s' (code=%d)\n", path, res);
        glDeleteTextures(1, &tex_id);
        if (upload_surface != surface) SDL_FreeSurface(upload_surface);
        SDL_FreeSurface(surface);
        return false;
    }

    // Filtering (IMPORTANT for large tiled ground)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Default wrap (UI images like help want clamp). Ground plane overrides to REPEAT at draw time.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    out->id = tex_id;
    out->width = upload_surface->w;
    out->height = upload_surface->h;

    if (upload_surface != surface) SDL_FreeSurface(upload_surface);
    SDL_FreeSurface(surface);

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