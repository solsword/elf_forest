// vim: syn=java

import perlin.*;

Perlin pnl = new Perlin(this);

Map THE_MAP;

PImage THE_IMAGE;

String MODE = "grid";

int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;

int MAP_WIDTH = 120;
int MAP_HEIGHT = 50;

//int MAP_WIDTH = 80;
//int MAP_HEIGHT = 30;

//int MAP_WIDTH = 8;
//int MAP_HEIGHT = 6;

//int MAP_WIDTH = 4;
//int MAP_HEIGHT = 4;

int IMAGE_WIDTH = 200;
int IMAGE_HEIGHT = 150;

float SEA_LEVEL = 0.0;

float GRID_RESOLUTION = 100.0;

float SG_HEIGHT = sin(PI/3.0);
float SG_OFFSET = cos(PI/3.0);

float BUMPINESS = 0.05;

float DEFAULT_SCALE = 0.15;
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


color colormap(float h) {
/*
  float[] result = new float[3];
  if (h < SEA_LEVEL) {
    result[0] = 0.7;
    result[1] = 0.45;
    result[2] = h;
  } else {
    result[0] = 0.02 + 0.24*h;
    result[1] = 0.45;
    result[2] = 0.5 + 0.4*h;
  }
  return color(result[0], result[1], result[2]);
  */
  return color(0, 0, h);
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

float init_z(float x, float y) {
  return BUMPINESS * GRID_RESOLUTION * pnoise(
    NS_LARGE * x + DSTR * pnoise(NS_LARGE * x + 20000, NS_LARGE * y),
    NS_LARGE * y + DSTR * pnoise(NS_LARGE * x, NS_LARGE * y + 20000)
  );
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
        z = init_z(x, y);
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

  void render(PImage fb) {
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
    Point p;
    for (i = 0; i < fb.width; ++i) {
      for (j = 0; j < fb.height; ++j) {
        fx = i / (float) (fb.width);
        fy = j / (float) (fb.height);
        fx = min_x + w*fx;
        fy = min_y + h*fy;
        p = new Point(fx, fy, 0);
        fb.set(i, j, colormap((this.get_height(p) - min_z) / d));
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

void setup() {
  randomSeed(17);
  noiseSeed(17);
  // TODO: some way of seeding the Perlin library?
  colorMode(HSB, 1.0, 1.0, 1.0);
  size(WINDOW_WIDTH, WINDOW_HEIGHT);
  THE_MAP = new Map(MAP_WIDTH, MAP_HEIGHT);
  THE_IMAGE = new PImage(IMAGE_WIDTH, IMAGE_HEIGHT);
  noLoop();
  redraw();
}


void draw() {
  background(0.6, 0.8, 0.25);
  stroke(0.0, 0.0, 1.0);
  noFill();
  if (MODE == "map") {
    // Draw the rendered map:
    pushMatrix();
    scale(
      WINDOW_WIDTH / (float) THE_IMAGE.width,
      WINDOW_HEIGHT / (float) THE_IMAGE.height
    );
    THE_MAP.render(THE_IMAGE);
    image(THE_IMAGE, 0, 0);
    popMatrix();
  } else if (MODE == "grid") {
    // Center the map:
    pushMatrix();
    THE_MAP.update_center();
    translate(-THE_MAP.center.x*SCALE, -THE_MAP.center.y*SCALE);
    translate(WINDOW_WIDTH/2.0, WINDOW_HEIGHT/2.0);
    scale(SCALE);
    // And draw it:
    THE_MAP.render_wireframe();
    popMatrix();
  }
}

void keyPressed() {
  Point a, b;
  int i;
  boolean push = false;
  if (key == 'q') {
    exit();
  } else if (key == 'k') {
    SCALE *= 1.2;
  } else if (key == 'j') {
    SCALE /= 1.2;
  } else if (key == 'g') {
    THE_MAP = new Map(MAP_WIDTH, MAP_HEIGHT);
    THE_MAP.rustle(0.4, NS_TINY, (int) random(15000));
    THE_MAP.rustle(0.4, NS_MEDIUM, (int) random(15000));
    for (i = 0; i < 25; ++i) {
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
      push = (int) random(2) == 1;
      THE_MAP.seam(
        new LineSegment(a, b),
        random(MIN_SEAM_WIDTH, MAX_SEAM_WIDTH),
        push,
        false
      );
      THE_MAP.seam(
        new LineSegment(a, b),
        random(MIN_SEAM_WIDTH, MAX_SEAM_WIDTH),
        push,
        false
      );
    }
    THE_MAP.rustle(0.4, NS_TINY, random(15000));
    THE_MAP.rustle(0.4, NS_LARGE, (int) random(15000));
    THE_MAP.settle(DEFAULT_SETTLE_STEPS, false);
    THE_MAP.untangle(DEFAULT_UNTANGLE_STEPS, false);
    for (i = 0; i < 4; ++i) {
      THE_MAP.rustle(0.6, NS_TINY, random(15000));
      THE_MAP.settle(DEFAULT_SETTLE_STEPS, false);
      THE_MAP.untangle(DEFAULT_UNTANGLE_STEPS, false);
    }
    THE_MAP.stretch(
      THE_MAP.max_x() - THE_MAP.min_x(),
      THE_MAP.max_y() - THE_MAP.min_y()
    );
    THE_MAP.untangle(DEFAULT_UNTANGLE_STEPS, false);
    THE_MAP.rustle(0.6, NS_TINY, random(15000));
    THE_MAP.rustle(0.4, NS_TINY, random(15000));
    THE_MAP.untangle(DEFAULT_UNTANGLE_STEPS, false);
    THE_MAP.settle(DEFAULT_SETTLE_STEPS/2, false);
    THE_MAP.stretch(
      THE_MAP.max_x() - THE_MAP.min_x(),
      THE_MAP.max_y() - THE_MAP.min_y()
    );
    THE_MAP.untangle(DEFAULT_UNTANGLE_STEPS, false);
    THE_MAP.untangle(DEFAULT_UNTANGLE_STEPS, true);
    THE_MAP.rustle(0.8, NS_MEDIUM, (int) random(15000));
    THE_MAP.rustle(1.2, NS_LARGE, (int) random(15000));
  } else if (key == 'w') {
    THE_MAP.rustle(DEFAULT_RUSTLE_STR, NS_LARGE, (int) random(15000));
  } else if (key == 'u') {
    THE_MAP.untangle(DEFAULT_UNTANGLE_STEPS, false);
  } else if (key == 'U') {
    THE_MAP.untangle(DEFAULT_UNTANGLE_STEPS, true);
  } else if (key == 'R') {
    THE_MAP = new Map(MAP_WIDTH, MAP_HEIGHT);
    SCALE = DEFAULT_SCALE;
  } else if (key == 'r') {
    THE_MAP.rustle(0.8, NS_TINY, random(15000));
  } else if (key == 'S') {
    THE_MAP.stretch(
      THE_MAP.max_x() - THE_MAP.min_x(),
      THE_MAP.max_y() - THE_MAP.min_y()
    );
  } else if (key == 's') {
    THE_MAP.settle(DEFAULT_SETTLE_STEPS, false);
  } else if (key == 'p') {
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
    THE_MAP.seam(
      new LineSegment(a, b),
      random(MIN_SEAM_WIDTH, MAX_SEAM_WIDTH),
      false,
      false
    );
  } else if (key == 'P') {
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
    THE_MAP.seam(
      new LineSegment(a, b),
      random(MIN_SEAM_WIDTH, MAX_SEAM_WIDTH),
      true,
      false
    );
  } else if (key == 'm') {
    if (MODE == "map") {
      MODE = "grid";
    } else {
      MODE = "map";
    }
  }
  redraw();
}
