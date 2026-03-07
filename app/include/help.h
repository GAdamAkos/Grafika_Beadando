#ifndef HELP_H
#define HELP_H

#include <stdbool.h>

typedef struct HelpOverlay HelpOverlay;

/**
 * Create and load help overlay texture from BMP.
 * The file is expected at e.g. ../assets/textures/help.bmp (run from app/).
 */
bool help_init(HelpOverlay** out_help, const char* bmp_path);

/** Draw help overlay on top of the scene. */
void help_draw(HelpOverlay* help, int window_w, int window_h);

/** Free resources. */
void help_destroy(HelpOverlay* help);

#endif