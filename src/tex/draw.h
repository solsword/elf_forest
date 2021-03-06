#ifndef DRAW_H
#define DRAW_H

// draw.h
// 2D drawing functions.

#include "datatypes/vector.h"
#include "noise/noise.h"
#include "math/curve.h"

/*************
 * Constants *
 *************/

extern float const LINE_DRAWING_RESOLUTION;
extern float const CURVE_DRAWING_RESOLUTION;

/**************
 * Structures *
 **************/

// Arguments to the function for drawing points along a curve:
struct draw_curve_point_args_s;
typedef struct draw_curve_point_args_s draw_curve_point_args;

// Arguments to the function for drawing thick segments along a curve:
struct draw_thick_curve_segment_args_s;
typedef struct draw_thick_curve_segment_args_s draw_thick_curve_segment_args;

/*************************
 * Structure Definitions *
 *************************/

struct draw_curve_point_args_s {
  texture *tx; // texture to draw on
  pixel color; // color to draw with
};

struct draw_thick_curve_segment_args_s {
  texture *tx; // texture to draw on
  pixel color; // main drawing color
  pixel shade_color; // color for one outer edge
  void *width_param; // extra parameter to pass to the width function
  float (*width_func)(float, void*); // function for determining width given t
};

/***********************
 * Iteration Functions *
 ***********************/

// An iteration function for curve_witheach that draws a pixel of the given
// color on the given texture at the current x,y position along the curve.
void draw_curve_point(float t, vector *pos, vector *dir, void *args);

// An iteration function for curve_witheach that draws a l
void draw_thick_curve_segment(float t, vector *pos, vector *dir, void *args);

/*************
 * Functions *
 *************/

// Draws the line between the given points using the given color.
void draw_line(texture *tx, vector *from, vector *to, pixel color);

// Draws the line between the given points using a gradient between the given
// colors.
void draw_line_gradient(
  texture *tx,
  vector *from,
  vector *to,
  pixel start_color,
  pixel end_color
);

// Draws the given curve onto the given texture using the given color:
void draw_curve(texture *tx, curve *c, pixel color);

// Draws a thick curve using the given function to compute thickness as a
// function of the curve parameter t. The shade color is used to shade one edge
// of the curve drawn.
void draw_thick_curve(
  texture *tx,
  curve *c,
  pixel color,
  pixel shade,
  void *param,
  float (*f)(float, void*)
);

#endif // ifndef DRAW_H
