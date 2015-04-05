// vim: syn=java

import perlin.*;

Perlin pnl = new Perlin(this);

int WINDOW_WIDTH = 900;
int WINDOW_HEIGHT = 600;

int ARRAY_WIDTH = 180;
int ARRAY_HEIGHT = 120;

int GRID_WIDTH = 120;
int GRID_HEIGHT = 50;

String DISPLAY = "height";


// Force-directed grid parameters:

float GRID_RESOLUTION = 100.0;

float SG_HEIGHT = sin(PI/3.0);
float SG_OFFSET = cos(PI/3.0);

float GRID_BASE_BUMPINESS = 0.05;

float DEFAULT_SCALE = 0.15 / 1.2;
float SCALE = DEFAULT_SCALE;
float NS_LARGE =  0.0014;
float NS_MEDIUM = 0.042;
float NS_SMALL = 0.104;
float NS_TINY = 0.225;

float DSTR = 1.3;

float DEFAULT_RUSTLE_STR = 0.8;

float SETTLE_K = 1.5;
float SETTLE_DISTANCE = GRID_RESOLUTION * 1.3;
float SETTLE_DT = 0.01;

float UNTANGLE_DT = 0.1;

float SEAM_DT = 0.1;
float MIN_SEAM_WIDTH = GRID_RESOLUTION * 4.3;
float MAX_SEAM_WIDTH = GRID_RESOLUTION * 12.1;

int DEFAULT_UNTANGLE_STEPS = 5;
int DEFAULT_SETTLE_STEPS = 20;


// Reaction-diffusion system parameters:

int BUILD_CYCLES = 80;
int STEP_PARTICLES = 400;
int EXTRA_PARTICLES = 20;
int SLIP = 3;
float PARTICLE_HEIGHT = 1.0;
float BLUR_BIAS = 8;
float MAX_SLOPE = 0.01;
float SLUMP_RATE = 0.2;
float SLUMP_STEPS = 50;
float FINAL_SLUMPS = 5;

int MAX_WANDER_STEPS = 30000;


// Precipitation & flow parameters:

float PRECIP_BASE_STR = 0.5;
float PRECIP_BASE_SCALE = 8.5;
float PRECIP_BASE_DSTR = 0.1;
float PRECIP_BASE_DSCALE = 5.4;

float PRECIP_EXTRA_STR = 0.3;
float PRECIP_EXTRA_SCALE = 3.7;
float PRECIP_EXTRA_DSTR = 0.08;
float PRECIP_EXTRA_DSCALE = 6.2;

int FLOW_MAP_STEPS = 4;
int SLUMP_FLOW_STEPS = 6;
float EROSION_STRENGTH = 0.2;
float FLOW_MAX_SLOPE = 0.2;
float FLOW_SLUMP_RATE = 0.3;

// Coloring parameters:

String COLORMODE = "grayscale";
float SEA_LEVEL = 0.4;


float pnoise(float x, float y) {
  float[] xy = new float[2];
  xy[0] = x;
  xy[1] = y;
  return pnl.noise2(xy);
}

float pnoise(float x, float y, float z) {
  float[] xyz = new float[3];
  xyz[0] = x;
  xyz[1] = y;
  xyz[2] = z;
  float result = pnl.noise3(xyz);
  if (result < -1.0 || result > 1.0) { println("Bad result: ", result); }
  return result;
}

float sigmoid(float x) {
  // Takes an input between 0 and 1 and smoothes it towards the extremes a bit,
  // using a sigmoid which has a slope of 0 at both endpoints.
  float remapped, result;
  if (x < 0.5) {
    remapped = x * 2;
    result = pow(remapped, 2) * 0.5;
  } else {
    remapped = 1 - (x - 0.5)*2;
    result = (1 - pow(remapped, 2))*0.5 + 0.5;
  }
  return result;
}


color colormap(float h) {//, String colormode) {
  if (COLORMODE == "grayscale") {
    return color(0, 0, h);
  } else if (COLORMODE == "false") {
    if (h < SEA_LEVEL) {
      return color(0.7, 0.45, 0.3 + 0.7 * h);
    } else {
      return color(
        0.38 - 0.28*(h - SEA_LEVEL),
        0.7,
        0.6 + 0.3*(h - SEA_LEVEL)
      );
    }
  } else {
    println("Unknown color mode '" + COLORMODE + "'.");
    return 0;
  }
}

class Point {
  public float x, y, z;
  Point(float x, float y, float z) {
    this.x = x;
    this.y = y;
    this.z = z;
  }

  Point(Point o) {
    this.x = o.x;
    this.y = o.y;
    this.z = o.z;
  }

  float magnitude() {
    return sqrt(pow(this.x, 2) + pow(this.y, 2) + pow(this.z, 2));
  }

  float mag2() {
    return pow(this.x, 2) + pow(this.y, 2) + pow(this.z, 2);
  }

  float mag_in_terms_of(Point o) {
    float result = this.magnitude() / o.magnitude();
    if (this.dot(o) < 0) { return -result; }
    return result;
  }

  float dist(Point other) {
    return sqrt(
      pow(other.x - this.x, 2)
    + pow(other.y - this.y, 2)
    + pow(other.z - this.z, 2)
    );
  }

  void norm() {
    float m = this.magnitude();
    this.x /= m;
    this.y /= m;
    this.z /= m;
  }

  Point vector_to(Point other) {
    return new Point(other.x - this.x, other.y - this.y, other.z - this.z);
  }

  Point halfway_to(Point other) {
    return new Point(
      this.x + (other.x - this.x)/2.0,
      this.y + (other.y - this.y)/2.0,
      this.z + (other.z - this.z)/2.0
    );
  }

  Point spring_force_from(Point other, float eq, float k) {
    Point v = this.vector_to(other);
    float d = v.magnitude();
    float s = d - eq;
    v.norm();
    v.scale(s * k);
    return v;
  }

  void scale(float s) {
    this.x *= s;
    this.y *= s;
    this.z *= s;
  }

  float dot(Point o) {
    return this.x * o.x + this.y * o.y + this.z * o.z;
  }

  Point cross(Point o) {
    return new Point(
      this.y * o.z - this.z * o.y,
      -(this.x * o.z - this.z * o.x),
      this.x * o.y - this.y * o.x
    );
  }

  void add(Point o) {
    this.x += o.x;
    this.y += o.y;
    this.z += o.z;
  }

  Point project_onto(Point v) {
    Point result = new Point(v.x, v.y, v.z);
    float m = this.dot(v) / v.mag2();
    result.scale(m);
    return result;
  }

  void xy_rotate(float theta) {
    float tx = this.x;
    this.x = cos(theta) * this.x - sin(theta) * this.y;
    this.y = sin(theta) * tx + cos(theta) * this.y;
  }
}

class LineSegment {
  Point from, to;
  LineSegment(Point from, Point to) {
    this.from = from;
    this.to = to;
  }

  boolean xy_same_side(Point a, Point b) {
    Point va, vb, vto;
    Point cr1, cr2;
    va = this.from.vector_to(a);
    vb = this.from.vector_to(b);
    vto = this.from.vector_to(this.to);
    va.z = 0;
    vb.z = 0;
    vto.z = 0;
    cr1 = vto.cross(va);
    cr2 = vto.cross(vb);
    return cr1.z > 0 == cr2.z > 0;
  }

  Point closest_point(Point p) {
    Point proj;
    Point v;
    float m;
    v = this.from.vector_to(this.to);
    proj = this.from.vector_to(p).project_onto(v);
    m = proj.mag_in_terms_of(v);
    if (m < 0) {
      return new Point(this.from);
    } else if (m > 1.0) {
      return new Point(this.to);
    } else {
      proj.add(this.from);
      return proj;
    }
  }
}

class Triangle {
  Point a, b, c;
  Triangle(Point a, Point b, Point c) {
    this.a = a;
    this.b = b;
    this.c = c;
  }

  Triangle flattened() {
    return new Triangle(
      new Point(this.a.x, this.a.y, 0),
      new Point(this.b.x, this.b.y, 0),
      new Point(this.c.x, this.c.y, 0)
    );
  }

  boolean xy_contains(Point p) {
    // Ignores z.
    Point b = xy__barycentric(p);
    return (b.x >= 0 && b.y >= 0 && b.z >= 0);
  }

  Point xy__barycentric(Point p) {
    // Ignores z.
    float det, b1, b2, b3;
    det = (
      (this.b.y - this.c.y) * (this.a.x - this.c.x)
    +
      (this.c.x - this.b.x) * (this.a.y - this.c.y)
    );

    b1 = (
      (this.b.y - this.c.y) * (p.x - this.c.x)
    +
      (this.c.x - this.b.x) * (p.y - this.c.y)
    );
    b1 /= det;

    b2 = (
      (this.c.y - this.a.y) * (p.x - this.c.x)
    +
      (this.a.x - this.c.x) * (p.y - this.c.y)
    );
    b2 /= det;

    b3 = 1 - b1 - b2;
    return new Point(b1, b2, b3);
  }

  Point barycentric__xyz(Point b) {
    return new Point(
      b.x * this.a.x + b.y * this.b.x + b.z * this.c.x,
      b.x * this.a.y + b.y * this.b.y + b.z * this.c.y,
      b.x * this.a.z + b.y * this.b.z + b.z * this.c.z
    );
  }
}

class Map {
  Triangle triangles[];
  Point points[];
  Point forces[];
  int avgcounts[];
  Point center;
  int width;
  int height;
  Map(int width, int height) {
    int i, j, idx;
    float x, y, z;
    Triangle t;
    Point a, b, c, d;
    this.width = width;
    this.height = height;
    this.triangles = new Triangle[width*height];
    this.points = new Point[this.pwidth()*this.pheight()];
    this.forces = new Point[this.pwidth()*this.pheight()];
    this.avgcounts = new int[this.pwidth()*this.pheight()];
    this.center = new Point(0, 0, 0);
    // Set up points:
    for (i = 0; i < this.pwidth(); ++i) {
      for (j = 0; j < this.pheight(); ++j) {
        idx = this.pidx(i, j);
        x = i * GRID_RESOLUTION;
        y = j * SG_HEIGHT * GRID_RESOLUTION;
        if (j % 2 == 1) {
          x += SG_OFFSET * GRID_RESOLUTION;
        }
        z = 0;
        this.points[idx] = new Point(x, y, z);
        this.forces[idx] = new Point(0, 0, 0);
        this.avgcounts[idx] = 0;
      }
    }
    // Set up triangles:
    for (i = 0; i < this.width; ++i) {
      for (j = 0; j < this.height; ++j) {
        idx = this.tidx(i, j);
        a = this.points[this.pidx_a(i, j)];
        b = this.points[this.pidx_b(i, j)];
        c = this.points[this.pidx_c(i, j)];
        this.triangles[idx] = new Triangle(a, b, c);
      }
    }
    this.update_center();
  }

  int tidx(int i, int j) { return j*this.width+i; }
  int pidx(int i, int j) { return j*this.pwidth()+i; }
  int pwidth() { return (this.width/2) + 1; }
  int pheight() { return this.height + 1; }
  int pidx_a(int i, int j) {
    if (j % 2 == 0) {
      return this.pidx((i+1)/2, j);
    } else {
      return this.pidx(i/2, j);
    }
  }
  int pidx_b(int i, int j) {
    if (j % 2 == 0) {
      return this.pidx(i/2, j+1);
    } else {
      return this.pidx((i+1)/2, j+1);
    }
  }
  int pidx_c(int i, int j) {
    boolean points_up = i % 2 == j % 2;
    if (points_up) {
      return this.pidx(i/2+1, j);
    } else {
      return this.pidx(i/2+1, j+1);
    }
  }

  void update_center() {
    int i, j, idx;
    this.center.x = 0;
    this.center.y = 0;
    this.center.z = 0;
    for (i = 0; i < this.pwidth(); ++i) {
      for (j = 0; j < this.pheight(); ++j) {
        idx = this.pidx(i, j);
        this.center.add(this.points[idx]);
      }
    }
    this.center.scale(1.0 / (float) (this.pwidth()*this.pheight()));
  }

  float min_x() {
    int i, j, idx;
    float result = this.points[0].x;
    for (i = 0; i < this.pwidth(); ++i) {
      for (j = 0; j < this.pheight(); ++j) {
        idx = this.pidx(i, j);
        if (this.points[idx].x < result) {
          result = this.points[idx].x;
        }
      }
    }
    return result;
  }

  float max_x() {
    int i, j, idx;
    float result = this.points[0].x;
    for (i = 0; i < this.pwidth(); ++i) {
      for (j = 0; j < this.pheight(); ++j) {
        idx = this.pidx(i, j);
        if (this.points[idx].x > result) {
          result = this.points[idx].x;
        }
      }
    }
    return result;
  }

  float min_y() {
    int i, j, idx;
    float result = this.points[0].y;
    for (i = 0; i < this.pwidth(); ++i) {
      for (j = 0; j < this.pheight(); ++j) {
        idx = this.pidx(i, j);
        if (this.points[idx].y < result) {
          result = this.points[idx].y;
        }
      }
    }
    return result;
  }

  float max_y() {
    int i, j, idx;
    float result = this.points[0].y;
    for (i = 0; i < this.pwidth(); ++i) {
      for (j = 0; j < this.pheight(); ++j) {
        idx = this.pidx(i, j);
        if (this.points[idx].y > result) {
          result = this.points[idx].y;
        }
      }
    }
    return result;
  }

  float min_z() {
    int i, j, idx;
    float result = this.points[0].z;
    for (i = 0; i < this.pwidth(); ++i) {
      for (j = 0; j < this.pheight(); ++j) {
        idx = this.pidx(i, j);
        if (this.points[idx].z < result) {
          result = this.points[idx].z;
        }
      }
    }
    return result;
  }

  float max_z() {
    int i, j, idx;
    float result = this.points[0].z;
    for (i = 0; i < this.pwidth(); ++i) {
      for (j = 0; j < this.pheight(); ++j) {
        idx = this.pidx(i, j);
        if (this.points[idx].z > result) {
          result = this.points[idx].z;
        }
      }
    }
    return result;
  }

  void stretch(float width, float height) {
    // Stretches the grid to a specified minimum width/height
    this.update_center();
    float min_x, max_x, min_y, max_y;
    min_x = this.center.x - width/2.0;
    max_x = this.center.x + width/2.0;
    min_y = this.center.y - height/2.0;
    max_y = this.center.y + height/2.0;
    int i, j;
    // First, pull all edge points straight outwards to meed the desired
    // minimum dimensions as necessary:
    for (i = 0; i < this.pwidth(); ++i) {
      if (this.points[this.pidx(i, 0)].y > min_y) {
        this.points[this.pidx(i, 0)].y = min_y;
      }
      if (this.points[this.pidx(i, this.pheight() - 1)].y < max_y) {
        this.points[this.pidx(i, this.pheight() - 1)].y = max_y;
      }
    }
    for (j = 0; j < this.pheight(); ++j) {
      if (this.points[pidx(0, j)].x > min_x) {
        this.points[pidx(0, j)].x = min_x;
      }
      if (this.points[pidx(this.pwidth() - 1, j)].x < max_x) {
        this.points[pidx(this.pwidth() - 1, j)].x = max_x;
      }
    }
    // Next, settle down the inner points holding edge points in place:
    this.settle(DEFAULT_SETTLE_STEPS, true);
  }

  void rustle(float strength, float size, float seed) {
    // Randomly moves each grid point a little bit in the x/y plane. Strength
    // represents the max perturbation as a percentage of the default
    // between-point distance.
    float s = strength*GRID_RESOLUTION;
    Point p;
    int i, j;
    for (i = 0; i < this.pwidth(); ++i) {
      for (j = 0; j < this.pheight(); ++j) {
        p = this.points[this.pidx(i, j)];
        p.x += s * pnoise(p.x*size, p.y*size, seed);
        p.y += s * pnoise(p.x*size + 11000.0, p.y*size, seed);
      }
    }
  }

  void jiggle(
    float strength,
    float size,
    float distortion,
    float dsize,
    boolean only_up,
    float seed
  ) {
    // Randomly moves each grid point up or down a little (or only upwards if
    // only_up is given).
    float s = strength * GRID_RESOLUTION;
    float ds = distortion * GRID_RESOLUTION;
    float n;
    Point p;
    int i, j;
    float dx, dy;
    for (i = 0; i < this.pwidth(); ++i) {
      for (j = 0; j < this.pheight(); ++j) {
        p = this.points[this.pidx(i, j)];
        dx = ds * pnoise(dsize * p.x, dsize * p.y + 1400);
        dy = ds * pnoise(dsize * p.x, dsize * p.y + 1700);
        n = pnoise(p.x*size + dx + 1100.0, p.y*size + dy, seed);
        if (only_up) {
          n = 1 + n / 2.0;
        }
        p.z += s*n;
      }
    }
  }

  void continents(float strength, float scale, float seed) {
    float s = strength * GRID_RESOLUTION;
    Point p;
    int i, j;
    float fx, fy;
    float elevate;
    for (i = 0; i < this.pwidth(); ++i) {
      for (j = 0; j < this.pheight(); ++j) {
        p = this.points[this.pidx(i, j)];
        fx = (
          p.x
        +
          0.3 * this.pwidth() * GRID_RESOLUTION * pnoise(
            p.x * scale,
            p.y * scale + 170,
            seed + 1345
          )
        );
        fy = (
          p.y
        +
          0.3 * this.pheight() * GRID_RESOLUTION * pnoise(
            p.x * scale,
            p.y * scale - 60,
            seed + 1764
          )
        );
        // base height:
        elevate = (
          sin(2.0 * PI * fx / (this.pwidth() * GRID_RESOLUTION))
        *
          cos(2.0 * PI * fy / (this.pheight() * GRID_RESOLUTION))
        );
        p.z += elevate * s;
      }
    }
  }

  void settle(int iterations, boolean fix_edges) {
    // Slowly settles the graph by pretending that each grid line is a spring.
    Triangle t;
    Point p, o, f;
    int iter, i, j, idx;
    for (iter = 0; iter < iterations; ++iter) {
      // First pass: compute forces.
      for (i = 0; i < this.width; ++i) {
        for (j = 0; j < this.height; ++j) {
          t = this.triangles[this.tidx(i, j)];
          boolean points_up = i % 2 == j % 2;
          if (points_up) {
            // All six forces on all three corners of this triangle:
            // a <- b
            p = t.a;
            o = t.b;
            f = this.forces[this.pidx_a(i, j)];
            f.add(p.spring_force_from(o, SETTLE_DISTANCE, SETTLE_K));
            // a <- c
            o = t.c;
            f.add(p.spring_force_from(o, SETTLE_DISTANCE, SETTLE_K));
            // b <- a
            p = t.b;
            o = t.a;
            f = this.forces[this.pidx_b(i, j)];
            f.add(p.spring_force_from(o, SETTLE_DISTANCE, SETTLE_K));
            // b <- c
            o = t.c;
            f.add(p.spring_force_from(o, SETTLE_DISTANCE, SETTLE_K));
            // c <- a
            p = t.c;
            o = t.a;
            f = this.forces[this.pidx_c(i, j)];
            f.add(p.spring_force_from(o, SETTLE_DISTANCE, SETTLE_K));
            // c <- b
            o = t.b;
            f.add(p.spring_force_from(o, SETTLE_DISTANCE, SETTLE_K));
          } else if (i == 0 && (j % 2 == 1)) {
            // Both forces on our a <-> b edge:
            // a <- b
            p = t.a;
            o = t.b;
            f = this.forces[this.pidx_a(i, j)];
            f.add(p.spring_force_from(o, SETTLE_DISTANCE, SETTLE_K));
            // b <- a
            p = t.b;
            o = t.a;
            f = this.forces[this.pidx_b(i, j)];
            f.add(p.spring_force_from(o, SETTLE_DISTANCE, SETTLE_K));
          } else if ((i == this.width - 1) && (j % 2 == 0) ) {
            // Both forces on our a <-> c edge:
            // a <- c
            p = t.a;
            o = t.c;
            f = this.forces[this.pidx_a(i, j)];
            f.add(p.spring_force_from(o, SETTLE_DISTANCE, SETTLE_K));
            // c <- a
            p = t.c;
            o = t.a;
            f = this.forces[this.pidx_c(i, j)];
            f.add(p.spring_force_from(o, SETTLE_DISTANCE, SETTLE_K));
          }
        }
      }
      // Second pass: apply forces.
      for (i = 0; i < this.pwidth(); ++i) {
        for (j = 0; j < this.pheight(); ++j) {
          if (
            fix_edges
          &&
            (i == 0 || i == this.pwidth()-1 || j == 0 || j == this.pheight()-1)
          ) {
            this.forces[this.pidx(i, j)].scale(0);
            continue;
          }
          idx = this.pidx(i, j);
          p = this.points[idx];
          f = this.forces[idx];
          f.scale(SETTLE_DT);
          p.add(f);
          f.scale(0);
        }
      }
    }
  }

  void untangle(int iterations, boolean fix_edges) {
    // Untangles the graph by moving points towards the average of their
    // neighbors.
    Triangle t;
    Point p, f, v;
    int iter, i, j, idx;
    for (iter = 0; iter < iterations; ++iter) {
      // First pass: compute averages and store them in the force slots.
      for (i = 0; i < this.width; ++i) {
        for (j = 0; j < this.height; ++j) {
          t = this.triangles[this.tidx(i, j)];
          boolean points_up = i % 2 == j % 2;
          if (points_up) {
            // All six relations on all three corners of this triangle:
            // a <- b; a <- c
            idx = this.pidx_a(i, j);
            this.forces[idx].add(t.b);
            this.forces[idx].add(t.c);
            this.avgcounts[idx] += 2;
            // b <- a; b <- c
            idx = this.pidx_b(i, j);
            this.forces[idx].add(t.a);
            this.forces[idx].add(t.c);
            this.avgcounts[idx] += 2;
            // c <- a; c <- b
            idx = this.pidx_c(i, j);
            this.forces[idx].add(t.a);
            this.forces[idx].add(t.b);
            this.avgcounts[idx] += 2;
          } else if (i == 0 && (j % 2 == 1)) {
            // Both relations on our a <-> b edge:
            // a <- b
            idx = this.pidx_a(i, j);
            this.forces[idx].add(t.b);
            this.avgcounts[idx] += 1;
            // b <- a
            idx = this.pidx_b(i, j);
            this.forces[idx].add(t.a);
            this.avgcounts[idx] += 1;
          } else if ((i == this.width - 1) && (j % 2 == 0) ) {
            // Both forces on our a <-> c edge:
            // a <- c
            idx = this.pidx_a(i, j);
            this.forces[idx].add(t.c);
            this.avgcounts[idx] += 1;
            // c <- a
            idx = this.pidx_c(i, j);
            this.forces[idx].add(t.a);
            this.avgcounts[idx] += 1;
          }
        }
      }
      // Second pass: move each point towards its respective average
      for (i = 0; i < this.pwidth(); ++i) {
        for (j = 0; j < this.pheight(); ++j) {
          if (
            fix_edges
          &&
            (i == 0 || i == this.pwidth()-1 || j == 0 || j == this.pheight()-1)
          ) {
            this.forces[this.pidx(i, j)].scale(0);
            this.avgcounts[this.pidx(i, j)] = 0;
            continue;
          }
          idx = this.pidx(i, j);
          p = this.points[idx];
          f = this.forces[idx];
          f.scale((1 / (float) (this.avgcounts[idx])));
          v = p.vector_to(f);
          v.scale(UNTANGLE_DT);
          // Add only in the x/y plane:
          p.x += v.x;
          p.y += v.y;
          f.scale(0);
          this.avgcounts[idx] = 0;
        }
      }
    }
  }

  void seam(LineSegment seg, float width, boolean pull, boolean fix_edges) {
    int i, j, idx;
    Point p, f, cl, v;
    float d, str;
    // First pass: compute push/pull vectors
    for (i = 0; i < this.pwidth(); ++i) {
      for (j = 0; j < this.pheight(); ++j) {
        idx = this.pidx(i, j);
        p = this.points[idx];
        f = this.forces[idx];
        cl = seg.closest_point(p);
        v = cl.vector_to(p);
        d = v.magnitude();
        str = pow((width - d) / width, 0.6);
        if (d > width) {
          str = 0;
        }
        str = sigmoid(str);
        v.scale(str);
        if (pull) {
          v.scale(-1);
        }
        f.add(v);
      }
    }
    // Second pass: move each point towards its respective average
    for (i = 0; i < this.pwidth(); ++i) {
      for (j = 0; j < this.pheight(); ++j) {
        if (
          fix_edges
        &&
          (i == 0 || i == this.pwidth()-1 || j == 0 || j == this.pheight()-1)
        ) {
          this.forces[this.pidx(i, j)].scale(0);
          continue;
        }
        idx = this.pidx(i, j);
        p = this.points[idx];
        f = this.forces[idx];
        f.scale(SEAM_DT);
        p.add(f);
        f.scale(0);
      }
    }
  }

  void popup() {
    // Pops up the grid to ensure that the lowest point is at z=0.
    Point p;
    int i, j;
    for (i = 0; i < this.pwidth(); ++i) {
      for (j = 0; j < this.pheight(); ++j) {
        p = this.points[this.pidx(i, j)];
        if (p.z < 0) {
          p.z = 0;
        }
      }
    }
  }

  void render_wireframe() {
    int i, j;
    Triangle t;
    Point p, o;
    for (i = 0; i < this.width; ++i) {
      for (j = 0; j < this.height; ++j) {
        t = this.triangles[this.tidx(i, j)];
        boolean points_up = i % 2 == j % 2;
        if (points_up) {
          // All of this triangle:
          triangle(t.a.x, t.a.y, t.b.x, t.b.y, t.c.x, t.c.y);
        } else {
          if (i == 0 && (j % 2 == 1)) {
            // left edge
            line(t.a.x, t.a.y, t.b.x, t.b.y);
          }
          if ((i == this.width - 1) && (j % 2 == 0) ) {
            // right edge:
            line(t.a.x, t.a.y, t.c.x, t.c.y);
          }
          if (j == this.height - 1) {
            // top:
            line(t.b.x, t.b.y, t.c.x, t.c.y);
          }
        }
      }
    }
  }

  void render(
    float[] heights,
    float[] precipitation,
    float seed
  ) {
    float min_x, max_x, min_y, max_y, min_z, max_z;
    min_x = this.min_x();
    max_x = this.max_x();
    min_y = this.min_y();
    max_y = this.max_y();
    min_z = this.min_z();
    max_z = this.max_z();
    float w = max_x - min_x;
    float h = max_y - min_y;
    float d = max_z - min_z;
    int i, j;
    float fx, fy;
    float dfx, dfy;
    float bh, th; // base and terrain heights
    Point p;
    for (i = 0; i < ARRAY_WIDTH; ++i) {
      for (j = 0; j < ARRAY_HEIGHT; ++j) {
        fx = i / (float) (ARRAY_WIDTH);
        fy = j / (float) (ARRAY_HEIGHT);
        /*
        // base height distortion:
        dfx = fx + 0.3 * pnoise(fx * 5.0, fy * 4.0 + 170, seed + 7182);
        dfy = fy + 0.3 * pnoise(fx * 5.0, fy * 4.0 - 60, seed + 5468);
        // base height:
        bh = (
          sin(2.0 * PI * dfx)
        *
          sin(2.0 * PI * dfy)
        );
        bh = (1 + bh) / 2.0;
        // distortion:
        dfx = fx + 0.022 * pnoise(fx * 14.0, fy * 14.0, seed);
        dfy = fy + 0.022 * pnoise(fx * 14.0, fy * 14.0 + 1110.0, seed);
        fx = min_x + w*dfx;
        fy = min_y + h*dfy;
        */
        p = new Point(fx, fy, 0);
        th = (this.get_height(p) - min_z) / d;
        heights[i + j * ARRAY_WIDTH] = th;
        precipitation[i + j * ARRAY_WIDTH] = (
          0.25
        + 0.15 * pnoise(i * 0.05, j * 0.05)
        // + 0.6 * pow((th + bh) / 2.0, 0.4)
        + 0.6 * pow(th, 0.4)
        );
      }
    }
  }

  float get_height(Point p) {
    int i, j;
    Triangle t;
    Point bary, r;
    for (i = 0; i < this.width; ++i) {
      for (j = 0; j < this.height; ++j) {
        t = this.triangles[this.tidx(i, j)];
        if (t.xy_contains(p)) {
          bary = t.xy__barycentric(p);
          r = t.barycentric__xyz(bary);
          return r.z;
        }
      }
    }
    return 0.0;
  }
}



void reset_array(float[] array) {
  int i, j, idx;
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      array[idx] = 0;
    }
  }
}

void copy_array(float[] from, float[] to) {
  int i, j, idx;
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      to[idx] = from[idx];
    }
  }
}

void scale_array(float[] array, float scale) {
  int i, j, idx;
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      array[idx] *= scale;
    }
  }
}

void add_to_array(float[] array, float addend) {
  int i, j, idx;
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      array[idx] += addend;
    }
  }
}

void average_arrays(float[] main, float[] add_in, float main_weight) {
  int i, j, idx;
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      main[idx] = (main[idx]*main_weight + add_in[idx]) / (1.0 + main_weight);
    }
  }
}

void add_arrays(float[] main, float[] add_in) {
  int i, j, idx;
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      main[idx] += add_in[idx];
    }
  }
}

void normalize_array(float[] array) {
  int i, j, idx;
  float max = array[0], min = array[0];
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      if (array[idx] > max) { max = array[idx]; }
      if (array[idx] < min) { min = array[idx]; }
    }
  }
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      array[idx] = (array[idx] - min) / (max - min);
    }
  }
}

void blur_array(float[] array, float[] tmp) {
  int i, j, idx;
  float total;
  float divisor;
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      total  = array[idx] * BLUR_BIAS;
      divisor = BLUR_BIAS;
      if (i > 0) {
        total += array[idx - 1];
        divisor += 1;
      }
      if (i < ARRAY_WIDTH - 1) {
        total += array[idx + 1];
        divisor += 1;
      }
      if (j > 0) {
        total += array[idx - ARRAY_WIDTH];
        divisor += 1;
      }
      if (j < ARRAY_HEIGHT - 1) {
        total += array[idx + ARRAY_WIDTH];
        divisor += 1;
      }
      tmp[idx] = total / divisor;
    }
  }
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      array[idx] = tmp[idx];
    }
  }
}

void slump_array(float[] array, float[] tmp, float max_slope, float rate) {
  int i, j, idx, lnidx;
  float ln, midpoint;
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      tmp[idx] = 0;
    }
  }
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      ln = array[idx];
      lnidx = idx;
      if (i > 0 && array[idx - 1] < ln) {
        ln = array[idx - 1];
        lnidx = idx - 1;
      }
      if (i < ARRAY_WIDTH - 1 && array[idx + 1] < ln) {
        ln = array[idx + 1];
        lnidx = idx + 1;
      }
      if (j > 0 && array[idx - ARRAY_WIDTH] < ln) {
        ln = array[idx - ARRAY_WIDTH];
        lnidx = idx - ARRAY_WIDTH;
      }
      if (j < ARRAY_HEIGHT - 1 && array[idx + ARRAY_WIDTH] < ln) {
        ln = array[idx + ARRAY_WIDTH];
        lnidx = idx + ARRAY_WIDTH;
      }
      if (lnidx != idx && array[idx] - ln > max_slope) {
        midpoint = ln + (array[idx] - ln) / 2.0;
        tmp[idx] -= array[idx] - (midpoint + max_slope/2.0);
        tmp[lnidx] += (midpoint - max_slope/2.0) - ln;
      }
    }
  }
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      array[idx] += tmp[idx] * rate;
    }
  }
}

void noise_array(
  float[] array,
  float str,
  float scale,
  float dstr,
  float dscale,
  float seed
) {
  int i, j, idx;
  float fx, fy;
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      fx = i / (float) ARRAY_HEIGHT; // intentional
      fy = j / (float) ARRAY_HEIGHT;
      idx = i + ARRAY_WIDTH * j;
      array[idx] += str * pnoise(
        scale * fx + dstr * pnoise(
          dscale * fx + 90 * seed,
          dscale * fy + 7000
        ) + 2700,
        scale * fy + dstr * pnoise(
          dscale * fx + 70 * seed,
          dscale * fy + 11000
        ) + 110 * seed
      );
    }
  }
}

boolean run_particle(float[] array, float height, float slip) {
  int px = floor(random(0, ARRAY_WIDTH));
  int py = floor(random(0, ARRAY_HEIGHT));
  int i, j, idx, ndir, nx, ny;
  boolean valid;
  IntList directions = new IntList();
  directions.append(0);
  directions.append(1);
  directions.append(2);
  directions.append(3);
  for (i = 0; i < MAX_WANDER_STEPS; ++i) {
    idx = px + py*ARRAY_WIDTH;
    if (px > 0 && array[idx - 1] >= height) {
      slip -= 1;
      if (slip <= 0) { break; }
    }
    if (px < ARRAY_WIDTH - 1 && array[idx + 1] >= height) {
      slip -= 1;
      if (slip <= 0) { break; }
    }
    if (py > 0 && array[idx - ARRAY_WIDTH] >= height) {
      slip -= 1;
      if (slip <= 0) { break; }
    }
    if (py < ARRAY_HEIGHT - 1 && array[idx + ARRAY_WIDTH] >= height) {
      slip -= 1;
      if (slip <= 0) { break; }
    }

    nx = px;
    ny = py;
    directions.shuffle();
    valid = false;

    for (j = 0; j < directions.size(); ++j) {
      ndir = directions.get(j);
      if (ndir == 0) {
        nx = px - 1;
        ny = py;
      } else if (ndir == 1) {
        nx = px + 1;
        ny = py;
      } else if (ndir == 2) {
        nx = px;
        ny = py - 1;
      } else if (ndir == 3) {
        nx = px;
        ny = py + 1;
      } else {
        println("Bad direction!");
      }
      if (nx < 0 || nx > ARRAY_WIDTH - 1 || ny < 0 || ny > ARRAY_HEIGHT - 1) {
        continue;
      }
      idx = nx + ny*ARRAY_WIDTH;
      if (array[idx] < height) {
        valid = true;
        break;
      }
    }

    if (!valid) {
      break;
    }
    px = nx;
    py = ny;
  }
  idx = px + py*ARRAY_WIDTH;
  if (array[idx] < height) {
    array[idx] = height;
    return true;
  } else {
    return false;
  }
}

void build_mountains(float[] array, float[] tmp, float[] save) {
  int i, j;
  float height = PARTICLE_HEIGHT;
  reset_array(array);
  println("Starting build cycles...");
  for (i = 0; i < BUILD_CYCLES; ++i) {
    height *= 0.95;
    for (j = 0; j < STEP_PARTICLES; ++j) {
      run_particle(array, height, SLIP);
    }
    print("\r  ...build cycle " + (i+1) + "/" + BUILD_CYCLES + " completed...");
  }
  println("\n  ...done with build cycles.");
  copy_array(array, save);
  for (j = 0; j < SLUMP_STEPS; ++j) {
    slump_array(array, tmp, MAX_SLOPE, SLUMP_RATE);
    average_arrays(array, save, 20.0);
  }
  for (j = 0; j < FINAL_SLUMPS; ++j) {
    slump_array(array, tmp, MAX_SLOPE, SLUMP_RATE);
  }
}

void map_flows(
  int steps,
  float[] height,
  float[] precipitation,
  float[] flow,
  float[] water,
  float[] tmp
) {
  int n, i, j, ii, jj, idx, oidx;
  int[] obest = new int[3];
  float[] bslopes = new float[3];
  float h;
  float slope;
  float stotal;
  float diff;
  float[] outputs = new float[9];
  float output_coefficient;
  // Start with precipitation:
  copy_array(precipitation, flow);
  copy_array(precipitation, water);
  reset_array(tmp);
  for (n = 0; n < steps; ++n) {
    for (i = 0; i < ARRAY_WIDTH; ++i) {
      for (j = 0; j < ARRAY_HEIGHT; ++j) {
        idx = i + ARRAY_WIDTH * j;
        h = height[idx];

        // First, iterate over neighbors to find our 3 most-downhill neighbors:
        oidx = 0;
        obest[0] = -1;
        obest[1] = -1;
        obest[2] = -1;
        bslopes[0] = 0;
        bslopes[1] = 0;
        bslopes[2] = 0;
        slope = 0;
        stotal = 0;
        for (ii = i - 1; ii <= i + 1; ++ii) {
          for (jj = j - 1; jj <= j + 1; ++jj) {
            if ( // Ignore the center & any out-of-bounds:
              (ii == i && jj == j)
            || ii < 0
            || ii > ARRAY_WIDTH - 1
            || jj < 0
            || jj > ARRAY_HEIGHT - 1
            ) {
              continue;
            }
            oidx = ii + ARRAY_WIDTH * jj;
            slope = h - height[oidx];
            if (slope > bslopes[0]) {
              obest[2] = obest[1];
              bslopes[2] = bslopes[1];
              obest[1] = obest[0];
              bslopes[1] = bslopes[0];
              obest[0] = oidx;
              bslopes[0] = slope;
            } else if (slope > bslopes[1]) {
              obest[2] = obest[1];
              bslopes[2] = bslopes[1];
              obest[1] = oidx;
              bslopes[1] = slope;
            } else if (slope > bslopes[2]) {
              obest[2] = oidx;
              bslopes[2] = slope;
            }
          }
        }

        // Compute sum of best slopes:
        for (ii = 0; ii < 3; ++ii) {
          stotal += bslopes[ii];
        }

        // Redistribute water downstream (If we're at a local minimum, no water
        // leaves):
        if (stotal > 0) {
          // Local water flows onwards to three most-downhill neighbors
          // proportional to their slopes:
          for (ii = 0; ii < 3; ++ii) {
            if (bslopes[ii] == 0) {
              continue;
            }
            if (tmp[obest[ii]] >= 0) {
              tmp[obest[ii]] += water[idx] * (bslopes[ii] / stotal);
            }
          }
        }
      }
    }
    // Flip buffers and accumulate:
    for (i = 0; i < ARRAY_WIDTH; ++i) {
      for (j = 0; j < ARRAY_HEIGHT; ++j) {
        idx = i + ARRAY_WIDTH * j;
        flow[idx] += water[idx];
        water[idx] = tmp[idx];
        tmp[idx] = 0;
      }
    }
  }
}


Map THE_MAP;

float[] SHOW;

float[] HEIGHT;
float[] PRECIP;
float[] FLOW;

float[] TMP;
float[] TMP2;
float[] SAVE;

void setup() {
  int i;

  randomSeed(17);
  // TODO: some way of seeding the Perlin library?

  colorMode(HSB, 1.0, 1.0, 1.0);
  size(WINDOW_WIDTH, WINDOW_HEIGHT);

  THE_MAP = new Map(ARRAY_WIDTH, ARRAY_HEIGHT);

  SHOW = new float[ARRAY_WIDTH*ARRAY_HEIGHT];
  HEIGHT = new float[ARRAY_WIDTH*ARRAY_HEIGHT];
  PRECIP = new float[ARRAY_WIDTH*ARRAY_HEIGHT];
  FLOW = new float[ARRAY_WIDTH*ARRAY_HEIGHT];
  TMP = new float[ARRAY_WIDTH*ARRAY_HEIGHT];
  TMP2 = new float[ARRAY_WIDTH*ARRAY_HEIGHT];
  SAVE = new float[ARRAY_WIDTH*ARRAY_HEIGHT];
  reset_array(SHOW);
  reset_array(HEIGHT);
  reset_array(FLOW);
  reset_array(TMP);
  reset_array(TMP2);
  reset_array(SAVE);

  build_world(173.4);

  noLoop();
  redraw();
}

void build_world(float seed) {
  int i;
  randomSeed((int) seed);

  reset_array(HEIGHT);
  reset_array(PRECIP);
  noise_array(
    PRECIP,
    PRECIP_BASE_STR,
    PRECIP_BASE_SCALE,
    PRECIP_BASE_DSTR,
    PRECIP_BASE_DSCALE,
    seed
  );
  noise_array(
    PRECIP,
    PRECIP_EXTRA_STR,
    PRECIP_EXTRA_SCALE,
    PRECIP_EXTRA_DSTR,
    PRECIP_EXTRA_DSCALE,
    seed
  );
  normalize_array(PRECIP);
  scale_array(PRECIP, 0.5);
  add_to_array(PRECIP, 0.5);
  reset_array(FLOW);
  build_mountains(HEIGHT, TMP, SAVE);
  map_flows(FLOW_MAP_STEPS, HEIGHT, PRECIP, FLOW, TMP, TMP2);
  normalize_array(FLOW);
}


void draw() {
  int i, j, idx;
  background(0.6, 0.8, 0.25);
  noStroke();

  int cw = WINDOW_WIDTH / ARRAY_WIDTH;
  int ch = WINDOW_HEIGHT / ARRAY_HEIGHT;

  if (DISPLAY == "height") {
    copy_array(HEIGHT, SHOW);
  } else if (DISPLAY == "precipitation") {
    copy_array(PRECIP, SHOW);
  } else if (DISPLAY == "flow") {
    copy_array(FLOW, SHOW);
  }

  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + j * ARRAY_WIDTH;
      fill(colormap(SHOW[idx]));
      rect(i*cw, j*ch, cw, ch);
    }
  }
}

void keyPressed() {
  int i;
  if (key == 'q') {
    exit();
  } else if (key == 'r') {
    reset_array(HEIGHT);
  } else if (key == 'p') {
    run_particle(HEIGHT, PARTICLE_HEIGHT, SLIP);
  } else if (key == 'P') {
    for (i = 0; i < EXTRA_PARTICLES; ++i) {
      run_particle(HEIGHT, PARTICLE_HEIGHT, SLIP);
    }
  } else if (key == 'b') {
    blur_array(HEIGHT, TMP);
  } else if (key == 's') {
    slump_array(HEIGHT, TMP, MAX_SLOPE, SLUMP_RATE);
  } else if (key == 'f') {
    map_flows(FLOW_MAP_STEPS, HEIGHT, PRECIP, FLOW, TMP, TMP2);
    normalize_array(FLOW);
  } else if (key == 'F') {
    map_flows(FLOW_MAP_STEPS, HEIGHT, PRECIP, FLOW, TMP, TMP2);
    normalize_array(FLOW);
    for (i = 0; i < SLUMP_FLOW_STEPS; ++i) {
      slump_array(FLOW, TMP, FLOW_MAX_SLOPE, FLOW_SLUMP_RATE);
    }
    scale_array(FLOW, EROSION_STRENGTH);
    add_arrays(HEIGHT, FLOW);
    normalize_array(HEIGHT);
  } else if (key == 'm') {
    build_world(random(1000));
  } else if (key == 'j') {
    SEA_LEVEL -= 0.05;
  } else if (key == 'k') {
    SEA_LEVEL += 0.05;
  } else if (key == 'c') {
    if (COLORMODE == "grayscale") {
      COLORMODE = "false";
    } else {
      COLORMODE = "grayscale";
    }
  } else if (key == '1') {
    DISPLAY = "height";
  } else if (key == '2') {
    DISPLAY = "precipitation";
  } else if (key == '3') {
    DISPLAY = "flow";
  }
  redraw();
}
