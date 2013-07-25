// render.c
// Main render functions.

#include <math.h>
// DEBUG:
#include <stdio.h>

#include <GLee.h> // glBindBuffer etc.
#include <GL/glu.h>

#include "util.h"
#include "vbo.h"
#include "vector.h"
#include "gfx.h"
#include "tex.h"
#include "display.h"
#include "ctl.h"
#include "entities.h"

#include "render.h"

/***********
 * Globals *
 ***********/

view_mode VIEW_MODE = VM_THIRD;

float SECOND_PERSON_DISTANCE = 2.7;
float THIRD_PERSON_DISTANCE = 3.2;

/*************
 * Functions *
 *************/

void render_frame(
  frame *f,
  vector *head_pos,
  float yaw,
  float pitch
) {
  // Clear the buffers:
  clear_color_buffer();
  clear_depth_buffer();

  // Set up a fresh model view:
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // DEBUG: spin right 'round:
  /*
  static float theta = 0;
  float r = 0.75*FULL_FRAME - ZOOM;
  gluLookAt(
    HALF_FRAME + r*cos(theta),
    HALF_FRAME + r*sin(theta),
    FULL_FRAME * 0.75 - 0.25 * FULL_FRAME * (ZOOM/(0.75*FULL_FRAME)),
    //0, 0, 0,
    head_pos->x+HALF_FRAME, head_pos->y+HALF_FRAME, head_pos->z+HALF_FRAME,
    0, 0, 1
  ); // Look north from head_pos
  if (!PAUSED) {
    theta += M_PI/256;
  }
  */

  // Compute an eye vector:
  vector eye_vector, up_vector;
  vface(&eye_vector, yaw, pitch);
  vface(&up_vector, yaw, pitch + M_PI_2);

  //*
  if (VIEW_MODE == VM_FIRST) {
    // Look from head_pos in the direction given by eye_vector:
    gluLookAt(
      head_pos->x + HALF_FRAME, // look from
        head_pos->y + HALF_FRAME,
        head_pos->z + HALF_FRAME,
      head_pos->x + HALF_FRAME + eye_vector.x, // look at
        head_pos->y + HALF_FRAME + eye_vector.y,
        head_pos->z + HALF_FRAME + eye_vector.z,
      up_vector.x, // up
        up_vector.y,
        up_vector.z
    );
  } else if (VIEW_MODE == VM_SECOND) {
    // Look at head_pos in the opposite direction from eye_vector from
    // SECOND_PERSON_DISTANCE units away.
    vscale(&eye_vector, SECOND_PERSON_DISTANCE*ZOOM);
    gluLookAt(
      head_pos->x + HALF_FRAME + eye_vector.x, // look from
        head_pos->y + HALF_FRAME + eye_vector.y,
        head_pos->z + HALF_FRAME + eye_vector.z,
      head_pos->x + HALF_FRAME, // look at
        head_pos->y + HALF_FRAME,
        head_pos->z + HALF_FRAME,
      up_vector.x, // up
        up_vector.y,
        up_vector.z
    );
  } else if (VIEW_MODE == VM_THIRD) {
    // Look at head_pos in the direction given by the eye_vector from
    // THIRD_PERSON_DISTANCE units away.
    vscale(&eye_vector, THIRD_PERSON_DISTANCE*ZOOM);
    gluLookAt(
      head_pos->x + HALF_FRAME - eye_vector.x, // look from
        head_pos->y + HALF_FRAME - eye_vector.y,
        head_pos->z + HALF_FRAME - eye_vector.z,
      head_pos->x + HALF_FRAME, // look at
        head_pos->y + HALF_FRAME,
        head_pos->z + HALF_FRAME,
      up_vector.x, // up
        up_vector.y,
        up_vector.z
    );
  }
  // */

  // DEBUG: Set light position:
  GLfloat pos[4] = {
    HALF_FRAME,
    HALF_FRAME,
    (float) HALF_FRAME * 1.5,
    0.0
  };
  glLightfv( GL_LIGHT0, GL_POSITION, pos);

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
  foreach(f->entities, &render_entity);

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

  // Reset the model view matrix:
  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();
}

void render_entity(void *thing) {
  entity *e = (entity*) thing;
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();

  // Translate to the entity's position:
  glTranslatef(
    HALF_FRAME + e->pos.x,
    HALF_FRAME + e->pos.y,
    HALF_FRAME + e->pos.z
  );

  // Rotate according to the entity's facing:
  glRotatef(e->yaw*R2D, 0, 0, 1);

  // DEBUG: Test triangle:
  //*
  glColor4ub(255, 255, 255, 255); // white

  glBegin( GL_TRIANGLES );

  glVertex3f(-1, 0, -1);
  glVertex3f(0, 0, 1);
  glVertex3f(1, 0, -1);

  // 2-sided for extra visibility:
  glVertex3f(-1, 0, -1);
  glVertex3f(1, 0, -1);
  glVertex3f(0, 0, 1);

  glEnd();
  // */

  glPopMatrix();
}
