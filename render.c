// render.c
// Main render functions.

#include <math.h>
// DEBUG:
#include <stdio.h>
#include "ctl.h"

#include <GLee.h> // glBindBuffer etc.

#include "render.h"
#include "conv.h"
#include "display.h"
#include "gfx.h"
#include "tex.h"

/*************
 * Functions *
 *************/

void render_frame(
  frame *f,
  float ex, float ey, float ez,
  float yaw,
  float pitch
) {
  static float theta = 0;
  clear_color_buffer();
  clear_depth_buffer();

  // Set up a fresh model view:
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // DEBUG: spin right 'round:
  float r = 0.75*FULL_FRAME - ZOOM;
  gluLookAt(
    HALF_FRAME + r*cos(theta),
    HALF_FRAME + r*sin(theta),
    FULL_FRAME * 0.75 - 0.25 * FULL_FRAME * (ZOOM/(0.75*FULL_FRAME)),
    //0, 0, 0,
    ex+HALF_FRAME, ey+HALF_FRAME, ez+HALF_FRAME,
    0, 0, 1
  ); // Look north from ex, ey, ez
  if (!PAUSED) {
    theta += M_PI/256;
  }
  //theta = 0;
  
  // Transform according to the camera parameters:
  /*
  gluLookAt(
    ex+HALF_FRAME, ey-HALF_FRAME/2, ez+HALF_FRAME,
    //0, 0, 0,
    ex+HALF_FRAME, ey+HALF_FRAME + 1, ez+HALF_FRAME,
    0, 0, 1
  ); // Look north from ex, ey, ez
  */
  // Rotate according to the pitch and yaw given:
  glRotatef(-yaw*R2D, 0, 0, 1);
  glRotatef(-pitch*R2D, 1, 0, 0);

  //*
  // DEBUG: Render a bounding box:
  glColor4ub(128, 128, 128, 255); // grey

  glBegin( GL_LINE_LOOP );

  glVertex3f(0, 0, 0);
  glVertex3f(0, FULL_FRAME, 0);
  glVertex3f(FULL_FRAME, FULL_FRAME, 0);
  glVertex3f(FULL_FRAME, 0, 0);

  glEnd();

  glBegin( GL_LINE_LOOP );

  glVertex3f(0, 0, FULL_FRAME);
  glVertex3f(0, FULL_FRAME, FULL_FRAME);
  glVertex3f(FULL_FRAME, FULL_FRAME, FULL_FRAME);
  glVertex3f(FULL_FRAME, 0, FULL_FRAME);

  glEnd();

  // DEBUG: Render axes:
  glColor4ub(255, 0, 0, 255); // red = x
  glBegin( GL_LINES );
  glVertex3f(-0.5, -0.5, -0.5);
  glVertex3f(0.5, -0.5, -0.5);
  glEnd();
  glColor4ub(0, 128, 0, 255); // green = y
  glBegin( GL_LINES );
  glVertex3f(-0.5, -0.5, -0.5);
  glVertex3f(-0.5, 0.5, -0.5);
  glEnd();
  glColor4ub(0, 0, 255, 255); // blue = z
  glBegin( GL_LINES );
  glVertex3f(-0.5, -0.5, -0.5);
  glVertex3f(-0.5, -0.5, 0.5);
  glEnd();
  // */

  // Render the opaque parts of each chunk:
  uint16_t cx, cy, cz;
  for (cx = 0; cx < FRAME_SIZE; ++cx) {
    for (cy = 0; cy < FRAME_SIZE; ++cy) {
      for (cz = 0; cz < FRAME_SIZE; ++cz) {
        render_chunk_opaque(f, cx, cy, cz);
      }
    }
  }

  // Now render the translucent parts:
  for (cx = 0; cx < FRAME_SIZE; ++cx) {
    for (cy = 0; cy < FRAME_SIZE; ++cy) {
      for (cz = 0; cz < FRAME_SIZE; ++cz) {
        render_chunk_translucent(f, cx, cy, cz);
      }
    }
  }

  // DEBUG: Test quad for our texture:
  /*
  glColor4ub(255, 255, 255, 255); // white
  glBindTexture( GL_TEXTURE_2D, BLOCK_ATLAS );

  glBegin( GL_TRIANGLES );
  glTexCoord2f(0, 0);
  glVertex3f(0, 0, 0);

  glTexCoord2f(0, 1);
  glVertex3f(0, FULL_FRAME, 0);

  glTexCoord2f(1, 1);
  glVertex3f(FULL_FRAME, FULL_FRAME, 0);

  glTexCoord2f(0, 0);
  glVertex3f(0, 0, 0);

  glTexCoord2f(1, 1);
  glVertex3f(FULL_FRAME, FULL_FRAME, 0);

  glTexCoord2f(1, 0);
  glVertex3f(FULL_FRAME, 0, 0);

  glEnd();

  glBindTexture( GL_TEXTURE_2D, 0 );
  */
}

// We want two versions of this function: one that will render the opaque parts
// of the chunk, and another that will render the translucent parts. We'll
// define the function as a macro with substitution points for the key
// differences and then define each variant below without copy-pasting any
// code.
#define RENDER_CHUNK_VAR \
  void FNAME( \
    frame *f, \
    uint16_t cx, uint16_t cy, uint16_t cz \
  ) { \
    chunk *c = chunk_at(f, cx, cy, cz); \
    glMatrixMode( GL_MODELVIEW ); \
    glPushMatrix(); \
    /* Translate to the chunk position: */ \
    glTranslatef(cx*CHUNK_SIZE, cy*CHUNK_SIZE, cz*CHUNK_SIZE); \
    if (c->VBO == 0 || c->IBO == 0) { \
      glPopMatrix(); \
      return; \
    } \
    /* Enable array functionality: */ \
    glDisableClientState( GL_COLOR_ARRAY ); \
    glEnableClientState( GL_VERTEX_ARRAY ); \
    glEnableClientState( GL_NORMAL_ARRAY ); \
    glEnableClientState( GL_TEXTURE_COORD_ARRAY ); \
    /* Bind our texture: */ \
    glBindTexture( GL_TEXTURE_2D, BLOCK_ATLAS ); \
    /* Scale our texture coordinates so that 1.0 -> 1/width or 1/height: */ \
    glMatrixMode( GL_TEXTURE ); \
    glPushMatrix(); \
    glLoadIdentity(); \
    glScalef(1/(float)BLOCK_ATLAS_WIDTH, 1/(float)BLOCK_ATLAS_HEIGHT, 1); \
    glMatrixMode( GL_MODELVIEW ); \
    /* Set our drawing color: */ \
    glColor4ub(255, 255, 255, 255); \
    /* Bind the buffer object holding vertex data: */ \
    glBindBuffer( GL_ARRAY_BUFFER, c->VBO ); \
    /* Set the vertex/normal/texture data strides & offsets: */ \
    glVertexPointer( \
      3, \
      GL_SHORT, \
      VERTEX_STRIDE*sizeof(GLshort), \
      (const GLvoid *)0 \
    ); \
    glNormalPointer( \
      GL_SHORT, \
      VERTEX_STRIDE*sizeof(GLshort), \
      (const GLvoid *) (3*sizeof(GLshort)) \
    ); \
    glTexCoordPointer( \
      2, \
      GL_SHORT, \
      VERTEX_STRIDE*sizeof(GLshort), \
      (const GLvoid *) (6*sizeof(GLshort)) \
    ); \
    /* Bind the buffer object holding index data: */ \
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, c->IBO ); \
    /* Draw the entire index array as triangles: */ \
    glDrawElements( GL_TRIANGLES, c->VCOUNT, GL_UNSIGNED_INT, 0 ); \
    /* Unbind the buffers: */ \
    glBindBuffer( GL_ARRAY_BUFFER, 0 ); \
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ); \
    /* Unbind our texture: */ \
    glBindTexture( GL_TEXTURE_2D, 0 ); \
    /* Reset the texture matrix: */ \
    glMatrixMode( GL_TEXTURE ); \
    glPopMatrix(); \
    glMatrixMode( GL_MODELVIEW ); \
    /* Disable the array functionality: */ \
    glDisableClientState( GL_VERTEX_ARRAY ); \
    glDisableClientState( GL_NORMAL_ARRAY ); \
    /* Pop the model view matrix: */ \
    glPopMatrix(); \
  }

#define FNAME render_chunk_opaque
#define VBO opaque_vertex_bo
#define IBO opaque_index_bo
#define VCOUNT o_vcount

// The opaque variant:
RENDER_CHUNK_VAR

#undef FNAME
#undef VBO
#undef IBO
#undef VCOUNT

#define FNAME render_chunk_translucent
#define VBO translucent_vertex_bo
#define IBO translucent_index_bo
#define VCOUNT t_vcount

// The translucent variant:
RENDER_CHUNK_VAR
