// vim: syn=java

import perlin.*;

Perlin pnl = new Perlin(this);

float[] MATRIX;
float[] TMP;
float[] SAVE;

int WINDOW_WIDTH = 900;
int WINDOW_HEIGHT = 600;

int MAP_WIDTH = 180;
int MAP_HEIGHT = 120;

//int MAP_WIDTH = 80;
//int MAP_HEIGHT = 30;

//int MAP_WIDTH = 8;
//int MAP_HEIGHT = 6;

//int MAP_WIDTH = 4;
//int MAP_HEIGHT = 4;

int IMAGE_WIDTH = 120;
int IMAGE_HEIGHT = 80;

int BUILD_CYCLES = 80;
int STEP_PARTICLES = 400;
int EXTRA_PARTICLES = 20;
int SLIP = 3; // TODO: Why does this fail at 4?
float HEIGHT = 1.0;
float BLUR_BIAS = 8;
float MAX_SLOPE = 0.01;
float EROSION_RATE = 0.2;
float SLUMP_STEPS = 50;
float FINAL_SLUMPS = 5;

String COLORMODE = "grayscale";
float SEA_LEVEL = 0.4;

int MAX_WANDER_STEPS = 30000;

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

void reset_array(float[] array) {
  int i, j, idx;
  for (i = 0; i < MAP_WIDTH; ++i) {
    for (j = 0; j < MAP_HEIGHT; ++j) {
      idx = i + MAP_WIDTH * j;
      array[idx] = 0;
    }
  }
}

void copy_array(float[] from, float[] to) {
  int i, j, idx;
  for (i = 0; i < MAP_WIDTH; ++i) {
    for (j = 0; j < MAP_HEIGHT; ++j) {
      idx = i + MAP_WIDTH * j;
      to[idx] = from[idx];
    }
  }
}

void average_arrays(float[] main, float[] add_in, float main_weight) {
  int i, j, idx;
  for (i = 0; i < MAP_WIDTH; ++i) {
    for (j = 0; j < MAP_HEIGHT; ++j) {
      idx = i + MAP_WIDTH * j;
      main[idx] = (main[idx]*main_weight + add_in[idx]) / (1.0 + main_weight);
    }
  }
}

void blur_array(float[] array, float[] tmp) {
  int i, j, idx;
  float total;
  float divisor;
  for (i = 0; i < MAP_WIDTH; ++i) {
    for (j = 0; j < MAP_HEIGHT; ++j) {
      idx = i + MAP_WIDTH * j;
      total  = array[idx] * BLUR_BIAS;
      divisor = BLUR_BIAS;
      if (i > 0) {
        total += array[idx - 1];
        divisor += 1;
      }
      if (i < MAP_WIDTH - 1) {
        total += array[idx + 1];
        divisor += 1;
      }
      if (j > 0) {
        total += array[idx - MAP_WIDTH];
        divisor += 1;
      }
      if (j < MAP_HEIGHT - 1) {
        total += array[idx + MAP_WIDTH];
        divisor += 1;
      }
      tmp[idx] = total / divisor;
    }
  }
  for (i = 0; i < MAP_WIDTH; ++i) {
    for (j = 0; j < MAP_HEIGHT; ++j) {
      idx = i + MAP_WIDTH * j;
      array[idx] = tmp[idx];
    }
  }
}

void slump_array(float[] array, float[] tmp) {
  int i, j, idx, lnidx;
  float ln, midpoint;
  for (i = 0; i < MAP_WIDTH; ++i) {
    for (j = 0; j < MAP_HEIGHT; ++j) {
      idx = i + MAP_WIDTH * j;
      tmp[idx] = 0;
    }
  }
  for (i = 0; i < MAP_WIDTH; ++i) {
    for (j = 0; j < MAP_HEIGHT; ++j) {
      idx = i + MAP_WIDTH * j;
      ln = array[idx];
      lnidx = idx;
      if (i > 0 && array[idx - 1] < ln) {
        ln = array[idx - 1];
        lnidx = idx - 1;
      }
      if (i < MAP_WIDTH - 1 && array[idx + 1] < ln) {
        ln = array[idx + 1];
        lnidx = idx + 1;
      }
      if (j > 0 && array[idx - MAP_WIDTH] < ln) {
        ln = array[idx - MAP_WIDTH];
        lnidx = idx - MAP_WIDTH;
      }
      if (j < MAP_HEIGHT - 1 && array[idx + MAP_WIDTH] < ln) {
        ln = array[idx + MAP_WIDTH];
        lnidx = idx + MAP_WIDTH;
      }
      if (lnidx != idx && array[idx] - ln > MAX_SLOPE) {
        midpoint = ln + (array[idx] - ln) / 2.0;
        tmp[idx] -= array[idx] - (midpoint + MAX_SLOPE/2.0);
        tmp[lnidx] += (midpoint - MAX_SLOPE/2.0) - ln;
      }
    }
  }
  for (i = 0; i < MAP_WIDTH; ++i) {
    for (j = 0; j < MAP_HEIGHT; ++j) {
      idx = i + MAP_WIDTH * j;
      array[idx] += tmp[idx] * EROSION_RATE;
    }
  }
}

boolean run_particle(float[] array, float height, float slip) {
  int px = floor(random(0, MAP_WIDTH));
  int py = floor(random(0, MAP_HEIGHT));
  int i, idx, ndir, nx, ny;
  for (i = 0; i < MAX_WANDER_STEPS; ++i) {
    idx = px + py*MAP_WIDTH;
    if (px > 0 && array[idx - 1] >= height) {
      slip -= 1;
      if (slip <= 0) { break; }
    }
    if (px < MAP_WIDTH - 1 && array[idx + 1] >= height) {
      slip -= 1;
      if (slip <= 0) { break; }
    }
    if (py > 0 && array[idx - MAP_WIDTH] >= height) {
      slip -= 1;
      if (slip <= 0) { break; }
    }
    if (py < MAP_HEIGHT - 1 && array[idx + MAP_WIDTH] >= height) {
      slip -= 1;
      if (slip <= 0) { break; }
    }
    nx = px;
    ny = py;
    while (true) {
      ndir = floor(random(0, 4));
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
        println("Bad random!");
      }
      if (nx < 0 || nx > MAP_WIDTH - 1 || ny < 0 || ny > MAP_HEIGHT - 1) {
        continue;
      }
      idx = nx + ny*MAP_WIDTH;
      if (array[idx] < height) {
        break;
      }
    }
    px = nx;
    py = ny;
  }
  idx = px + py*MAP_WIDTH;
  if (array[idx] < height) {
    array[idx] = height;
    return true;
  } else {
    return false;
  }
}

void build_mountains() {
  int i, j;
  float height = HEIGHT;
  reset_array(MATRIX);
  println("Starting build cycles...");
  for (i = 0; i < BUILD_CYCLES; ++i) {
    print("\r  ...build cycle " + (i+1) + "/" + BUILD_CYCLES + " completed...");
    height *= 0.95;
    for (j = 0; j < STEP_PARTICLES; ++j) {
      run_particle(MATRIX, height, SLIP);
    }
  }
  println("\n  ...done with build cycles.");
  copy_array(MATRIX, SAVE);
  for (j = 0; j < SLUMP_STEPS; ++j) {
    slump_array(MATRIX, TMP);
    average_arrays(MATRIX, SAVE, 20.0);
  }
  for (j = 0; j < FINAL_SLUMPS; ++j) {
    slump_array(MATRIX, TMP);
  }
}

void setup() {
  int i;

  randomSeed(17);
  noiseSeed(17);
  // TODO: some way of seeding the Perlin library?

  colorMode(HSB, 1.0, 1.0, 1.0);
  size(WINDOW_WIDTH, WINDOW_HEIGHT);

  MATRIX = new float[MAP_WIDTH*MAP_HEIGHT];
  TMP = new float[MAP_WIDTH*MAP_HEIGHT];
  SAVE = new float[MAP_WIDTH*MAP_HEIGHT];
  reset_array(MATRIX);
  reset_array(TMP);
  reset_array(SAVE);

  /*
  println("Running " + BASE_PARTICLES + " particles...");
  for (i = 0; i < BASE_PARTICLES; ++i) {
    print("\r  ..." + i + "/" + BASE_PARTICLES + "...");
    run_particle(MATRIX, HEIGHT, SLIP);
  }
  */
  build_mountains();

  noLoop();
  redraw();
}


void draw() {
  int i, j, idx;
  background(0.6, 0.8, 0.25);
  noStroke();

  int cw = WINDOW_WIDTH / MAP_WIDTH;
  int ch = WINDOW_HEIGHT / MAP_HEIGHT;

  for (i = 0; i < MAP_WIDTH; ++i) {
    for (j = 0; j < MAP_HEIGHT; ++j) {
      idx = i + j * MAP_WIDTH;
      fill(colormap(MATRIX[idx]));
      rect(i*cw, j*ch, cw, ch);
    }
  }
}

void keyPressed() {
  int i;
  if (key == 'q') {
    exit();
  } else if (key == 'r') {
    reset_array(MATRIX);
  } else if (key == 'p') {
    run_particle(MATRIX, HEIGHT, SLIP);
  } else if (key == 'P') {
    for (i = 0; i < EXTRA_PARTICLES; ++i) {
      run_particle(MATRIX, HEIGHT, SLIP);
    }
  } else if (key == 'b') {
    blur_array(MATRIX, TMP);
  } else if (key == 's') {
    slump_array(MATRIX, TMP);
  } else if (key == 'm') {
    build_mountains();
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
  }
  redraw();
}
