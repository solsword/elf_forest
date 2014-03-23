#ifndef UI_H
#define UI_H

// ui.h
// UI rendering and management.

/***********
 * Globals *
 ***********/

// The font file:
extern const char *FONT_FILE;

// The font resolution:
extern const int FONT_RESOLUTION;

// The width/height of a surface that will fill the screen at z=-1 given an
// identity modelview matrix:
extern float OVERLAY_WIDTH;
extern float OVERLAY_HEIGHT;

/*************
 * Functions *
 *************/

// Sets up the UI module.
void setup_ui(void);

// Cleans up the UI module.
void cleanup_ui(void);

// Renders the user interface (as well as tinting the screen if necessary).
void render_ui(void);

// Renders the given (utf-8) string.
void render_string(const char *str, float size, float left, float top);

#endif // ifndef UI_H
