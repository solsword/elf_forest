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
#include "tick/tick.h"
#include "ui/ui.h"
#include "util.h"

/*************
 * Constants *
 *************/

float const AIR_FOG_DENSITY = 0.005; // TODO: adjust these.
float const WATER_FOG_DENSITY = 0.05;

// TODO: Good values here (match data.c!)
//r_cpos_t const MAX_RENDER_DISTANCES[N_LODS] = { 10, 20, 60, 175, 550 };
//r_cpos_t const MAX_RENDER_DISTANCES[N_LODS] = { 10, 18, 34, 66, 130 };
r_cpos_t const MAX_RENDER_DISTANCES[N_LODS] = { 9, 20, 35, 38, 40 };
//r_cpos_t const MAX_RENDER_DISTANCES[N_LODS] = { 3, 5, 7, 9, 25 };
//r_cpos_t const MAX_RENDER_DISTANCES[N_LODS] = { 3, 5, 7, 9, 11 };

float const MIN_CULL_DIST = 5 * CHUNK_SIZE;
float const RENDER_ANGLE_ALLOWANCE = M_PI/8.0;
// DEBUG:
//float const MIN_CULL_DIST = 0;
//float const RENDER_ANGLE_ALLOWANCE = -M_PI/8.0;

/***********
 * Globals *
 ***********/

view_mode VIEW_MODE = VM_FIRST;

float SECOND_PERSON_DISTANCE = 2.7;
float THIRD_PERSON_DISTANCE = 3.2;

float FOG_DENSITY = 0.01;

area_render_callback AREA_PRE_RENDER_CALLBACK = NULL;

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

static inline void compute_hv_angles(
  vector const * const obs, // the vector from the eye to the observed object
  vector const * const eye_fwd, // three basis vectors for the eye-space
    vector const * const eye_side,
    vector const * const eye_up,
  float * xangle, float * yangle // outputs
) {
  vector tmp;
  vcopy(&tmp, obs);
  // 1: transform the observation vector into eye-space:
  vintermsof(
    &tmp,
    eye_side, // x becomes the sideways direction
    eye_fwd, // y becomes the forwards direction
    eye_up // z is still the upwards direction
  );

  // 3: compute horizontal and vertical angles:
  *xangle = atan2(tmp.x, tmp.y);
  *yangle = atan2(tmp.z, tmp.y);
}

// DEBUG: draw a set of basis vectors at the given location:
static inline void draw_basis_vectors(
  vector const * const view_origin,
  vector const * const view_vector,
  vector const * const vx,
  vector const * const vy,
  vector const * const vz
) {
  vector where;
  vcopy(&where, view_origin);
  vadd(&where, view_vector);
  glColor4ub(255, 0, 0, 255);
  glBegin( GL_LINES );
  glVertex3f( where.x, where.y, where.z );
  glVertex3f(
    where.x + vx->x,
    where.y + vx->y,
    where.z + vx->z
  );
  glEnd();

  glColor4ub(255, 255, 0, 255);
  glBegin( GL_LINES );
  glVertex3f( where.x, where.y, where.z );
  glVertex3f(
    where.x + vy->x,
    where.y + vy->y,
    where.z + vy->z
  );
  glEnd();

  glColor4ub(0, 0, 255, 255);
  glBegin( GL_LINES );
  glVertex3f( where.x, where.y, where.z );
  glVertex3f(
    where.x + vz->x,
    where.y + vz->y,
    where.z + vz->z
  );
  glEnd();

  char txt[2048];
  sprintf(txt, "vx: %.2f, %.2f, %.2f :: %.2f", vx->x, vx->y, vx->z, vmag(vx));
  render_string(txt, BRIGHT_RED, 18, 20, 400);
  sprintf(txt, "vy: %.2f, %.2f, %.2f :: %.2f", vy->x, vy->y, vy->z, vmag(vy));
  render_string(txt, BRIGHT_RED, 18, 20, 370);
  sprintf(txt, "vz: %.2f, %.2f, %.2f :: %.2f", vz->x, vz->y, vz->z, vmag(vz));
  render_string(txt, BRIGHT_RED, 18, 20, 340);
}

/*************
 * Functions *
 *************/

void render_area(
  active_entity_area *area,
  vector *head_pos,
  float yaw,
  float pitch,
  float fovx,
  float fovy
) {
  size_t count = 0; // A count of how many chunks we rendered
  float xangle = 0; // horizontal angle to chunk being rendered
  float yangle = 0; // vertical angle to chunk being rendered
  int cull = 0; // Whether to cull this chunk

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

  // head_pos (the argument) is a vector from the origin to the head position
  vector eye_vector; // vector pointing in the player's model's view direction
  vector up_vector; // the up-vector: pi/2 above the eye vector

  // The camera position and focus point. The view origin will be on the line
  // defined by the head position and the eye vector, while the view vector is
  // a multiple of the eye vector.
  vector view_origin = { .x = 0, .y = 0, .z = 0 };
  vector view_vector = { .x = 0, .y = 0, .z = 0 };

  vector side_vector; // the cross product of the view and up vectors

  // The vector from the viewing position to the chunk being rendered, both in 
  // standard coordinates and in eye-space.
  vector chunk_vector;

  // Compute the eye, up, and side vectors:
  vface(&eye_vector, yaw, pitch);
  vface(&up_vector, yaw, pitch + M_PI_2);

  //*
  if (VIEW_MODE == VM_FIRST) {
    // Look from head_pos in the direction given by eye_vector:
    vcopy(&view_origin, head_pos);
    vcopy(&view_vector, &eye_vector);
  } else if (VIEW_MODE == VM_SECOND) {
    // Look at head_pos in the opposite direction from eye_vector from
    // SECOND_PERSON_DISTANCE units away.
    vscale(&eye_vector, SECOND_PERSON_DISTANCE*ZOOM);

    vcopy(&view_origin, head_pos);
    vadd(&view_origin, &eye_vector);

    vcopy(&view_vector, &eye_vector);
    vscale(&view_vector, -1);
  } else if (VIEW_MODE == VM_THIRD) {
    // Look at head_pos in the direction given by the eye_vector from
    // THIRD_PERSON_DISTANCE units away.
    vscale(&eye_vector, THIRD_PERSON_DISTANCE*ZOOM);

    vcopy(&view_origin, head_pos);
    vsub(&view_origin, &eye_vector);

    vcopy(&view_vector, &eye_vector);
  }

  // Call gluLookAt with the computed view origin and vector:
  gluLookAt(
    view_origin.x, // look from
      view_origin.y,
      view_origin.z,
    view_origin.x + view_vector.x, // look at
      view_origin.y + view_vector.y,
      view_origin.z + view_vector.z,
    up_vector.x, // up
      up_vector.y,
      up_vector.z
  );
  // Now that we've done our gluLookAt, we can normalize our view vector and
  // compute the side vector, which together with the view vector and up vector
  // completely defines our viewing coordinate space.
  vnorm(&view_vector);
  vcross(&side_vector, &view_vector, &up_vector);
  // */

  // Take this opportunity to call the AREA_PRE_RENDER_CALLBACK now that our
  // model view matrix is set up:
  if (AREA_PRE_RENDER_CALLBACK != NULL) {
    (*AREA_PRE_RENDER_CALLBACK)(area);
  }

  // DEBUG: BLUE TRIANGLE
  /*
  //printf("hp: %.2f, %.2f, %.2f\n", head_pos->x, head_pos->y, head_pos->z);

  glColor4ub(0, 0, 255, 255);
  glBegin( GL_TRIANGLES );
  glVertex3f(0, 0, -1);
  glVertex3f(0, 1, -1);
  glVertex3f(1, 1, -1);
  glEnd();
  // */

  // DEBUG: Render a bounding box:
  /*
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
    // distance constraints) and rendering one of its layers:
    for (
      rcpos.x = origin.x - farthest_render_distance;
      rcpos.x < origin.x + farthest_render_distance + 1;
      ++rcpos.x
    ) {
      chunk_vector.x = (rcpos.x - origin.x) * CHUNK_SIZE - view_origin.x;
      xdist_sq = (rcpos.x - origin.x);
      xdist_sq *= xdist_sq;
      skipy = farthest_render_distance - fastceil(
        sqrt(farthest_render_distance*farthest_render_distance - xdist_sq)
      );
      for (
        rcpos.y = origin.y - farthest_render_distance + skipy;
        rcpos.y < origin.y + farthest_render_distance + 1 - skipy;
        ++rcpos.y
      ) { 
        chunk_vector.y = (rcpos.y - origin.y) * CHUNK_SIZE - view_origin.y;
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
          chunk_vector.z = (rcpos.z - origin.z) * CHUNK_SIZE - view_origin.z;
          dist_sq = (rcpos.z - origin.z);
          dist_sq *= dist_sq;
          dist_sq += xydist_sq;
          get_best_data_limited(&rcpos, desired_detail(dist_sq), &coa);
          // Compute angle to chunk:
          compute_hv_angles(
            &chunk_vector,
            &view_vector, &side_vector, &up_vector,
            &xangle, &yangle
          );
          cull = ( // chunk is farther than the min cull distance
            (
              fabs(chunk_vector.x) +
              fabs(chunk_vector.y) +
              fabs(chunk_vector.z)
            ) > MIN_CULL_DIST
          &&
            ( // and it fails the frustum test
              fabs(xangle) > (fovx/2) + RENDER_ANGLE_ALLOWANCE
            ||
              fabs(yangle) > (fovy/2) + RENDER_ANGLE_ALLOWANCE
            )
          );

          // DEBUG: Draw vectors to nearby chunks:
          /*
          if (
            (
              fabs(chunk_vector.x) +
              fabs(chunk_vector.y) +
              fabs(chunk_vector.z)
            ) < CHUNK_SIZE * 3
          ) {
            glColor4ub(255, 255, 0, 255);
            //glLineWidth(2);
            glBegin( GL_LINES );
            glVertex3f(
              view_origin.x + view_vector.x,
              view_origin.y + view_vector.y,
              view_origin.z + view_vector.z
            );
            glVertex3f(
              view_origin.x + chunk_vector.x,
              view_origin.y + chunk_vector.y,
              view_origin.z + chunk_vector.z
            );
            glEnd();
            glColor4ub(255, 255, 128, 255);
            glPointSize(10);
            glBegin( GL_POINTS );
            glVertex3f(
              view_origin.x + chunk_vector.x,
              view_origin.y + chunk_vector.y,
              view_origin.z + chunk_vector.z
            );
            glEnd();
          }
          // */
          if ( !cull && render_chunk_layer(&coa, &(area->origin), ly) ) {
            count += 1;
          }
        }
      }
    }
    if (ly == L_TRANSLUCENT) {
      // Re-enable depth masking and face culling if necessary:
      glDepthMask( GL_TRUE );
      glEnable( GL_CULL_FACE );
    }
  }

  // DEBUG: Draw eye-space vectors:
  /*
  draw_basis_vectors(
    &view_origin,
    &view_vector,
    &side_vector,
    &view_vector,
    &up_vector
  );
  // */

  // Update the count of rendered layers:
  update_count(&CHUNK_LAYERS_RENDERED, count);
}

// This function renders one layer of the given chunk. It returns 1 if it
// renders the layer and 0 if it skips it (due to missing data or an empty
// layer).
int render_chunk_layer(
  chunk_or_approx *coa,
  region_pos *origin,
  layer ly
) {
  chunk *c = NULL;
  chunk_approximation *ca = NULL;
  chunk_flag flags = 0;
  vertex_buffer *vb;
  region_pos rpos;
  if (coa->type == CA_TYPE_NOT_LOADED) { return 0; }
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
    return 0;
  }

  // Skip this layer quickly if it's empty:
  if (vb->vertices == 0 || vb->indices == 0) {
    return 0;
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

  // DEBUG: Draw a bounding box:
  /*
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

  // We successfully rendered the layer:
  return 1;
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