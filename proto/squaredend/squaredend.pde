// vim: syn=java

import perlin.*;

Perlin pnl = new Perlin(this);

Grid THE_GRID;

String MODE = "grid";
String HEIGHTMODE = "simplex";
String COLOR_MODE = "grayscale";

int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 800;

//int MAP_WIDTH = 60;
//int MAP_HEIGHT = 60;

int MAP_WIDTH = 100;
int MAP_HEIGHT = 100;

float SEA_LEVEL = 0.2;

float BUMPINESS = 10;

float DEFAULT_SCALE = 0.15 / 1.2;
float SCALE = DEFAULT_SCALE;
float GRID_SCALE = 100;
float NS_LARGE = 0.1;
float NS_MEDIUM = 0.8;
float NS_SMALL = 1.6;
float NS_TINY = 3.2;

float DSTR = 1.3;

int PRUNE_PERCENT = 75;


int choice(int i, int j, int seed, int count) {
  if (count == 0) {
    return -1;
  }
  return abs(((5581 + (7873 + 1229 ^ i) ^ j) ^ seed) % count);
}

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


color colormap(float h, String colormode) {
  if (colormode == "grayscale") {
    return color(0, 0, h);
  } else if (colormode == "erosion") {
    if (h < 0) {
      return color(0.0, 1.0, -h);
    } else if (h > 0) {
      return color(0.7, 1.0, h);
    } else {
      return color(0, 0, 1);
    }
  } else if (colormode == "false") {
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
    println("Unknown color mode '" + colormode + "'.");
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

  float dist2d(Point other) {
    return sqrt(pow(other.x - this.x, 2) + pow(other.y - this.y, 2));
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

class Edge {
  Point from, to;
  int state;
  Edge(Point from, Point to) {
    this.from = from;
    this.to = to;
    this.state = 0;
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

  Point closest_point_2d(Point p) {
    Point proj;
    Point v, to_p;
    float m;
    v = this.from.vector_to(this.to);
    v.z = 0;
    to_p = this.from.vector_to(p);
    to_p.z = 0;
    proj = to_p.project_onto(v);
    m = proj.mag_in_terms_of(v);
    if (m < 0) {
      return new Point(this.from);
    } else if (m > 1.0) {
      return new Point(this.to);
    } else {
      proj.add(this.from);
      proj.z = 0;
      return proj;
    }
  }

  void draw(float scale) {
    if (this.state == 1) {
      strokeWeight(2.0);
      if (this.from.z > this.to.z) {
        ellipse(
          (this.from.x + 0.9*(this.to.x - this.from.x)) * scale,
          (this.from.y + 0.9*(this.to.y - this.from.y)) * scale,
          6, 6
        );
      } else {
        ellipse(
          (this.from.x + 0.1*(this.to.x - this.from.x)) * scale,
          (this.from.y + 0.1*(this.to.y - this.from.y)) * scale,
          6, 6
        );
      }
      line(
        this.from.x * scale, this.from.y * scale,
        this.to.x * scale, this.to.y * scale
      );
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

class Grid {
  Edge edges[];
  Point points[];
  Point forces[];
  int avgcounts[];
  Point center;
  int width;
  int height;
  Grid(int width, int height) {
    int i, j, idx, hidx, vidx, didx;
    float x, y, z;
    Edge e;
    Point a, b, c, d;
    this.width = width;
    this.height = height;
    this.points = new Point[width*height];
    this.edges = new Edge[this.ewidth()*this.eheight()];
    this.center = new Point(0, 0, 0);
    // Set up points:
    for (i = 0; i < this.width; ++i) {
      for (j = 0; j < this.height; ++j) {
        idx = this.pidx(i, j);
        x = i;
        y = j;
        z = 0;
        // scatter grid points randomly within grid cells:
        x = x - 0.4 + random(0.8);
        y = y - 0.4 + random(0.8);
        this.points[idx] = new Point(x, y, z);
      }
    }
    // Set up edges:
    for (i = 0; i < this.width; ++i) {
      for (j = 0; j < this.height; ++j) {
        hidx = this.eidx_h(i, j);
        vidx = this.eidx_v(i, j);
        a = this.points[this.pidx(i, j)];
        if (i < this.width - 1) {
          b = this.points[this.pidx(i+1, j)];
          this.edges[hidx] = new Edge(a, b);
        } else {
          this.edges[hidx] = null;
        }
        if (j < this.height - 1) {
          c = this.points[this.pidx(i, j+1)];
          this.edges[vidx] = new Edge(a, c);
        } else {
          this.edges[vidx] = null;
        }
      }
    }
    this.update_center();
  }

  int ewidth() { return 2*this.width; }
  int eheight() { return this.height; }

  int pidx(int i, int j) { return j*this.width+i; }

  int eidx_v(int i, int j) { return j*this.ewidth()+2*i; }
  int eidx_h(int i, int j) { return j*this.ewidth()+2*i + 1; }

  void edges_from(int i, int j, Edge[] result) {
    // north / east / south / west
    result[0] = this.edges[this.eidx_v(i, j)];
    result[1] = this.edges[this.eidx_h(i, j)];
    if (j > 0) {
      result[2] = this.edges[this.eidx_v(i, j-1)];
    } else {
      result[2] = null;
    }
    if (i > 0) {
      result[3] = this.edges[this.eidx_h(i-1, j)];
    } else {
      result[3] = null;
    }
  }

  void update_center() {
    int i, j, idx;
    this.center.x = 0;
    this.center.y = 0;
    this.center.z = 0;
    for (i = 0; i < this.width; ++i) {
      for (j = 0; j < this.height; ++j) {
        idx = this.pidx(i, j);
        this.center.add(this.points[idx]);
      }
    }
    this.center.scale(1.0 / (float) (this.width*this.height));
  }

  float min_x() {
    int i, j, idx;
    float result = this.points[0].x;
    for (i = 0; i < this.width; ++i) {
      for (j = 0; j < this.height; ++j) {
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
    for (i = 0; i < this.width; ++i) {
      for (j = 0; j < this.height; ++j) {
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
    for (i = 0; i < this.width; ++i) {
      for (j = 0; j < this.height; ++j) {
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
    for (i = 0; i < this.width; ++i) {
      for (j = 0; j < this.height; ++j) {
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
    for (i = 0; i < this.width; ++i) {
      for (j = 0; j < this.height; ++j) {
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
    for (i = 0; i < this.width; ++i) {
      for (j = 0; j < this.height; ++j) {
        idx = this.pidx(i, j);
        if (this.points[idx].z > result) {
          result = this.points[idx].z;
        }
      }
    }
    return result;
  }

  void dendrize(int seed) {
    int i, j;
    int count, r;
    Edge[] edges = new Edge[4];
    for (i = 0; i < this.width; ++i) {
      for (j = 0; j < this.height; ++j) {
        // Turn off all but one downhill edge from this point:
        this.edges_from(i, j, edges);
        count = 0;
        if (edges[0] != null && edges[0].from.z > edges[0].to.z) {
          edges[0].state = 0;
          count += 1;
        }
        if (edges[1] != null && edges[1].from.z > edges[1].to.z) {
          edges[1].state = 0;
          count += 1;
        }
        if (edges[2] != null && edges[2].from.z < edges[2].to.z) {
          edges[2].state = 0;
          count += 1;
        }
        if (edges[3] != null && edges[3].from.z < edges[3].to.z) {
          edges[3].state = 0;
          count += 1;
        }
        if (count == 0) {
          continue;
        }
        r = choice(i, j, seed, count);
        if (edges[0] != null && edges[0].from.z > edges[0].to.z) {
          if (r == 0) {
            edges[0].state = 1;
          }
          r -= 1;
        }
        if (edges[1] != null && edges[1].from.z > edges[1].to.z) {
          if (r == 0) {
            edges[1].state = 1;
          }
          r -= 1;
        }
        if (edges[2] != null && edges[2].from.z < edges[2].to.z) {
          if (r == 0) {
            edges[2].state = 1;
          }
          r -= 1;
        }
        if (edges[3] != null && edges[3].from.z < edges[3].to.z) {
          if (r == 0) {
            edges[3].state = 1;
          }
          r -= 1;
        }
      }
    }
  }

  void set_heights(String mode) {
    Point p;
    int i, j;
    for (i = 0; i < this.width; ++i) {
      for (j = 0; j < this.height; ++j) {
        p = this.points[this.pidx(i, j)];
        if (mode == "uniform-x") {
          p.z = p.x;
        } else if (mode == "uniform-y") {
          p.z = p.y;
        } else if (mode == "simplex") {
          p.z = BUMPINESS * pnoise(
            NS_LARGE * p.x + DSTR * pnoise(NS_LARGE * p.x+2000, NS_LARGE * p.y),
            NS_LARGE * p.y + DSTR * pnoise(NS_LARGE * p.x, NS_LARGE * p.y+2000)
          );
        }
      }
    }
  }

  void render(float scale) {
    int i, j;
    for (i = 0; i < this.width; ++i) {
      for (j = 0; j < this.height; ++j) {
        if (i < this.width - 1) {
          this.edges[this.eidx_h(i, j)].draw(scale);
        }
        if (j < this.height - 1) {
          this.edges[this.eidx_v(i, j)].draw(scale);
        }
      }
    }
  }

  void prune(int seed) {
    int i, j;
    int count, r;
    Edge[] edges = new Edge[6];
    Edge last;
    for (i = 0; i < this.width; ++i) {
      for (j = 0; j < this.height; ++j) {
        this.edges_from(i, j, edges);
        count = 0;
        last = null;
        if (edges[0] != null && edges[0].state != 0) {
          count += 1;
          last = edges[0];
        }
        if (edges[1] != null && edges[1].state != 0) {
          count += 1;
          last = edges[1];
        }
        if (edges[2] != null && edges[2].state != 0) {
          count += 1;
          last = edges[2];
        }
        if (edges[3] != null && edges[3].state != 0) {
          count += 1;
          last = edges[3];
        }
        if (count == 1 && choice(i*j, j+i, seed, 100) < PRUNE_PERCENT) {
          last.state = -1;
        }
      }
    }
  }

  float flat_distance_to(Point p) {
    Edge e;
    int i, j, a_i, a_j;
    float est, dist = 6;
    a_i = floor(this.width  * (p.x-this.min_x()) / (this.max_x()-this.min_x()));
    a_j = floor(this.height * (p.y-this.min_y()) / (this.max_y()-this.min_y()));
    for (i = a_i - 2; i < a_i + 2; ++i) {
      for (j = a_j - 2; j < a_j + 2; ++j) {
        if (i < 0 || j < 0 || i > this.width - 1 || j > this.height - 1) {
          continue;
        }
        e = this.edges[this.eidx_v(i, j)];
        if (e != null && e.state == 1) {
          est = e.closest_point_2d(p).dist2d(p);
          if (est < dist) { dist = est; }
        }
        e = this.edges[this.eidx_h(i, j)];
        if (e != null && e.state == 1) {
          est = e.closest_point_2d(p).dist2d(p);
          if (est < dist) { dist = est; }
        }
      }
    }
    return dist;
  }
}

void setup() {
  randomSeed(17);
  noiseSeed(17);
  // TODO: some way of seeding the Perlin library?

  colorMode(HSB, 1.0, 1.0, 1.0);
  size(WINDOW_WIDTH, WINDOW_HEIGHT);

  THE_GRID = new Grid(MAP_WIDTH, MAP_HEIGHT);
  THE_GRID.set_heights(HEIGHTMODE);
  THE_GRID.dendrize((int) random(1000));
  THE_GRID.prune((int) random(1000));

  noLoop();
  redraw();
}


void draw() {
  int i, j;
  float x, y, gw, gh, d;
  background(0.6, 0.8, 0.25);
  //background(0, 0, 1);
  if (MODE == "grid") {
    // Center the map:
    smooth();
    stroke(0.0, 0.0, 1.0);
    noFill();
    pushMatrix();
    THE_GRID.update_center();
    translate(
      -THE_GRID.center.x*GRID_SCALE*SCALE,
      -THE_GRID.center.y*GRID_SCALE*SCALE
    );
    translate(WINDOW_WIDTH/2.0, WINDOW_HEIGHT/2.0);
    scale(SCALE);
    // And draw it:
    THE_GRID.render(GRID_SCALE);
    popMatrix();
  } else if (MODE == "dist") {
    gw = THE_GRID.max_x() - THE_GRID.min_x();
    gh = THE_GRID.max_y() - THE_GRID.min_y();
    noStroke();
    print("Rendering...");
    int c = 0;
    for (i = 0; i < WINDOW_WIDTH; i += 4) {
      for (j = 0; j < WINDOW_HEIGHT; j += 4) {
        x = THE_GRID.min_x() + gw * (float) ((i + 2) / (float) WINDOW_WIDTH);
        y = THE_GRID.min_y() + gh * (float) ((j + 2) / (float) WINDOW_HEIGHT);
        x += 0.4 * pnoise(
          NS_MEDIUM * x,
          NS_MEDIUM * y + 181,
          7.3
        );
        y += 0.4 * pnoise(
          NS_MEDIUM * x + 5645,
          NS_MEDIUM * y,
          35.8
        );
        d = THE_GRID.flat_distance_to(new Point(x, y, 0));
        fill(colormap((d), COLOR_MODE));
        rect(i, j, 4, 4);
        c += 1;
        print("\r  ...", c, "/", WINDOW_WIDTH * WINDOW_HEIGHT/16, "...");
      }
    }
    println("\n  ...done.");
  }
}

void keyPressed() {
  Point a, b;
  int i, j, idx;
  boolean push = false;
  if (key == 'q') {
    exit();
  } else if (key == 'k') {
    SCALE *= 1.2;
  } else if (key == 'j') {
    SCALE /= 1.2;
  } else if (key == 'K') {
    SEA_LEVEL += 0.05;
  } else if (key == 'J') {
    SEA_LEVEL -= 0.05;
  } else if (key == 'h') {
    // re-set heights and re-dendrize
    if (HEIGHTMODE == "uniform-x") {
      HEIGHTMODE = "uniform-y";
    } else if (HEIGHTMODE == "uniform-y") {
      HEIGHTMODE = "simplex";
    } else if (HEIGHTMODE == "simplex") {
      HEIGHTMODE = "uniform-x";
    }
    println(HEIGHTMODE);
    THE_GRID.set_heights(HEIGHTMODE);
    THE_GRID.dendrize((int) random(1000));
  } else if (key == 'd') {
    // re-dendrize with a random seed:
    THE_GRID.dendrize((int) random(1000));
  } else if (key == 'p') {
    THE_GRID.prune((int) random(1000));
  } else if (key == 'r') {
    // Reset stuff to 0:
    THE_GRID = new Grid(MAP_WIDTH, MAP_HEIGHT);
    SCALE = DEFAULT_SCALE;
  } else if (key == 'c') {
    if (COLOR_MODE == "grayscale") {
      COLOR_MODE = "false";
    } else {
      COLOR_MODE = "grayscale";
    }
  } else if (key == '1') {
    MODE = "grid";
  } else if (key == '2') {
    MODE = "dist";
  }
  redraw();
}
