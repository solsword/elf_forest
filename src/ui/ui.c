
// ui.c
// UI rendering and management.

#include <GL/gl.h>

#include <FTGL/ftgl.h>

#include <stdio.h>

#include "tex/tex.h"
#include "data/data.h"
#include "graphics/gfx.h"
#include "graphics/render.h"
#include "shaders/pipeline.h"
#include "prof/ptime.h"
#include "prof/pmem.h"
#include "control/ctl.h"
#include "world/entities.h"
#include "world/blocks.h"
#include "gen/terrain.h" // to draw geoform and other terrain data
#include "math/manifold.h"

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

uint8_t DRAW_DEBUG_INFO = 0;

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

  cell *hc = head_cell(PLAYER);
  tcoords st = { .s=0, .t=0 };
  // Figure out the tint color (and set the fog distance):
  if (hc == NULL) {
    no_tint();
    FOG_DENSITY = AIR_FOG_DENSITY;
  } else if (b_is_opaque(hc->primary)) {
    blind = 1;
    // compute texture coordinates
    compute_dynamic_face_tc(
      hc->primary,
      BD_FACE_BOT,
      &st
    );
    s = st.s / ((float) LAYER_ATLASES[L_OPAQUE]->size);
    t = st.t / ((float) LAYER_ATLASES[L_OPAQUE]->size);
    step_s = 1.0/LAYER_ATLASES[L_OPAQUE]->size;
    step_t = 0.5/LAYER_ATLASES[L_OPAQUE]->size;
    FOG_DENSITY = 1.0;
  } else if (b_same_liquid(hc->primary, b_make_block(B_WATER))) {
    set_tint(0.5, 0.5, 0.9, 1.0);
    FOG_DENSITY = WATER_FOG_DENSITY;
  } else {
    no_tint();
    FOG_DENSITY = AIR_FOG_DENSITY;
  }

  // Bind our texture atlas or set the blend function appropriately:
  if (blind) {
    glBindTexture( GL_TEXTURE_2D, LAYER_ATLASES[L_OPAQUE]->handle );
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

static inline void draw_rates(int *h) {
  // Draw framerate:
  sprintf(
    TXT,
    "framerate :: %.1f",
    FRAMERATE.rate
  );
  render_string_shadow(TXT, COOL_BLUE, LEAF_SHADOW, 1, 17, 590, *h);
  *h -= 25;

  // Draw tick rate:
  sprintf(
    TXT,
    "tick rate :: %.1f",
    TICKRATE.rate
  );
  render_string_shadow(TXT, COOL_BLUE, LEAF_SHADOW, 1, 17, 590, *h);
  *h -= 25;
}

static inline void draw_durations(int *h) {
  // Draw render times:
  sprintf(
    TXT,
    "render ms :: %.2f",
    1000.0 * RENDER_TIME.duration
  );
  render_string_shadow(TXT, COOL_BLUE, LEAF_SHADOW, 1, 17, 500, *h);
  *h -= 25;
  sprintf(
    TXT,
    "render area ms :: %.2f",
    1000.0 * RENDER_AREA_TIME.duration
  );
  render_string_shadow(TXT, COOL_BLUE, LEAF_SHADOW, 1, 17, 500, *h);
  *h -= 25;
  sprintf(
    TXT,
    "render ui ms :: %.2f",
    1000.0 * RENDER_UI_TIME.duration
  );
  render_string_shadow(TXT, COOL_BLUE, LEAF_SHADOW, 1, 17, 500, *h);
  *h -= 25;
  sprintf(
    TXT,
    "render core ms :: %.2f",
    1000.0 * RENDER_CORE_TIME.duration
  );
  render_string_shadow(TXT, COOL_BLUE, LEAF_SHADOW, 1, 17, 500, *h);
  *h -= 25;
  sprintf(
    TXT,
    "render inner ms :: %.6f",
    1000.0 * RENDER_INNER_TIME.duration
  );
  render_string_shadow(TXT, COOL_BLUE, LEAF_SHADOW, 1, 17, 500, *h);
  *h -= 25;

  // Draw compile time:
  sprintf(
    TXT,
    "compile ms :: %.2f",
    1000.0 * COMPILE_TIME.duration
  );
  render_string_shadow(TXT, COOL_BLUE, LEAF_SHADOW, 1, 17, 500, *h);
  *h -= 25;

  // Draw physics time:
  sprintf(
    TXT,
    "physics ms :: %.2f",
    1000.0 * PHYSICS_TIME.duration
  );
  render_string_shadow(TXT, COOL_BLUE, LEAF_SHADOW, 1, 17, 500, *h);
  *h -= 25;

  // Draw data time:
  sprintf(
    TXT,
    "data ms :: %.2f",
    1000.0 * DATA_TIME.duration
  );
  render_string_shadow(TXT, COOL_BLUE, LEAF_SHADOW, 1, 17, 500, *h);
  *h -= 25;

  // Draw terrain generation time:
  sprintf(
    TXT,
    "terrain gen ms :: %.2f",
    1000.0 * TGEN_TIME.duration
  );
  render_string_shadow(TXT, COOL_BLUE, LEAF_SHADOW, 1, 17, 500, *h);
  *h -= 25;
}

static inline void draw_mem(int *h) {
  // First compute memory info:
  compute_chunk_cache_mem();
  // Draw memory info:
  sprintf(
    TXT,
    "chunk cache size :: %.2f MB",
    CHUNK_CACHE_RAM_USAGE.data / (1024.0 * 1024.0)
  );
  render_string_shadow(TXT, COOL_BLUE, LEAF_SHADOW, 1, 17, 490, *h);
  *h -= 25;
  sprintf(
    TXT,
    "chunk cache overhead :: %.2f MB",
    CHUNK_CACHE_RAM_USAGE.overhead / (1024.0 * 1024.0)
  );
  render_string_shadow(TXT, COOL_BLUE, LEAF_SHADOW, 1, 17, 445, *h);
  *h -= 25;
  sprintf(
    TXT,
    "chunk cache GPU usage :: %.2f MB",
    CHUNK_CACHE_GPU_USAGE.data / (1024.0 * 1024.0)
  );
  render_string_shadow(TXT, COOL_BLUE, LEAF_SHADOW, 1, 17, 445, *h);
  *h -= 25;
  sprintf(
    TXT,
    "texture RAM usage :: %.2f MB",
    TEXTURE_RAM_USAGE.data / (1024.0 * 1024.0)
  );
  render_string_shadow(TXT, COOL_BLUE, LEAF_SHADOW, 1, 17, 445, *h);
  *h -= 25;
  sprintf(
    TXT,
    "texture RAM overhead :: %.2f MB",
    TEXTURE_RAM_USAGE.overhead / (1024.0 * 1024.0)
  );
  render_string_shadow(TXT, COOL_BLUE, LEAF_SHADOW, 1, 17, 445, *h);
  *h -= 25;
  sprintf(
    TXT,
    "texture GPU usage :: %.2f MB",
    TEXTURE_GPU_USAGE.data / (1024.0 * 1024.0)
  );
  render_string_shadow(TXT, COOL_BLUE, LEAF_SHADOW, 1, 17, 445, *h);
  *h -= 25;
}

static inline void draw_pos_info(int *h) {
  // Gather position info:
  world_map_pos wmpos;
  global_pos player_pos;
  get_head_glpos(PLAYER, &player_pos);
  glpos__wmpos(&player_pos, &wmpos);

  // Draw geoform data:
  /*
  terrain_region region;
  float tr_interp;
  geoform_info(&player_pos, &region, &tr_interp);
  sprintf(
    TXT,
    "%s <- %.2f -> %s",
    TR_REGION_NAMES[region], tr_interp, TR_REGION_NAMES[region+1]
  );
  render_string_shadow(TXT, FRESH_CREAM, LEAF_SHADOW, 1, 20, 30, *h);
  *h -= 30;
  */

  // Draw fractional height:
  /*
  manifold_point dontcare, th;
  compute_terrain_height(&player_pos, &dontcare, &dontcare, &th);
  sprintf(
    TXT,
    "h: %.4f",
    player_pos.z / th.z
  );
  render_string_shadow(TXT, FRESH_CREAM, LEAF_SHADOW, 1, 20, 30, *h);
  *h -= 30;
  */

  // DEBUG:
  if (PLAYER->area != NULL) {
    sprintf(
      TXT,
      "area origin :: %+6ld x   %+6ld y   %+6ld z",
      PLAYER->area->origin.x, PLAYER->area->origin.y, PLAYER->area->origin.z
    );
  } else {
    sprintf(TXT, "Player is out-of-bounds.");
  }
  render_string_shadow(TXT, FRESH_CREAM, LEAF_SHADOW, 1, 20, 30, *h);
  *h -= 30;

  // Draw region position:
  sprintf(
    TXT,
    "region :: %+6ld x    %+6ld y    %+6ld z",
    player_pos.x, player_pos.y, player_pos.z
  );
  render_string_shadow(TXT, FRESH_CREAM, LEAF_SHADOW, 1, 20, 30, *h);
  *h -= 30;

  // Draw active entity area position:
  sprintf(
    TXT,
    "area :: %0.1f x    %0.1f y    %0.1f z",
    PLAYER->pos.x, PLAYER->pos.y, PLAYER->pos.z
  );
  render_string_shadow(TXT, FRESH_CREAM, LEAF_SHADOW, 1, 20, 30, *h);
  *h -= 30;

  // Draw chunk cache loading:
  sprintf(
    TXT,
    "chunk cache :: %0.3f // %0.3f",
    m_utilization(CHUNK_CACHE->levels[LOD_BASE]),
    m_crowding(CHUNK_CACHE->levels[LOD_BASE])
  );
  render_string_shadow(TXT, FRESH_CREAM, LEAF_SHADOW, 1, 20, 30, *h);
  *h -= 30;
}

static inline void draw_perf_info(int *h) {
  sprintf(
    TXT,
    "chunk layers rendered :: %d",
    CHUNK_LAYERS_RENDERED.average
  );
  render_string_shadow(TXT, FRESH_CREAM, LEAF_SHADOW, 1, 20, 30, *h);
  *h -= 30;

  sprintf(
    TXT,
    "chunks loaded :: %d",
    CHUNKS_LOADED.average
  );
  render_string_shadow(TXT, FRESH_CREAM, LEAF_SHADOW, 1, 20, 30, *h);
  *h -= 30;

  sprintf(
    TXT,
    "chunks compiled :: %d",
    CHUNKS_COMPILED.average
  );
  render_string_shadow(TXT, FRESH_CREAM, LEAF_SHADOW, 1, 20, 30, *h);
  *h -= 30;
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
  int h;
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
  use_pipeline(&RAW_PIPELINE);
  draw_hud();

  use_pipeline(&TEXT_PIPELINE);

  // Draw an indicator if the game is paused:
  draw_paused();

  // Watermark:
  draw_watermark();

  // Debugging info:
  h = 570;
  if (DRAW_DEBUG_INFO) {
    draw_rates(&h);
#ifdef PROFILE_TIME
    draw_durations(&h);
#endif
#ifdef PROFILE_MEM
    draw_mem(&h);
#endif
    h = 570;
    draw_pos_info(&h);
    draw_perf_info(&h);
  }

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
