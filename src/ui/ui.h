#ifndef UI_H
#define UI_H

// ui.h
// UI rendering and management.

/**************
 * Structures *
 **************/

struct color_s;
typedef struct color_s color;

/***********
 * Globals *
 ***********/

// The font file:
extern char const * const FONT_FILE;

// The font resolution:
extern int const FONT_RESOLUTION;

// The size of the crosshairs in % of the screen height:
extern float CROSSHAIRS_SIZE;
extern color CROSSHAIRS_COLOR;

// A few colors:
extern color const WHITE;
extern color const BLACK;
extern color const ELF_FOREST_GREEN;
extern color const BRIGHT_RED;
extern color const COOL_BLUE;
extern color const FRESH_CREAM;

extern color const LIGHT_SHADOW;
extern color const DARK_SHADOW;
extern color const LEAF_SHADOW;

// The width/height of a surface that will fill the screen at z=-1 given an
// identity modelview matrix:
extern float OVERLAY_WIDTH;
extern float OVERLAY_HEIGHT;

/*************************
 * Structure Definitions *
 *************************/

struct color_s {
  uint8_t r, g, b, a;
};

/********************
 * Inline Functions *
 ********************/

static inline void set_color(color c) {
  glColor4ub(c.r, c.g, c.b, c.a);
}

/*************
 * Functions *
 *************/

// Sets up the UI module.
void setup_ui(void);

// Cleans up the UI module.
void cleanup_ui(void);

// Renders the user interface (as well as tinting the screen if necessary).
void render_ui(void);

// Renders the given (utf-8) string with the given size and color, at the given
// position relative to the bottom-left corner of the screen.
void render_string(
  char const * const str,
  color text_color,
  float size,
  float left,
  float bot
);

// Calls render_string twice to render a string with a drop shadow.
static inline void render_string_shadow(
  char const * const string,
  color text_color,
  color shadow_color,
  float drop,
  float size,
  float left,
  float bot
) {
  render_string(string, shadow_color, size, left + drop, bot - drop);
  render_string(string, text_color, size, left, bot);
}

#endif // ifndef UI_H
