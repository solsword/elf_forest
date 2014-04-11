
// ui.c
// UI rendering and management.

#include <GL/gl.h>

#include <FTGL/ftgl.h>

#include <stdio.h>

#include "graphics/tex.h"
#include "graphics/gfx.h"
#include "graphics/render.h"
#include "prof/ptime.h"
#include "prof/pmem.h"
#include "control/ctl.h"
#include "world/entities.h"
#include "gen/terrain.h" // to draw geoform and other terrain data

#include "ui/ui.h"

/*******************
 * Local Variables *
 *******************/

FTGLfont *FONT = NULL;

char TXT[2048];

/***********
 * Globals *
 ***********/

char const * const FONT_FILE = "res/VerilySerifMono.otf";

int const FONT_RESOLUTION = 72;

float CROSSHAIRS_SIZE = 1.5;
color CROSSHAIRS_COLOR;

color const WHITE = { .r=255, .g=255, .b=255, .a=255 };
color const BLACK = { .r=0, .g=0, .b=0, .a=255 };
color const ELF_FOREST_GREEN = { .r=17, .g=91, .b=27, .a=255 };
color const BRIGHT_RED = { .r=255, .g=45, .b=35, .a=255 };
color const SUN_YELLOW = { .r=255, .g=255, .b=96, .a=255 };
color const COOL_BLUE = { .r=80, .g=95, .b=255, .a=255 };
color const FRESH_CREAM = { .r=250, .g=255, .b=215, .a=255 };

color const LIGHT_SHADOW = { .r=112, .g=112, .b=112, .a=144 };
color const DARK_SHADOW = { .r=56, .g=56, .b=56, .a=160 };
color const LEAF_SHADOW = { .r=32, .g=45, .b=41, .a=170 };

float OVERLAY_WIDTH = 1.0;
float OVERLAY_HEIGHT = 1.0;

/********************
 * Inline Functions *
 ********************/

static inline void compute_overlay_size(void) {
  OVERLAY_HEIGHT = 2 * tanf(FOV*0.5);
  OVERLAY_WIDTH = OVERLAY_HEIGHT * ASPECT;
}

static inline void render_vision_effects() {
  int blind = 0; // whether our head is inside an opaque block or not
  float s = 0, t = 0, step_s = 1, step_t = 1; // texture coords

  block hb = head_block(PLAYER);
  tcoords st = { .s=0, .t=0 };
  // Figure out the tint color (and set the fog distance):
  if (b_is_opaque(hb)) {
    blind = 1;
    // compute texture coordinates
    compute_face_tc(hb, BD_FACE_BOT, &st);
    s = st.s / ((float) BLOCK_ATLAS_WIDTH);
    t = st.t / ((float) BLOCK_ATLAS_HEIGHT);
    step_s = 1.0/BLOCK_ATLAS_WIDTH;
    step_t = 0.5/BLOCK_ATLAS_WIDTH;
    FOG_DENSITY = 1.0;
  } else if (b_shares_translucency(hb, B_WATER)) {
    set_tint(0.5, 0.5, 0.9, 1.0);
    FOG_DENSITY = WATER_FOG_DENSITY;
  } else {
    no_tint();
    FOG_DENSITY = AIR_FOG_DENSITY;
  }

  // Bind our texture atlas or set the blend function appropriately:
  if (blind) {
    glBindTexture( GL_TEXTURE_2D, BLOCK_ATLAS );
  } else {
    glBlendFunc( GL_ZERO, GL_CONSTANT_COLOR );
  }

  // Draw the overlay plane:
  glBegin( GL_TRIANGLES );

  glTexCoord2f(s         , t + step_t);
  glVertex3f(-OVERLAY_WIDTH / 2, -OVERLAY_HEIGHT / 2, -1.0);

  glTexCoord2f(s         , t);
  glVertex3f(-OVERLAY_WIDTH / 2,  OVERLAY_HEIGHT / 2, -1.0);

  glTexCoord2f(s + step_s, t);
  glVertex3f( OVERLAY_WIDTH / 2,  OVERLAY_HEIGHT / 2, -1.0);
                             
  glTexCoord2f(s         , t + step_t);
  glVertex3f(-OVERLAY_WIDTH / 2, -OVERLAY_HEIGHT / 2, -1.0);

  glTexCoord2f(s + step_s, t);
  glVertex3f( OVERLAY_WIDTH / 2,  OVERLAY_HEIGHT / 2, -1.0);

  glTexCoord2f(s + step_s, t + step_t);
  glVertex3f( OVERLAY_WIDTH / 2, -OVERLAY_HEIGHT / 2, -1.0);
 
  glEnd();

  // Release our texture or reset our blending function:
  if (blind) {
    glBindTexture( GL_TEXTURE_2D, 0 );
  } else {
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  }
  no_tint();
}

static inline void draw_hud(void) {
  // Crosshairs:
  set_color(CROSSHAIRS_COLOR);
  float chsize = CROSSHAIRS_SIZE*OVERLAY_HEIGHT/200.0;
  glBegin( GL_LINES );
  glVertex3f( -chsize, 0, -1);
  glVertex3f( chsize, 0, -1);
  glVertex3f( 0, -chsize, -1);
  glVertex3f( 0, chsize, -1);
  glEnd();
}

static inline void draw_paused(void) {
  if (PAUSED) {
    render_string(
      "PAUSED",
      WHITE,
      20,
      368, 270
    );
  }
}

static inline void draw_watermark(void) {
  render_string(
    "Elf Forest",
    WHITE,
    45,
    -2, 0
  );
  render_string(
    "<alpha test>",
    WHITE,
    22,
    620, 15
  );
}

static inline void draw_rates(void) {
  // Draw framerate:
  sprintf(
    TXT,
    "framerate :: %.1f",
    FRAMERATE.rate
  );
  render_string_shadow(TXT, COOL_BLUE, LEAF_SHADOW, 1, 17, 590, 570);

  // Draw tick rate:
  sprintf(
    TXT,
    "tick rate :: %.1f",
    TICKRATE.rate
  );
  render_string_shadow(TXT, COOL_BLUE, LEAF_SHADOW, 1, 17, 590, 545);
}

static inline void draw_mem(void) {
  // First compute memory info:
  compute_chunk_cache_mem();
  // Draw memory info:
  sprintf(
    TXT,
    "chunk cache size :: %.2f MB",
    CHUNK_CACHE_RAM_USAGE.data / (1024.0 * 1024.0)
  );
  render_string_shadow(TXT, COOL_BLUE, LEAF_SHADOW, 1, 17, 490, 520);
  sprintf(
    TXT,
    "chunk cache overhead :: %.2f MB",
    CHUNK_CACHE_RAM_USAGE.overhead / (1024.0 * 1024.0)
  );
  render_string_shadow(TXT, COOL_BLUE, LEAF_SHADOW, 1, 17, 445, 495);
}

static inline void draw_pos_info(void) {
  // Gather position info:
  float depths = 0, oceans = 0, plains = 0, hills = 0, mountains = 0;
  region_pos player_pos;
  get_head_rpos(PLAYER, &player_pos);
  get_geoforms(
    fastfloor(player_pos.x), fastfloor(player_pos.y),
    &depths, &oceans, &plains, &hills, &mountains
  );

  // Draw geoform data:
  sprintf(
    TXT,
    "d: %0.2f  o: %0.2f  p: %0.2f  h: %0.2f  m: %0.2f",
    depths, oceans, plains, hills, mountains
  );
  render_string_shadow(TXT, FRESH_CREAM, LEAF_SHADOW, 1, 20, 30, 570);

  // Draw region position:
  sprintf(
    TXT,
    "region :: %+4ld x    %+4ld y    %+4ld z",
    player_pos.x, player_pos.y, player_pos.z
  );
  render_string_shadow(TXT, FRESH_CREAM, LEAF_SHADOW, 1, 20, 30, 540);

  // Draw active entity area position:
  sprintf(
    TXT,
    "area :: %0.1f x    %0.1f y    %0.1f z",
    PLAYER->pos.x, PLAYER->pos.y, PLAYER->pos.z
  );
  render_string_shadow(TXT, FRESH_CREAM, LEAF_SHADOW, 1, 20, 30, 510);

}

static inline void draw_perf_info(void) {
  sprintf(
    TXT,
    "chunk layers rendered :: %d ",
    CHUNK_LAYERS_RENDERED.average
  );
  render_string_shadow(TXT, FRESH_CREAM, LEAF_SHADOW, 1, 20, 30, 480);

  sprintf(
    TXT,
    "chunks loaded :: %d chunks compiled :: %d",
    CHUNKS_LOADED.average, CHUNKS_COMPILED.average
  );
  render_string_shadow(TXT, FRESH_CREAM, LEAF_SHADOW, 1, 20, 30, 450);
}

/*************
 * Functions *
 *************/

void setup_ui(void) {
  CROSSHAIRS_COLOR = DARK_SHADOW;
  // Load our font:
  FONT = ftglCreateTextureFont(FONT_FILE);
  if (!FONT) {
    fprintf(stderr, "Failed to load font '%s'.\n", FONT_FILE);
    exit(-1);
  }
  ftglSetFontFaceSize(FONT, FONT_RESOLUTION, FONT_RESOLUTION);
}

void cleanup_ui(void) {
  ftglDestroyFont(FONT);
}

void render_ui(void) {
  // Update the overlay plane size:
  compute_overlay_size();

  // Disable depth testing and load an identity matrix:
  glDisable( GL_DEPTH_TEST );
  glDisable( GL_CULL_FACE );
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glLoadIdentity();

  // Tinting/blinding:
  render_vision_effects();

  // HUD:
  draw_hud();

  // Draw an indicator if the game is paused:
  draw_paused();

  // Watermark:
  draw_watermark();

  // Debugging info:
  draw_rates();
  draw_mem();
  draw_pos_info();
  draw_perf_info();

  // Reenable depth testing and face culling and pop back to the previous
  // model view matrix state:
  glPopMatrix();
  glEnable( GL_CULL_FACE );
  glEnable( GL_DEPTH_TEST );
}

void render_string(
  char const * const string,
  color text_color,
  float size,
  float left,
  float bot
) {
  set_color(text_color);

  glDisable( GL_DEPTH_TEST );
  glDisable( GL_CULL_FACE );
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glLoadIdentity();
  glAlphaFunc(GL_ALWAYS, 0); // Turn on full alpha blending

  glTranslatef(
    (left / WINDOW_WIDTH) * OVERLAY_WIDTH - (OVERLAY_WIDTH / 2),
    (bot / WINDOW_HEIGHT) * OVERLAY_HEIGHT - (OVERLAY_HEIGHT / 2),
    -1
  );
  float scale = ((size / WINDOW_HEIGHT) * OVERLAY_HEIGHT) / FONT_RESOLUTION;
  glScalef(scale, scale, 0);

  ftglRenderFont(FONT, string, FTGL_RENDER_ALL);
  glBindTexture( GL_TEXTURE_2D, 0 ); // FTGL doesn't unbind the texture...

  glAlphaFunc(GL_GREATER, 0.5); // Back to binary alpha for transparency
  glPopMatrix();
  glEnable( GL_CULL_FACE );
  glEnable( GL_DEPTH_TEST );
}
