// render.c
// Main render functions.

#include <math.h>
// DEBUG:
#include <stdio.h>

#include <GLee.h> // glBindBuffer etc.
#include <GL/glu.h>

#include "vbo.h"
#include "gfx.h"
#include "tex.h"
#include "display.h"

#include "render.h"

#include "datatypes/vector.h"
#include "control/ctl.h"
#include "world/world.h"
#include "world/entities.h"
#include "data/data.h"
#include "util.h"

/*************
 * Constants *
 *************/

float const AIR_FOG_DENSITY = 0.005; // TODO: adjust these.
float const WATER_FOG_DENSITY = 0.05;

// TODO: Good values here (match data.c!)
//r_cpos_t const MAX_RENDER_DISTANCES[N_LODS] = { 10, 20, 60, 175, 550 };
//r_cpos_t const MAX_RENDER_DISTANCES[N_LODS] = { 10, 18, 34, 66, 130 };
//r_cpos_t const MAX_RENDER_DISTANCES[N_LODS] = { 5, 9, 14, 18, 22 };
r_cpos_t const MAX_RENDER_DISTANCES[N_LODS] = { 3, 4, 4, 4, 4 };

/***********
 * Globals *
 ***********/

view_mode VIEW_MODE = VM_FIRST;

float SECOND_PERSON_DISTANCE = 2.7;
float THIRD_PERSON_DISTANCE = 3.2;

float FOG_DENSITY = 0.01;

/*********************
 * Private Functions *
 *********************/

// Takes a squared distance value and returns the highest level of detail
// desired at that distance.
static inline lod desired_detail(r_cpos_t dist_sq) {
  lod result = LOD_BASE;
  r_cpos_t mrd_sq = 0;
  for (result = LOD_BASE; result < N_LODS; ++result) {
    mrd_sq = MAX_RENDER_DISTANCES[result];
    mrd_sq *= mrd_sq;
    if (dist_sq <= mrd_sq) {
      break;
    }
  }
  return result;
}

/* TODO: Get rid of these
void iter_render_opaque_layer(void * coa_ptr, void *area_ptr) {
  chunk_or_approx *coa = (chunk_or_approx *) coa_ptr;
  active_entity_area *area = (active_entity_area *) area_ptr;
  if (within_render_range(coa, area)) {
    render_chunk_layer(coa, &(area->origin), L_OPAQUE);
  }
}

void iter_render_transparent_layer(void * coa_ptr, void *area_ptr) {
  chunk_or_approx *coa = (chunk_or_approx *) coa_ptr;
  active_entity_area *area = (active_entity_area *) area_ptr;
  if (within_render_range(coa, area)) {
    render_chunk_layer(coa, &(area->origin), L_TRANSPARENT);
  }
}

void iter_render_translucent_layer(void * coa_ptr, void *area_ptr) {
  chunk_or_approx *coa = (chunk_or_approx *) coa_ptr;
  active_entity_area *area = (active_entity_area *) area_ptr;
  if (within_render_range(coa, area)) {
    render_chunk_layer(coa, &(area->origin), L_TRANSLUCENT);
  }
}
*/

/*************
 * Functions *
 *************/

void render_area(
  active_entity_area *area,
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

  // Set the fog density:
  set_fog_density(FOG_DENSITY);

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

  // DEBUG: RED TRIANGLE
  /*
  glBindTexture( GL_TEXTURE_2D, 0);
  glColor4ub(255, 0, 0, 255);
  glBegin( GL_TRIANGLES );
  glVertex3f(0, 0, -1);
  glVertex3f(0, 0.125, -1);
  glVertex3f(0.125, 0.125, -1);
  glEnd();
  // */

  // Compute an eye vector:
  vector eye_vector, up_vector;
  vface(&eye_vector, yaw, pitch);
  vface(&up_vector, yaw, pitch + M_PI_2);

  //*
  if (VIEW_MODE == VM_FIRST) {
    // Look from head_pos in the direction given by eye_vector:
    gluLookAt(
      head_pos->x, // look from
        head_pos->y,
        head_pos->z,
      head_pos->x + eye_vector.x, // look at
        head_pos->y + eye_vector.y,
        head_pos->z + eye_vector.z,
      up_vector.x, // up
        up_vector.y,
        up_vector.z
    );
  } else if (VIEW_MODE == VM_SECOND) {
    // Look at head_pos in the opposite direction from eye_vector from
    // SECOND_PERSON_DISTANCE units away.
    vscale(&eye_vector, SECOND_PERSON_DISTANCE*ZOOM);
    gluLookAt(
      head_pos->x + eye_vector.x, // look from
        head_pos->y + eye_vector.y,
        head_pos->z + eye_vector.z,
      head_pos->x, // look at
        head_pos->y,
        head_pos->z,
      up_vector.x, // up
        up_vector.y,
        up_vector.z
    );
  } else if (VIEW_MODE == VM_THIRD) {
    // Look at head_pos in the direction given by the eye_vector from
    // THIRD_PERSON_DISTANCE units away.
    vscale(&eye_vector, THIRD_PERSON_DISTANCE*ZOOM);
    gluLookAt(
      head_pos->x - eye_vector.x, // look from
        head_pos->y - eye_vector.y,
        head_pos->z - eye_vector.z,
      head_pos->x, // look at
        head_pos->y,
        head_pos->z,
      up_vector.x, // up
        up_vector.y,
        up_vector.z
    );
  }
  // */


  // DEBUG: Specific look
  /*
  glLoadIdentity();
  gluLookAt(
    head_pos->x, // look from
      head_pos->y,
      head_pos->z,
    0, // look at
      0,
      0,
    up_vector.x, // up
      up_vector.y,
      up_vector.z
  );
  // */

  // DEBUG: BLUE TRIANGLE
  //*
  //printf("hp: %.2f, %.2f, %.2f\n", head_pos->x, head_pos->y, head_pos->z);

  glColor4ub(0, 0, 255, 255);
  glBegin( GL_TRIANGLES );
  glVertex3f(0, 0, -1);
  glVertex3f(0, 1, -1);
  glVertex3f(1, 1, -1);
  glEnd();
  // */

  //*
  // DEBUG: Render a bounding box:
  glColor4ub(128, 128, 128, 255); // 50% grey
  float half_box = ((float) (area->size))/2.0;

  glBegin( GL_LINE_LOOP );

  glVertex3f(-half_box, -half_box, -half_box);
  glVertex3f(-half_box, half_box, -half_box);
  glVertex3f(half_box, half_box, -half_box);
  glVertex3f(half_box, -half_box, -half_box);

  glEnd();

  glBegin( GL_LINE_LOOP );

  glVertex3f(-half_box, -half_box, half_box);
  glVertex3f(-half_box, half_box, half_box);
  glVertex3f(half_box, half_box, half_box);
  glVertex3f(half_box, -half_box, half_box);

  glEnd();
  // */

  // Render all chunks within the max render distance, starting with their
  // opaque layer, then rendering entities, then rendering their transparent
  // layer and finally rendering their translucent layer with appropriate
  // settings.

  // First, some loop variables:
  chunk_or_approx coa;
  region_chunk_pos origin;
  region_chunk_pos rcpos;
  layer ly;

  rpos__rcpos(&(area->origin), &origin);

  // Iterate over chunk positions in a sphere:
  r_cpos_t farthest_render_distance = MAX_RENDER_DISTANCES[N_LODS - 1];
  r_cpos_t skipy = 0, skipz = 0, xdist_sq = 0, xydist_sq = 0, dist_sq = 0;
  for (ly = L_OPAQUE; ly <= L_TRANSLUCENT; ++ly) {
    if (ly == L_TRANSPARENT) {
      // Before rendering the transparent layer render all entities:
      l_foreach(area->list, &iter_render_entity);
    } else if (ly == L_TRANSLUCENT) {
      // Disable face culling and set the depth mask to read-only for the
      // translucent layer:
      glDisable( GL_CULL_FACE );
      glDepthMask( GL_FALSE );
    }
    // The main loop: loop over a spherical region out to the farthest render
    // distance, getting the best data for each chunk (subject to render
    // distance constraints) and rendering it:
    for (
      rcpos.x = origin.x - farthest_render_distance;
      rcpos.x < origin.x + farthest_render_distance + 1;
      ++rcpos.x
    ) {
      xdist_sq = rcpos.x - origin.x;
      xdist_sq *= xdist_sq;
      skipy = farthest_render_distance - fastceil(
        sqrt(farthest_render_distance*farthest_render_distance - xdist_sq)
      );
      for (
        rcpos.y = origin.y - farthest_render_distance + skipy;
        rcpos.y < origin.y + farthest_render_distance + 1 - skipy;
        ++rcpos.y
      ) { 
        xydist_sq = (rcpos.y - origin.y);
        xydist_sq *= xydist_sq;
        xydist_sq += xdist_sq;
        skipz = farthest_render_distance - fastceil(
          sqrt(farthest_render_distance*farthest_render_distance - xydist_sq)
        );
        for (
          rcpos.z = origin.z - farthest_render_distance + skipz;
          rcpos.z < origin.z + farthest_render_distance + 1 - skipz;
          ++rcpos.z
        ) { 
          dist_sq = (rcpos.z - origin.z);
          dist_sq *= dist_sq;
          dist_sq += xydist_sq;
          get_best_data_limited(&rcpos, desired_detail(dist_sq), &coa);
          render_chunk_layer(&coa, &(area->origin), ly);
        }
      }
    }
    if (ly == L_TRANSLUCENT) {
      // Re-enable depth masking and face culling if necessary:
      glDepthMask( GL_TRUE );
      glEnable( GL_CULL_FACE );
    }
  }

  /* TODO: Get rid of me
  coa.type = CA_TYPE_CHUNK;
  m3_witheach(
    CHUNK_CACHE->levels[detail],
    (void *) area,
    &iter_render_opaque_layer
  );
  for (detail = LOD_BASE + 1; detail < N_LODS; ++detail) {
    m3_witheach(
      CHUNK_CACHE->levels[detail],
      (void *) area,
      &iter_render_opaque_layer
    );
  }

  // Now render all of our entities:
  l_foreach(area->list, &iter_render_entity);

  // Now render the (partially) transparent parts
  for (detail = LOD_BASE; detail < N_LODS; ++detail) {
    m3_witheach(
      cc->levels[detail],
      (void *) area,
      &iter_render_transparent_layer
    );
  }

  // Finally the translucent parts (without face-culling and using a read-only
  // depth buffer):
  glDisable( GL_CULL_FACE );
  glDepthMask( GL_FALSE );
  for (detail = LOD_BASE; detail < N_LODS; ++detail) {
    m3_witheach(
      cc->levels[detail],
      (void *) area,
      &iter_render_translucent_layer
    );
  }
  glDepthMask( GL_TRUE );
  glEnable( GL_CULL_FACE );
  */
}

// This function renders one layer of the given chunk.
void render_chunk_layer(
  chunk_or_approx *coa,
  region_pos *origin,
  layer ly
) {
  chunk *c = NULL;
  chunk_approximation *ca = NULL;
  chunk_flag flags = 0;
  vertex_buffer *vb;
  region_pos rpos;
  if (coa->type == CA_TYPE_NOT_LOADED) { return; }
  // Assign the relevant variables depending on the chunk/approximation type:
  if (coa->type == CA_TYPE_CHUNK) {
    c = (chunk *) (coa->ptr);
    flags = c->chunk_flags;
    vb = &(c->layers[ly]);
    rcpos__rpos(&(c->rcpos), &rpos);
  } else if (coa->type == CA_TYPE_APPROXIMATION) {
    ca = (chunk_approximation *) (coa->ptr);
    flags = ca->chunk_flags;
    vb = &(ca->layers[ly]);
    rcpos__rpos(&(ca->rcpos), &rpos);
  }
  // Skip this chunk if it's out-of-date:
  if (!(flags & CF_LOADED) || !(flags & CF_COMPILED)) {
    return;
  }

  // Skip this layer quickly if it's empty:
  if (vb->vertices == 0 || vb->indices == 0) {
    return;
  }

  // Push a model view matrix:
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();

  // Translate to the chunk position:
  glTranslatef(
    rpos.x - origin->x,
    rpos.y - origin->y,
    rpos.z - origin->z
  );

  // Set our drawing color:
  glColor4ub(255, 255, 255, 255); // 100% white

  //*
  // DEBUG: Draw a bounding box:
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

void iter_render_entity(void *thing) {
  entity *e = (entity*) thing;
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();

  // Translate to the entity's position:
  glTranslatef(
    e->pos.x,
    e->pos.y,
    e->pos.z
  );

  // Rotate according to the entity's facing:
  glRotatef(e->yaw*R2D, 0, 0, 1);

  // DEBUG: Render bounding box:
  //*
  glColor4ub(255, 255, 255, 255); // white

  float hx, hy, hz;
  hx = e->size.x*0.5;
  hy = e->size.y*0.5;
  hz = e->size.z*0.5;

  glBegin( GL_LINE_LOOP );

  glVertex3f(-hx, -hy, -hz);
  glVertex3f(-hx, hy, -hz);
  glVertex3f(hx, hy, -hz);
  glVertex3f(hx, -hy, -hz);

  glEnd();


  glBegin( GL_LINE_LOOP );

  glVertex3f(-hx, -hy, hz);
  glVertex3f(-hx, hy, hz);
  glVertex3f(hx, hy, hz);
  glVertex3f(hx, -hy, hz);

  glEnd();

  glBegin( GL_LINES );

  glVertex3f(-hx, -hy, -hz);
  glVertex3f(-hx, -hy, hz);

  glVertex3f(-hx, hy, -hz);
  glVertex3f(-hx, hy, hz);

  glVertex3f(hx, hy, -hz);
  glVertex3f(hx, hy, hz);

  glVertex3f(hx, -hy, -hz);
  glVertex3f(hx, -hy, hz);

  glEnd();
  // */

  glPopMatrix();
}
