
// ui.c
// UI rendering and management.

#include <GL/gl.h>

#include <FTGL/ftgl.h>

#include <stdio.h>

#include "tex.h"
#include "ctl.h"
#include "gfx.h"
#include "render.h"
#include "entities.h"
#include "terrain.h"
#include "ui.h"

/*******************
 * Local Variables *
 *******************/

FTGLfont *FONT = NULL;

char * TXT = NULL;

/***********
 * Globals *
 ***********/

const char *FONT_FILE = "res/VerilySerifMono.otf";

const int FONT_RESOLUTION = 72;

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
  if (is_opaque(hb)) {
    blind = 1;
    // compute texture coordinates
    compute_face_tc(hb, BD_FACE_BOT, &st);
    s = st.s / ((float) BLOCK_ATLAS_WIDTH);
    t = st.t / ((float) BLOCK_ATLAS_HEIGHT);
    step_s = 1.0/BLOCK_ATLAS_WIDTH;
    step_t = 0.5/BLOCK_ATLAS_WIDTH;
    FOG_DENSITY = 1.0;
  } else if (shares_translucency(hb, B_WATER)) {
    glBlendColor(0.5, 0.5, 0.9, 1.0);
    FOG_DENSITY = WATER_FOG_DENSITY;
  } else {
    glBlendColor(1.0, 1.0, 1.0, 1.0);
    FOG_DENSITY = AIR_FOG_DENSITY*4;
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
}

/*************
 * Functions *
 *************/

void setup_ui(void) {
  // Load our font:
  FONT = ftglCreateTextureFont(FONT_FILE);
  if (!FONT) {
    fprintf(stderr, "Failed to load font '%s'.\n", FONT_FILE);
    exit(-1);
  }
  ftglSetFontFaceSize(FONT, FONT_RESOLUTION, FONT_RESOLUTION);
  TXT = (char *) malloc(sizeof(char)*2048);
}

void cleanup_ui(void) {
  ftglDestroyFont(FONT);
  free(TXT);
}

void render_string(const char *string, float size, float left, float bot) {
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glLoadIdentity();

  glTranslatef(
    (left / WINDOW_WIDTH) * OVERLAY_WIDTH - (OVERLAY_WIDTH / 2),
    (bot / WINDOW_HEIGHT) * OVERLAY_HEIGHT - (OVERLAY_HEIGHT / 2),
    -1
  );
  float scale = ((size / WINDOW_HEIGHT) * OVERLAY_HEIGHT) / FONT_RESOLUTION;
  glScalef(scale, scale, 0);

  ftglRenderFont(FONT, string, FTGL_RENDER_ALL);

  glPopMatrix();
}

void render_ui(void) {
  // Update the overlay plane size:
  compute_overlay_size();

  // Disable depth testing and load an identity matrix:
  glDisable( GL_DEPTH_TEST );
  glDisable( GL_CULL_FACE );
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  // Tinting/blinding:
  render_vision_effects();

  // Draw a watermark:
  render_string("Elf Forest", 45, -2, 0);
  render_string("<alpha test>", 22, 620, 15);

  //*
  // DEBUG: Draw geoform data:
  float depths = 0, oceans = 0, plains = 0, hills = 0, mountains = 0;
  region_pos player_pos;
  get_region_pos(PLAYER, &player_pos);
  get_geoforms(
    fastfloor(player_pos.x), fastfloor(player_pos.y),
    &depths, &oceans, &plains, &hills, &mountains
  );

  sprintf(
    TXT,
    "d: %0.2f  o: %0.2f  p: %0.2f  h: %0.2f  m: %0.2f",
    depths, oceans, plains, hills, mountains
  );

  glColor4ub(32, 32, 32, 255);
  render_string(TXT, 20, 31, 569);
  glColor4ub(255, 255, 255, 255);
  render_string(TXT, 20, 30, 570);

  sprintf(
    TXT,
    "%+4ld x    %+4ld y    %+4ld z",
    player_pos.x, player_pos.y, player_pos.z
  );

  glColor4ub(32, 32, 32, 255);
  render_string(TXT, 20, 31, 543);
  glColor4ub(255, 255, 255, 255);
  render_string(TXT, 20, 30, 544);

  glPopMatrix();
  // */

  //glPopMatrix();

  //glMatrixMode( GL_PROJECTION );

  //glPopMatrix();

  //glMatrixMode( GL_MODELVIEW );

  // HUD:

  // Crosshairs:

  // Reenable depth testing:
  glEnable( GL_CULL_FACE );
  glEnable( GL_DEPTH_TEST );
}
