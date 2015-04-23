// vim: syn=java

import perlin.*;

Perlin pnl = new Perlin(this);

int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 640;

int ARRAY_WIDTH = 200;
int ARRAY_HEIGHT = 160;

int GRID_WIDTH = 160;
int GRID_HEIGHT = 64;

//int GRID_WIDTH = 80;
//int GRID_HEIGHT = 30;

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

float SETTLE_K = 1.6;
float SETTLE_DISTANCE = GRID_RESOLUTION * 1.3;
float SETTLE_DT = 0.01;

float UNTANGLE_DT = 0.1;

float SEAM_DT = 0.15;
float MIN_SEAM_WIDTH = GRID_RESOLUTION * 4.3;
float MAX_SEAM_WIDTH = GRID_RESOLUTION * 12.1;

int DEFAULT_UNTANGLE_STEPS = 5;
int DEFAULT_SETTLE_STEPS = 20;

float CONTINENT_STRENGTH = 0.06;

float TECTONIC_DISTORTION = 0.052;

int N_TECTONIC_SEAMS = 20;
int N_TECTONIC_RUSTLE_SETTLE_STEPS = 4;

float NEGATIVE_SHRINK_AMOUNT = 26;

// Reaction-diffusion system parameters:

int BUILD_CYCLES = 50;
int STEP_PARTICLES = 300;
int EXTRA_PARTICLES = 10;
int EXTRA_EXTRA_PARTICLES = 2;
float HEIGHT_FALLOFF = 0.9;
int SLIP = 3;
float PARTICLE_HEIGHT = 1.0;

float BLUR_BIAS = 8;
float SHARPEN_BIAS = 12;
float MAX_SLOPE = 0.01;
float SLUMP_RATE = 0.2;

int SLUMP_STEPS = 24;
int FINAL_SLUMPS = 4;

int MAX_WANDER_STEPS = 20000;

float TECTONICS_PARTICLE_BASE_HEIGHT = 0.9;
int TECTONICS_PARTICLE_MAX_WANDER = 100000;
int TECTONICS_PARTICLE_SLIP = 3;
float TECTONICS_PARTICLE_HEIGHT_FALLOFF = 0.83;
int TECTONICS_PARTICLE_WAVES = 27;
int TECTONICS_PARTICLE_WAVE_SIZE = 90;
int TECTONICS_PARTICLE_WAVE_GROWTH = 20;
int TECTONICS_PARTICLE_WAVE_GROWTH_GROWTH = 4;
int TECTONICS_MOUNTAINS_SLUMP_STEPS = 5;
int TECTONICS_MOUNTAINS_FINAL_SLUMPS = 1;

// Precipitation & flow parameters:

float PRECIP_BASE_STR = 0.5;
float PRECIP_BASE_SCALE = 8.5;
float PRECIP_BASE_DSTR = 0.1;
float PRECIP_BASE_DSCALE = 5.4;

float PRECIP_EXTRA_STR = 0.3;
float PRECIP_EXTRA_SCALE = 3.7;
float PRECIP_EXTRA_DSTR = 0.08;
float PRECIP_EXTRA_DSCALE = 6.2;

int FLOW_MAP_STEPS = 6;
int SLUMP_FLOW_STEPS = 6;
float EROSION_STRENGTH = 0.2;
float FLOW_MAX_SLOPE = 0.2;
float FLOW_SLUMP_RATE = 0.3;
int SBLUR_STEPS = ARRAY_WIDTH * ARRAY_HEIGHT;
float SBLUR_RATE = 0.05;

// Erosion & combination parameters:

float MOD_BASE_STR = 0.5;
float MOD_BASE_SCALE = 6.3;
float MOD_BASE_DSTR = 0.1;
float MOD_BASE_DSCALE = 4.7;

float MOD_EXTRA_STR = 0.4;
float MOD_EXTRA_SCALE = 2.6;
float MOD_EXTRA_DSTR = 0.08;
float MOD_EXTRA_DSCALE = 5.1;

int MOUNTAINS_EROSION_STEPS = 3;
float MOUNTAINS_EROSION_MODSTR = 0.7;
int TECTONICS_EROSION_STEPS = 3;
float TECTONICS_EROSION_MODSTR = 0.9;

// Hydrology parameters:

int HS_LAND = 0;
int HS_OCEAN = 1;
int HS_LAKE = 2;
int HS_RIVER = 3;

float FLOW_RIVER_THRESHOLD = 0.15;
float HYDRO_RIVER_DEPTH = 0.1;

float LAKE_RISE_INCREMENT = 0.03;

// Coloring parameters:

String COLORMODE = "grayscale";
float SEA_LEVEL = 0.1;


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


color colormap(float h, float water, int wtype) {//, String colormode) {
  if (COLORMODE == "grayscale") {
    return color(0, 0, h);
  } else if (COLORMODE == "false") {
    if (wtype == HS_LAND) {
      return color(
        0.38 - 0.28*(h - SEA_LEVEL),
        0.7,
        0.6 + 0.3*(h - SEA_LEVEL)
      );
    } else if (wtype == HS_OCEAN) {
      h = 1 - (water / SEA_LEVEL);
      return color(0.7 - 0.2*h, 0.45, 0.3 + 0.7 * h);
    } else if (wtype == HS_RIVER) {
      return color(0.6, 0.52, 0.95);
    } else if (wtype == HS_LAKE) {
      return color(0.65, 0.65, 0.7);
    } else {
      println("Unknown water type '" + str(wtype) + "'.");
      return 0;
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
    // First, pull all edge points straight outwards to meet the desired
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
    float sphase, cphase;
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
        sphase = 2.0 * PI * fx / (this.pwidth() * GRID_RESOLUTION);
        cphase = 2.0 * PI * fy / (this.pheight() * GRID_RESOLUTION);
        elevate = (
          sin(sphase + seed * 1.74)
        *
          cos(cphase + seed * 2.91)
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
            // Both relations on our a <-> c edge:
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
    Point p, cl, v;
    float d, str;
    // Only pass: compute push/pull vectors
    for (i = 0; i < this.pwidth(); ++i) {
      for (j = 0; j < this.pheight(); ++j) {
        if (
          fix_edges
        &&
          (i == 0 || i == this.pwidth()-1 || j == 0 || j == this.pheight()-1)
        ) {
          continue;
        }
        idx = this.pidx(i, j);
        p = this.points[idx];
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
        v.scale(SEAM_DT);
        p.add(v);
      }
    }
  }

  void shrink_negative() {
    // Dramatically flattens negative values to create a lopsided height
    // distribution.
    float minz = this.min_z();
    float t;
    if (minz >= 0) {
      return;
    }
    Point p;
    int i, j;
    for (i = 0; i < this.pwidth(); ++i) {
      for (j = 0; j < this.pheight(); ++j) {
        p = this.points[this.pidx(i, j)];
        if (p.z < 0) {
          t = pow(p.z / minz, 2.5);
          p.z = pow(minz, 2) * t / (NEGATIVE_SHRINK_AMOUNT * p.z);
        }
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
    float[] height,
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
    float th; // terrain height
    Point p;
    for (i = 0; i < ARRAY_WIDTH; ++i) {
      for (j = 0; j < ARRAY_HEIGHT; ++j) {
        fx = i / (float) (ARRAY_WIDTH);
        fy = j / (float) (ARRAY_HEIGHT);
        // distortion:
        dfx = TECTONIC_DISTORTION * pnoise(fx * 14.0, fy * 14.0, seed);
        dfy = TECTONIC_DISTORTION * pnoise(fx * 14.0, fy * 14.0 + 110.0, seed);
        if (fx < 0.03) {
          dfx *= sigmoid(fx / 0.03);
        }
        if (fx > 1.0 - 0.03) {
          dfx *= sigmoid((1.0 - fx) / 0.03);
        }
        if (fy < 0.03) {
          dfy *= sigmoid(fy / 0.03);
        }
        if (fy > 1.0 - 0.03) {
          dfy *= sigmoid((1.0 - fy) / 0.03);
        }
        fx = min_x + w*(fx + dfx);
        fy = min_y + h*(fy + dfy);
        p = new Point(fx, fy, 0);
        th = (this.get_height(p) - min_z) / d;
        height[i + j * ARRAY_WIDTH] = th;
        precipitation[i + j * ARRAY_WIDTH] = (
          0.25
        + 0.5 * pnoise(i * 0.05, j * 0.05)
        + 0.25 * pow(th, 0.4)
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

void min_arrays(float[] array, float[] alt) {
  int i, j, idx;
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      if (alt[idx] < array[idx]) {
        array[idx] = alt[idx];
      }
    }
  }
}

void max_arrays(float[] array, float[] alt) {
  int i, j, idx;
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      if (alt[idx] > array[idx]) {
        array[idx] = alt[idx];
      }
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

void combine_modulate_arrays(
  float[] main,
  float[] add_in,
  float[] modulation,
  float modulation_strength
) {
  int i, j, idx;
  float mod;
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      mod = modulation[idx] * modulation_strength + (1 - modulation_strength);
      main[idx] = main[idx] * mod + add_in[idx] * (1 - mod);
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

void mult_arrays(float[] main, float[] multiplicand) {
  int i, j, idx;
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      main[idx] *= multiplicand[idx];
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
      total = array[idx] * BLUR_BIAS;
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

void sharpen_array(float[] array, float[] tmp) {
  int i, j, idx;
  float total;
  float divisor;
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      total = array[idx] * SHARPEN_BIAS;
      divisor = SHARPEN_BIAS;
      if (i > 0) {
        total -= array[idx - 1];
        divisor -= 1;
      }
      if (i < ARRAY_WIDTH - 1) {
        total -= array[idx + 1];
        divisor -= 1;
      }
      if (j > 0) {
        total -= array[idx - ARRAY_WIDTH];
        divisor -= 1;
      }
      if (j < ARRAY_HEIGHT - 1) {
        total -= array[idx + ARRAY_WIDTH];
        divisor -= 1;
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

void sblur_array(
  float[] array,
  float[] tmp,
  float[] prmap,
  int iterations,
  float rate
) {
  int i, j, idx;
  int gidx, lidx, tidx;
  float rnd;
  float wsum = 0;
  float iwsum = 0;
  float midpoint;
  float maxn, minn;
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      tmp[idx] = 0;
      wsum += prmap[idx];
      iwsum += 1 - prmap[idx];
    }
  }
  for (i = 0; i < iterations; ++i) {
    rnd = random(0, iwsum);
    for (j = 0; j < ARRAY_WIDTH * ARRAY_HEIGHT; ++j) {
      rnd -= (1 - prmap[j]);
      if (rnd <= 0) {
        break;
      }
    }
    gidx = j;
    rnd = random(0, wsum);
    for (j = 0; j < ARRAY_WIDTH * ARRAY_HEIGHT; ++j) {
      rnd -= prmap[j];
      if (rnd <= 0) {
        break;
      }
    }
    lidx = j;
    if (array[gidx] > array[lidx]) {
      tidx = gidx;
      gidx = lidx;
      lidx = tidx;
    }
    midpoint = array[lidx] + (array[gidx] - array[lidx]) / 2.0;
    tmp[gidx] -= (array[gidx] - midpoint);
    tmp[lidx] += (midpoint - array[lidx]);
  }
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      tmp[idx] = array[idx] + tmp[idx] * rate;
    }
  }
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      maxn = array[idx];
      minn = array[idx];
      if (i > 0) {
        if (tmp[idx - 1] > maxn) { maxn = tmp[idx - 1]; }
        if (tmp[idx - 1] < minn) { minn = tmp[idx - 1]; }
      }
      if (i < ARRAY_WIDTH - 1) {
        if (tmp[idx + 1] > maxn) { maxn = tmp[idx + 1]; }
        if (tmp[idx + 1] < minn) { minn = tmp[idx + 1]; }
      }
      if (j > 0) {
        if (tmp[idx - ARRAY_WIDTH] > maxn) { maxn = tmp[idx - ARRAY_WIDTH]; }
        if (tmp[idx - ARRAY_WIDTH] < minn) { minn = tmp[idx - ARRAY_WIDTH]; }
      }
      if (j < ARRAY_HEIGHT - 1) {
        if (tmp[idx + ARRAY_WIDTH] > maxn) { maxn = tmp[idx + ARRAY_WIDTH]; }
        if (tmp[idx + ARRAY_WIDTH] < minn) { minn = tmp[idx + ARRAY_WIDTH]; }
      }
      if (tmp[idx] < array[idx] && tmp[idx] < minn) {
        array[idx] = minn;
      } else if (tmp[idx] > array[idx] && tmp[idx] > maxn) {
        array[idx] = maxn;
      } else {
        array[idx] = tmp[idx];
      }
    }
  }
}

void fill_troughs(float[] array, float[] fill, float[] tmp) {
  int i, j, idx;
  float minn, maxn;
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      tmp[idx] = array[idx] + fill[idx];
    }
  }
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      maxn = array[idx];
      minn = array[idx];
      if (i > 0) {
        if (array[idx - 1] > maxn) { maxn = array[idx - 1]; }
        if (array[idx - 1] < minn) { minn = array[idx - 1]; }
      }
      if (i < ARRAY_WIDTH - 1) {
        if (array[idx + 1] > maxn) { maxn = array[idx + 1]; }
        if (array[idx + 1] < minn) { minn = array[idx + 1]; }
      }
      if (j > 0) {
        if (array[idx - ARRAY_WIDTH] > maxn) { maxn = array[idx - ARRAY_WIDTH];}
        if (array[idx - ARRAY_WIDTH] < minn) { minn = array[idx - ARRAY_WIDTH];}
      }
      if (j < ARRAY_HEIGHT - 1) {
        if (array[idx + ARRAY_WIDTH] > maxn) { maxn = array[idx + ARRAY_WIDTH];}
        if (array[idx + ARRAY_WIDTH] < minn) { minn = array[idx + ARRAY_WIDTH];}
      }
      if (tmp[idx] < array[idx] && tmp[idx] < minn) {
        tmp[idx] = minn;
      } else if (tmp[idx] > array[idx] && tmp[idx] > maxn) {
        tmp[idx] = maxn;
      }
    }
  }
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      array[idx] = tmp[idx];
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

boolean run_particle(float[] array, float height, float slip, int max_steps) {
  int px = floor(random(0, ARRAY_WIDTH));
  int py = floor(random(0, ARRAY_HEIGHT));
  int i, j, idx, ndir, nx, ny;
  boolean valid;
  IntList directions = new IntList();
  directions.append(0);
  directions.append(1);
  directions.append(2);
  directions.append(3);
  for (i = 0; i < max_steps; ++i) {
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

void build_mountains(
  float[] array,
  float[] tmp,
  float[] save,
  float height,
  int max_steps,
  int slip,
  float height_falloff,
  int cycles,
  int particles,
  int particle_growth,
  int particle_growth_growth,
  int slumps,
  int final_slumps
) {
  int i, j;
  println("Starting build cycles...");
  for (i = 0; i < cycles; ++i) {
    height *= height_falloff;
    for (j = 0; j < particles; ++j) {
      run_particle(array, height, slip, max_steps);
    }
    particles += particle_growth;
    particle_growth += particle_growth_growth;
    print("\r  ...build cycle " + (i+1) + "/" + cycles + " completed...");
  }
  println("\n  ...done with build cycles.");
  copy_array(array, save);
  for (j = 0; j < slumps; ++j) {
    slump_array(array, tmp, MAX_SLOPE, SLUMP_RATE);
    average_arrays(array, save, 8.0);
  }
  for (j = 0; j < final_slumps; ++j) {
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
  float[] outputs = new float[9];
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

void slumped_flows(
  int steps,
  float[] height,
  float[] precipitation,
  float[] flow,
  float[] water,
  float[] tmp
) {
  int i;
  map_flows(steps, height, precipitation, flow, water, tmp);
  normalize_array(flow);
  for (i = 0; i < SLUMP_FLOW_STEPS; ++i) {
    slump_array(flow, tmp, FLOW_MAX_SLOPE, FLOW_SLUMP_RATE);
  }
}


Map THE_MAP;

float[] SHOW;

float[] HEIGHT;
float[] TECTONICS;
float[] FLOW;
float[] HYDROLOGY;
float[] PRECIP;
float[] MODULATE;

int[] HYDRO_STATUS;

float[] TMP;
float[] TMP2;
float[] SAVE;
float[] EX_SAVE;

void setup() {
  int i;

  randomSeed(17);
  // TODO: some way of seeding the Perlin library?

  colorMode(HSB, 1.0, 1.0, 1.0);
  size(WINDOW_WIDTH, WINDOW_HEIGHT);

  THE_MAP = new Map(GRID_WIDTH, GRID_HEIGHT);

  SHOW = new float[ARRAY_WIDTH*ARRAY_HEIGHT];
  HEIGHT = new float[ARRAY_WIDTH*ARRAY_HEIGHT];
  TECTONICS = new float[ARRAY_WIDTH*ARRAY_HEIGHT];
  FLOW = new float[ARRAY_WIDTH*ARRAY_HEIGHT];
  HYDROLOGY = new float[ARRAY_WIDTH*ARRAY_HEIGHT];
  PRECIP = new float[ARRAY_WIDTH*ARRAY_HEIGHT];
  MODULATE = new float[ARRAY_WIDTH*ARRAY_HEIGHT];
  TMP = new float[ARRAY_WIDTH*ARRAY_HEIGHT];
  TMP2 = new float[ARRAY_WIDTH*ARRAY_HEIGHT];
  SAVE = new float[ARRAY_WIDTH*ARRAY_HEIGHT];
  EX_SAVE = new float[ARRAY_WIDTH*ARRAY_HEIGHT];

  HYDRO_STATUS = new int[ARRAY_WIDTH*ARRAY_HEIGHT];

  reset_array(SHOW);
  reset_array(HEIGHT);
  reset_array(TECTONICS);
  reset_array(FLOW);
  reset_array(HYDROLOGY);
  reset_array(TMP);
  reset_array(TMP2);
  reset_array(SAVE);
  reset_array(EX_SAVE);

  //build_dual_world(173.4);
  build_tectonics(true, 173.4);
  copy_array(TECTONICS, HEIGHT);
  get_hydrology();
  COLORMODE = "false";

  noLoop();
  redraw();
}

void get_modulation(float[] array, float seed) {
  int i;
  randomSeed((int) seed);
  noise_array(
    array,
    MOD_BASE_STR,
    MOD_BASE_SCALE,
    MOD_BASE_DSTR,
    MOD_BASE_DSCALE,
    seed * 1.1
  );
  noise_array(
    array,
    MOD_EXTRA_STR,
    MOD_EXTRA_SCALE,
    MOD_EXTRA_DSTR,
    MOD_EXTRA_DSCALE,
    seed * 0.7
  );
  normalize_array(array);
}

void erode_array(float[] array, float[] modulation, float mstr) {
  copy_array(array, SAVE);
  slumped_flows(FLOW_MAP_STEPS, array, PRECIP, FLOW, TMP, TMP2);
  scale_array(FLOW, EROSION_STRENGTH);
  fill_troughs(array, FLOW, TMP);
  normalize_array(array);
  combine_modulate_arrays(array, SAVE, modulation, mstr);
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
    seed * 1.2
  );
  noise_array(
    PRECIP,
    PRECIP_EXTRA_STR,
    PRECIP_EXTRA_SCALE,
    PRECIP_EXTRA_DSTR,
    PRECIP_EXTRA_DSCALE,
    seed * 0.4
  );
  normalize_array(PRECIP);
  scale_array(PRECIP, 0.5);
  add_to_array(PRECIP, 0.5);
  reset_array(FLOW);
  reset_array(HEIGHT);
  build_mountains(
    HEIGHT,
    TMP,
    SAVE,
    PARTICLE_HEIGHT,
    MAX_WANDER_STEPS,
    SLIP,
    HEIGHT_FALLOFF,
    BUILD_CYCLES,
    STEP_PARTICLES,
    EXTRA_PARTICLES,
    EXTRA_EXTRA_PARTICLES,
    SLUMP_STEPS,
    FINAL_SLUMPS
  );

  get_modulation(MODULATE, seed * 1.3);
  for (i = 0; i < MOUNTAINS_EROSION_STEPS; ++i) {
    erode_array(HEIGHT, MODULATE, MOUNTAINS_EROSION_MODSTR);
  }
}

void build_dual_world(float seed) {
  build_world(seed);
  copy_array(HEIGHT, EX_SAVE);
  build_world(seed);
  get_modulation(MODULATE, seed);
  combine_modulate_arrays(HEIGHT, EX_SAVE, MODULATE, 1.0);
  normalize_array(HEIGHT);
}

void build_tectonics(boolean flatten_vs_chop, float seed) {
  randomSeed((long) (seed * 29));
  int i;
  boolean pull;
  Point a, b;
  THE_MAP = new Map(GRID_WIDTH, GRID_HEIGHT);
  THE_MAP.rustle(0.4, NS_TINY, random(15000));
  THE_MAP.rustle(0.6, NS_MEDIUM, random(15000));
  THE_MAP.continents(CONTINENT_STRENGTH, NS_LARGE, random(15000));
  THE_MAP.continents(CONTINENT_STRENGTH, NS_LARGE, random(15000));
  for (i = 0; i < N_TECTONIC_SEAMS; ++i) {
    a = new Point(
      random(THE_MAP.min_x(), THE_MAP.max_x()),
      random(THE_MAP.min_y(), THE_MAP.max_y()),
      0
    );
    b = new Point(
      random(THE_MAP.min_x(), THE_MAP.max_x()),
      random(THE_MAP.min_y(), THE_MAP.max_y()),
      0
    );
    pull = random(2) == 1;
    THE_MAP.seam(
      new LineSegment(a, b),
      random(MIN_SEAM_WIDTH, MAX_SEAM_WIDTH),
      pull,
      false
    );
    THE_MAP.seam(
      new LineSegment(a, b),
      random(MIN_SEAM_WIDTH, MAX_SEAM_WIDTH),
      pull,
      false
    );
  }
  //THE_MAP.rustle(0.4, NS_TINY, random(15000));
  //THE_MAP.rustle(0.4, NS_LARGE, random(15000));
  //THE_MAP.settle(DEFAULT_SETTLE_STEPS, false);
  //THE_MAP.untangle(DEFAULT_UNTANGLE_STEPS, false);
  for (i = 0; i < N_TECTONIC_RUSTLE_SETTLE_STEPS; ++i) {
    THE_MAP.rustle(0.3, NS_SMALL, random(15000));
    THE_MAP.settle(DEFAULT_SETTLE_STEPS, false);
    THE_MAP.untangle(DEFAULT_UNTANGLE_STEPS, false);
  }
  THE_MAP.stretch(
    THE_MAP.max_x() - THE_MAP.min_x(),
    THE_MAP.max_y() - THE_MAP.min_y()
  );
  THE_MAP.untangle(DEFAULT_UNTANGLE_STEPS, true);
  if (flatten_vs_chop) {
    THE_MAP.shrink_negative();
  } else {
    THE_MAP.popup();
  }
  THE_MAP.render(TECTONICS, PRECIP, seed + 53);

  //*
  normalize_array(TECTONICS);
  build_mountains(
    TECTONICS,
    TMP,
    SAVE,
    TECTONICS_PARTICLE_BASE_HEIGHT,
    TECTONICS_PARTICLE_MAX_WANDER,
    TECTONICS_PARTICLE_SLIP,
    TECTONICS_PARTICLE_HEIGHT_FALLOFF,
    TECTONICS_PARTICLE_WAVES,
    TECTONICS_PARTICLE_WAVE_SIZE,
    TECTONICS_PARTICLE_WAVE_GROWTH,
    TECTONICS_PARTICLE_WAVE_GROWTH_GROWTH,
    TECTONICS_MOUNTAINS_SLUMP_STEPS,
    TECTONICS_MOUNTAINS_FINAL_SLUMPS
  );

  // Erode things a bit:
  get_modulation(MODULATE, random(1000));
  for (i = 0; i < TECTONICS_EROSION_STEPS; ++i) {
    erode_array(TECTONICS, MODULATE, TECTONICS_EROSION_MODSTR);
  }
  get_modulation(MODULATE, random(1000));
  for (i = 0; i < TECTONICS_EROSION_STEPS - 1; ++i) {
    erode_array(TECTONICS, MODULATE, TECTONICS_EROSION_MODSTR);
  }
  // */
}

void consolidate_headwaters(float[] height, float[] water, float[] tmp) {
  int i, j, ii, jj, idx, iidx;
  float h;
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      h = height[idx];
      tmp[idx] = water[idx];
      for (ii = i - 1; ii < i + 2; ++ii) {
        for (jj = j - 1; jj < j + 2; ++jj) {
          if (
            (ii == i && jj == j)
          || ii < 0
          || ii > ARRAY_WIDTH - 1
          || jj < 0
          || jj > ARRAY_HEIGHT - 1
          ) {
            continue;
          }
          iidx = ii + ARRAY_WIDTH * jj;
          if (height[iidx] > h && water[iidx] > 0) {
            tmp[idx] = 0;
          }
        }
      }
    }
  }
  copy_array(tmp, water);
}

void discover_depths(float[] height, float[] water, float[] result) {
  int i, j, ii, jj, idx, iidx;
  float h;
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      h = height[idx];
      result[idx] = water[idx];
      for (ii = i - 1; ii < i + 2; ++ii) {
        for (jj = j - 1; jj < j + 2; ++jj) {
          if (
            (ii == i && jj == j)
          || ii < 0
          || ii > ARRAY_WIDTH - 1
          || jj < 0
          || jj > ARRAY_HEIGHT - 1
          ) {
            continue;
          }
          iidx = ii + ARRAY_WIDTH * jj;
          if (height[iidx] < h && water[iidx] > 0) {
            result[idx] = 0;
          }
        }
      }
    }
  }
}

boolean flow_rivers_once(
  float sea_level,
  float[] height,
  float[] rivers,
  float[] tmp
) {
  int i, j, ii, jj, idx, iidx, lnidx;
  float h, ln;
  boolean result = false;
  reset_array(tmp);
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      h = height[idx];
      ln = h;
      lnidx = idx;
      tmp[idx] += rivers[idx];
      if (rivers[idx] <= 0 || h < sea_level) {
        continue;
      }
      for (ii = i - 1; ii < i + 2; ++ii) {
        for (jj = j - 1; jj < j + 2; ++jj) {
          if (
            (ii == i && jj == j)
          || ii < 0
          || ii > ARRAY_WIDTH - 1
          || jj < 0
          || jj > ARRAY_HEIGHT - 1
          ) {
            continue;
          }
          iidx = ii + ARRAY_WIDTH * jj;
          if (height[iidx] < ln) {
            ln = height[iidx];
            lnidx = iidx;
          }
        }
      }
      if (ln < h) { // if we have a lower neighbor
        if (rivers[lnidx] < rivers[idx]) {
          tmp[lnidx] += rivers[idx];
          result = true;
        }
      }
    }
  }
  copy_array(tmp, rivers);
  return result;
}

void compute_hydrology(
  float sea_level,
  float[] height,
  float[] flow,
  float[] hydro,
  int[] hydro_status,
  float[] tmp
) {
  int i, j, idx;
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      hydro[idx] = 0;
      hydro_status[idx] = HS_LAND;
    }
  }
  // Start with our flow info:
  copy_array(flow, hydro);
  sharpen_array(hydro, tmp);
  // After sharpening, threshold it to find rivers:
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      if (height[idx] < sea_level) {
        hydro[idx] = 0;
        continue;
      }
      if (hydro[idx] > FLOW_RIVER_THRESHOLD) {
        hydro[idx] = HYDRO_RIVER_DEPTH;
      } else {
        hydro[idx] = 0;
      }
    }
  }
  // Find the headwaters of each strong flow region:
  consolidate_headwaters(height, hydro, tmp);
  // Flow rivers:
  println("Flowing rivers...");
  while(flow_rivers_once(sea_level, height, hydro, tmp)) {}
  println("  ...done.");
  // hydro now holds river information
  // Finally, add oceans:
  for (i = 0; i < ARRAY_WIDTH; ++i) {
    for (j = 0; j < ARRAY_HEIGHT; ++j) {
      idx = i + ARRAY_WIDTH * j;
      if (hydro[idx] > 0) {
        hydro_status[idx] = HS_RIVER;
      }
      if (height[idx] < sea_level) {
        hydro_status[idx] = HS_OCEAN;
        hydro[idx] = sea_level - height[idx];
      }
    }
  }
}

void get_hydrology() {
  slumped_flows(FLOW_MAP_STEPS, HEIGHT, PRECIP, FLOW, TMP, TMP2);
  compute_hydrology(
    SEA_LEVEL,
    HEIGHT,
    FLOW,
    HYDROLOGY,
    HYDRO_STATUS,
    TMP
  );
}

void draw() {
  int i, j, idx;
  background(0.6, 0.8, 0.25);
  float gw, gh;
  float scale_factor;

  int cw = WINDOW_WIDTH / ARRAY_WIDTH;
  int ch = WINDOW_HEIGHT / ARRAY_HEIGHT;

  if (DISPLAY == "height") {
    copy_array(HEIGHT, SHOW);
  } else if (DISPLAY == "tectonics") {
    copy_array(TECTONICS, SHOW);
  } else if (DISPLAY == "flow") {
    copy_array(FLOW, SHOW);
  } else if (DISPLAY == "precipitation") {
    copy_array(PRECIP, SHOW);
  } else if (DISPLAY == "hydrology") {
    copy_array(HYDROLOGY, SHOW);
  } else if (DISPLAY == "modulate") {
    copy_array(MODULATE, SHOW);
  }

  if (DISPLAY == "grid") {
    stroke(0.0, 0.0, 1.0);
    noFill();
    gw = THE_MAP.max_x() - THE_MAP.min_x();
    gh = THE_MAP.max_y() - THE_MAP.min_y();
    scale_factor = 0.8 * WINDOW_WIDTH / gw;
    pushMatrix();
    scale(scale_factor);
    translate(
      (WINDOW_WIDTH / 2.0) / scale_factor,
      (WINDOW_HEIGHT / 2.0)  / scale_factor
    );
    translate(
      -(THE_MAP.min_x() + gw / 2.0),
      -(THE_MAP.min_y() + gh / 2.0)
    );
    THE_MAP.render_wireframe();
    popMatrix();
  } else {
    noStroke();
    for (i = 0; i < ARRAY_WIDTH; ++i) {
      for (j = 0; j < ARRAY_HEIGHT; ++j) {
        idx = i + j * ARRAY_WIDTH;
        fill(colormap(SHOW[idx], HYDROLOGY[idx], HYDRO_STATUS[idx]));
        rect(i*cw, j*ch, cw, ch);
      }
    }
  }
}

void keyPressed() {
  int i;
  if (key == 'q') {
    exit();
  } else if (key == 'r') {
    // Reset the HEIGHT array
    reset_array(HEIGHT);
  } else if (key == 'p') {
    // Run a single particle at PARTICLE_HEIGHT
    run_particle(HEIGHT, PARTICLE_HEIGHT, SLIP, MAX_WANDER_STEPS);
  } else if (key == 'P') {
    // Run EXTRA_PARTICLES particles at PARTICLE_HEIGHT
    for (i = 0; i < EXTRA_PARTICLES; ++i) {
      run_particle(HEIGHT, PARTICLE_HEIGHT, SLIP, MAX_WANDER_STEPS);
    }
  } else if (key == 'b') {
    // Blur the HEIGHT array
    blur_array(HEIGHT, TMP);
  } else if (key == 's') {
    // Slump the HEIGHT array
    slump_array(HEIGHT, TMP, MAX_SLOPE, SLUMP_RATE);
  } else if (key == 'S') {
    // Stochastic blur on the HEIGHT array
    sblur_array(HEIGHT, TMP, FLOW, SBLUR_STEPS, SBLUR_RATE);
  } else if (key == 'f') {
    // Compute raw flow values
    slumped_flows(FLOW_MAP_STEPS, HEIGHT, PRECIP, FLOW, TMP, TMP2);
  } else if (key == 'F') {
    // Compute flow values and use them for erosion
    get_modulation(MODULATE, random(1000));
    erode_array(HEIGHT, MODULATE, MOUNTAINS_EROSION_MODSTR);
    get_hydrology();
  } else if (key == 'm') {
    // Build a new world map
    build_world(random(1000));
  } else if (key == 'M') {
    // Build a complex world map by merging two basic maps
    build_dual_world(random(1000));
  } else if (key == 't') {
    // Build new tectonics
    println("Building tectonics...");
    build_tectonics(true, random(1000));
    println("  ...done.");
    copy_array(TECTONICS, HEIGHT);
    get_hydrology();
  } else if (key == 'T') {
    reset_array(HEIGHT);
    reset_array(TMP);
    noise_array(
      HEIGHT,
      MOD_BASE_STR,
      MOD_BASE_SCALE,
      MOD_BASE_DSTR,
      MOD_BASE_DSCALE,
      random(1000)
    );
    noise_array(
      TMP,
      MOD_BASE_STR,
      MOD_BASE_SCALE,
      MOD_BASE_DSTR,
      MOD_BASE_DSCALE,
      random(1000)
    );
    normalize_array(HEIGHT);
    normalize_array(TMP);
    min_arrays(HEIGHT, TMP);
    //max_arrays(HEIGHT, TMP);
    //mult_arrays(HEIGHT, TMP);
    //normalize_array(HEIGHT);
  } else if (key == 'h') {
    // Compute hydrology
    get_hydrology();
  } else if (key == 'j') {
    // Lower the sea level
    SEA_LEVEL -= 0.05;
    get_hydrology();
  } else if (key == 'k') {
    // Raise the sea level
    SEA_LEVEL += 0.05;
    get_hydrology();
  } else if (key == 'c') {
    // Switch color modes
    if (COLORMODE == "grayscale") {
      COLORMODE = "false";
    } else {
      COLORMODE = "grayscale";
    }
  } else if (key == '1') {
    // View the HEIGHT array
    DISPLAY = "height";
  } else if (key == '2') {
    // View the TECTONICS grid
    DISPLAY = "grid";
  } else if (key == '3') {
    // View the TECTONICS array
    DISPLAY = "tectonics";
  } else if (key == '4') {
    // View the FLOW array
    DISPLAY = "flow";
  } else if (key == '5') {
    // View the HYDROLOGY array
    DISPLAY = "hydrology";
  } else if (key == '6') {
    // View the PRECIP array
    DISPLAY = "precipitation";
  } else if (key == '7') {
    // View the MODULATE array
    DISPLAY = "modulate";
  }
  redraw();
}
