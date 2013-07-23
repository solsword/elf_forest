// render.c
// Main render functions.

#include <math.h>
// DEBUG:
#include <stdio.h>
#include "ctl.h"

#include <GLee.h> // glBindBuffer etc.

#include "vbo.h"
#include "physics.h"
#include "render.h"
#include "conv.h"
#include "display.h"
#include "gfx.h"
#include "tex.h"
#include "entities.h"

/*************
 * Functions *
 *************/

void render_frame(
  frame *f,
  vector *eye_pos,
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
    eye_pos->x+HALF_FRAME, eye_pos->y+HALF_FRAME, eye_pos->z+HALF_FRAME,
    0, 0, 1
  ); // Look north from eye_pos
  if (!PAUSED) {
    theta += M_PI/256;
  }
  //theta = 0;
  
  // Transform according to the camera parameters:
  /*
  gluLookAt(
    eye_pos->x+HALF_FRAME, eye_pos->y-HALF_FRAME/2, eye_pos->z+HALF_FRAME,
    //0, 0, 0,
    eye_pos->x+HALF_FRAME, eye_pos->y+HALF_FRAME + 1, eye_pos->z+HALF_FRAME,
    0, 0, 1
  ); // Look north from eye_pos
  */
  // Rotate according to the pitch and yaw given:
  glRotatef(-yaw*R2D, 0, 0, 1);
  glRotatef(-pitch*R2D, 1, 0, 0);

  //*
  // DEBUG: Render a bounding box:
  glColor4ub(128, 128, 128, 128); // 50% 50% grey

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
  frame_chunk_index idx;
  for (idx.x = 0; idx.x < FRAME_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < FRAME_SIZE; ++idx.y) {
      for (idx.z = 0; idx.z < FRAME_SIZE; ++idx.z) {
        render_chunk_layer(f, idx, L_OPAQUE);
      }
    }
  }

  // Now render all of our entities:
  //foreach(f->entities, &render_entity);

  // Now render the translucent parts:
  for (idx.x = 0; idx.x < FRAME_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < FRAME_SIZE; ++idx.y) {
      for (idx.z = 0; idx.z < FRAME_SIZE; ++idx.z) {
        render_chunk_layer(f, idx, L_TRANSLUCENT);
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
  // */
}

// This function renders one layer of the given chunk.
void render_chunk_layer(
  frame *f,
  frame_chunk_index idx,
  layer l
) {
  chunk *c = chunk_at(f, idx);
  vertex_buffer *vb = &(c->opaque_vertices); // Default value
  if (l == L_OPAQUE) {
    vb = &(c->opaque_vertices);
  } else if (l == L_TRANSLUCENT) {
    vb = &(c->translucent_vertices);
  }
  // Skip this layer quickly if it's empty:
  if (vb->vertices == 0 || vb->indices == 0) {
    return;
  }

  // Push a model view matrix:
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();

  // Translate to the chunk position:
  glTranslatef(idx.x*CHUNK_SIZE, idx.y*CHUNK_SIZE, idx.z*CHUNK_SIZE);

  // Set our drawing color:
  glColor4ub(255, 255, 255, 255); // 100% white

  // DEBUG: Draw a bounding box:
  //*
  glBegin( GL_LINE_LOOP );

  glVertex3f(0, 0, 0);
  glVertex3f(0, CHUNK_SIZE, 0);
  glVertex3f(CHUNK_SIZE, CHUNK_SIZE, 0);
  glVertex3f(CHUNK_SIZE, 0, 0);

  glEnd();

  glBegin( GL_LINE_LOOP );

  glVertex3f(0, 0, CHUNK_SIZE);
  glVertex3f(0, CHUNK_SIZE, CHUNK_SIZE);
  glVertex3f(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE);
  glVertex3f(CHUNK_SIZE, 0, CHUNK_SIZE);

  glEnd();
  // */

  // Scale our texture coordinates so that 1.0 -> 1/width or 1/height:
  glMatrixMode( GL_TEXTURE );
  glPushMatrix();
  glLoadIdentity();
  glScalef(1/(float)BLOCK_ATLAS_WIDTH, 1/(float)BLOCK_ATLAS_HEIGHT, 1);
  glMatrixMode( GL_MODELVIEW );

  // Draw the appropriate vertex buffer:
  draw_vertex_buffer(vb, BLOCK_ATLAS);

  // Reset the texture matrix:
  glMatrixMode( GL_TEXTURE );
  glPopMatrix();
  glMatrixMode( GL_MODELVIEW );

  // Pop the model view matrix:
  glPopMatrix();
}

void render_entity(void *thing) {
  entity *e = (entity*) thing;
  // DEBUG: Test triangle:
  //*
  glColor4ub(255, 255, 255, 255); // white

  glBegin( GL_TRIANGLES );

  glVertex3f(e->pos.x - 1, e->pos.y, e->pos.z - 1);

  glVertex3f(e->pos.x, e->pos.y, e->pos.z + 1);

  glVertex3f(e->pos.x + 1, e->pos.y, e->pos.z - 1);

  glEnd();
  // */
}
