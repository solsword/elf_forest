// draw.c
// 2D drawing functions.

#include "datatypes/vector.h"
#include "noise/noise.h"
#include "math/curve.h"

#include "tex.h"

#include "draw.h"

/*************
 * Constants *
 *************/

float const LINE_DRAWING_RESOLUTION = 0.05;
float const CURVE_DRAWING_RESOLUTION = 0.05;

/***********************
 * Iteration Functions *
 ***********************/

void draw_curve_point(float t, vector *pos, vector *dir, void *args) {
  draw_curve_point_args *dcpargs = (draw_curve_point_args*) args;
  size_t x, y;
  x = (size_t) pos->x;
  y = (size_t) pos->y;
  if (
    ((x % dcpargs->tx->width) == x)
  &&
    ((y % dcpargs->tx->height) == y)
  ) {
    tx_set_px(
      dcpargs->tx,
      dcpargs->color,
      x, y
    );
  }
}

void draw_thick_curve_segment(float t, vector *pos, vector *dir, void *args) {
  draw_thick_curve_segment_args *dtcsargs=(draw_thick_curve_segment_args*) args;
  // Compute a vector from the point pos to the edge of the current segment:
  vector toedge;
  vcopy(&toedge, dir);
  vrotz(&toedge, M_PI_2);
  vnorm(&toedge);
  float width = (*(dtcsargs->width_func))(t, dtcsargs->width_param);
  vscale(&toedge, width/2.0);

  curve segment; // actually just a straight line...

  segment.from.x = pos->x - toedge.x; // from one edge
  segment.from.y = pos->y - toedge.y;
  segment.from.z = 0;
  segment.come_from.x = pos->x - toedge.x;
  segment.come_from.y = pos->y - toedge.y;
  segment.come_from.z = 0;

  segment.go_towards.x = pos->x + toedge.x; // to the other
  segment.go_towards.y = pos->y + toedge.y;
  segment.go_towards.z = 0;
  segment.to.x = pos->x + toedge.x;
  segment.to.y = pos->y + toedge.y;
  segment.to.z = 0;

  // Draw our segment:
  draw_curve(
    dtcsargs->tx,
    &segment,
    dtcsargs->color
  );

  // Draw the shade pixel:
  tx_set_px(
    dtcsargs->tx,
    dtcsargs->shade_color,
    (size_t) segment.from.x,
    (size_t) segment.from.y
  );
}

/*************
 * Functions *
 *************/

void draw_line(
  texture *tx,
  vector *from,
  vector *to,
  pixel color
) {
  vector line, here;
  float t, length;
  size_t row, col;
  vcopy(&line, to);
  vsub(&line, from);
  line.z = 0;
  length = vmag(&line);
  for (t = 0; t < 1; t += LINE_DRAWING_RESOLUTION / length) {
    vlerp(from, to, t, &here);
    col = (size_t) here.x;
    row = (size_t) here.y;
    if (
      ((col % tx->width) == col)
    &&
      ((row % tx->height) == row)
    ) {
      tx_set_px(tx, color, col, row);
    }
  }
}

void draw_line_gradient(
  texture *tx,
  vector *from,
  vector *to,
  pixel start_color,
  pixel end_color
) {
  vector line, here;
  float t, length;
  size_t row, col;
  vcopy(&line, to);
  vsub(&line, from);
  line.z = 0;
  length = vmag(&line);
  for (t = 0; t < 1; t += LINE_DRAWING_RESOLUTION / length) {
    vlerp(from, to, t, &here);
    col = (size_t) here.x;
    row = (size_t) here.y;
    if (
      ((col % tx->width) == col)
    &&
      ((row % tx->height) == row)
    ) {
      tx_set_px(tx, px_interp(start_color, end_color, t), col, row);
    }
  }
}

void draw_curve(
  texture *tx,
  curve *c,
  pixel color
) {
  draw_curve_point_args dcpargs;
  dcpargs.tx = tx;
  dcpargs.color = color;
  curve_witheach(c, CURVE_DRAWING_RESOLUTION, &dcpargs, draw_curve_point);
}

void draw_thick_curve(
  texture *tx,
  curve *c,
  pixel color,
  pixel shade,
  void *param,
  float (*f)(float, void*)
) {
  draw_thick_curve_segment_args dtcsargs;
  dtcsargs.tx = tx;
  dtcsargs.color = color;
  dtcsargs.shade_color = shade;
  dtcsargs.width_func = f;
  dtcsargs.width_param = param;
  curve_witheach(
    c,
    CURVE_DRAWING_RESOLUTION,
    &dtcsargs,
    draw_thick_curve_segment
  );
}
