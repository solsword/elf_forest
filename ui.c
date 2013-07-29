
// ui.c
// UI rendering and management.

#include <GL/gl.h>

#include "tex.h"
#include "ctl.h"
#include "gfx.h"
#include "render.h"
#include "entities.h"
#include "ui.h"

/*************
 * Functions *
 *************/

void render_ui(void) {
  int blind = 0; // whether our head is inside an opaque block or not
  float s = 0, t = 0, step_s = 1, step_t = 1; // texture coords
  // Overlay plane size:
  float h2 = tanf(FOV*0.5);
  float w2 = h2 * ASPECT;
  // Disable depth testing and lighting, and load an identity matrix:
  glDisable( GL_DEPTH_TEST );
  glDisable( GL_LIGHTING );
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

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
    glBlendColor(0.2, 0.2, 0.9, 1.0);
    FOG_DENSITY = WATER_FOG_DENSITY;
  } else {
    glBlendColor(1.0, 1.0, 1.0, 1.0);
    FOG_DENSITY = AIR_FOG_DENSITY*4;
  }

  // Tint or blind:
  if (blind) {
    glBindTexture( GL_TEXTURE_2D, BLOCK_ATLAS );
  } else {
    glBlendFunc( GL_ZERO, GL_CONSTANT_COLOR );
  }

  // Draw the overlay plane:
  glBegin( GL_TRIANGLES );

  glTexCoord2f(s         , t + step_t);    glVertex3f(-w2, -h2, -1.0);
  glTexCoord2f(s         , t         );    glVertex3f(-w2,  h2, -1.0);
  glTexCoord2f(s + step_s, t         );    glVertex3f( w2,  h2, -1.0);
                             
  glTexCoord2f(s         , t + step_t);    glVertex3f(-w2, -h2, -1.0);
  glTexCoord2f(s + step_s, t         );    glVertex3f( w2,  h2, -1.0);
  glTexCoord2f(s + step_s, t + step_t);    glVertex3f( w2, -h2, -1.0);
 
  glEnd();

  // Release our texture or reset our blending function:
  if (blind) {
    glBindTexture( GL_TEXTURE_2D, 0 );
  } else {
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  }


  // HUD:

  // Crosshairs:
  glEnable( GL_DEPTH_TEST );
}
