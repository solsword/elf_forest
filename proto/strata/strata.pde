// vim: syn=java

import perlin.*;

Perlin pnl = new Perlin(this);

/*
int WIDTH = 640;
int HEIGHT = 480;
/*/
int WIDTH = 400;
int HEIGHT = 300;
// */

float RADIAL_NOISE_SCALE = 2.4;
float TNOISE_SCALE = 0.03;

float DNOISE_SCALE = 0.02;
float DNOISE_ATTENUATE = 0.8;
float DNOISE_STRENGTH = 60;

float SNOISE_SCALE = 0.0017;
float SNOISE_STRENGTH = 270;

//int N_BLOBS = 1;
//int N_BLOBS = 35;
int N_BLOBS = 45;
//int N_BLOBS = 200;

//float MIN_R = 210;
//float MAX_R = 340;
float MIN_R = 70;
float MAX_R = 230;
float MIN_VAR = 0.2;
float MAX_VAR = 0.5;
float MIN_LOG_T = 1.2; // 1.2;
float MAX_LOG_T = 1.8; // 1.4;

float GRID_MARGINS = 120.0;
//float PGRID_SCALE = 15;
float PGRID_SCALE = 40;

float WORLEY_STRENGTH = 0.8;
float PERLIN_STRENGTH = 0.1;

boolean DISTORT_BLOBS = true;
boolean BLOB_THICKNESS_PARABOLIC = true;
boolean BLOB_PSEUDO_POISSON_MODE = true;
boolean ADD_BLOBS = true;
boolean ADD_WORLEY = true;
boolean ADD_PERLIN = true;
boolean TEST_NOISE = false;
boolean TEST_GRADIENT = true;
boolean SHADE_TERRAIN = true;
int EROSION_STEPS = 0;

int N_TRAILS = 30;
int STEPS_PER_TRAIL = 100;

float SEA_LEVEL = 0.7;
float SEA_LEVEL_STEP = 0.05;
boolean DRAW_COLOR = false;

float[] LIGHT_POSITION = {120, -40, 10};

float MAX_WORLEY_DIST = sqrt(2.0);

int[] HASH = { // a tiny hash table
  0, 14, 15, 6,
  8, 12, 4, 13,
  2, 1, 9, 10,
  7, 3, 11, 5,
  0, 14, 15, 6,
  8, 12, 4, 13,
  2, 1, 9, 10,
  7, 3, 11, 5
};

float spread_up(float x) {
  float result;
  if (x >= 0) {
    result = (log(x+0.1) - log(0.1));
  } else {
    result = -(log(-x+0.1) - log(0.1));
  }
  result /= (log(1.1) - log(0.1));
  return result;
}

float spline_up(float x) {
  float result, l1, l2;
  float t;
  if (x >= 0) {
    t = x;
  } else {
    t = -x;
  }
  l1 = 0;
  //l1 = lerp(0.0, 0.2, t);
  l2 = lerp(1.5, 0.9, t);
  result = lerp(l1, l2, t);
  if (x >= 0) {
    return result;
  } else {
    return -result;
  }
}

float spread_down(float x) {
  if (x >= 0) {
    return x*x;
  } else {
    return -x*x;
  }
}

float pnoise(float x, float y) {
  float[] xy = new float[2];
  xy[0] = x;
  xy[1] = y;
  return (1 + pnl.noise2(xy)) / 2.0;
}

float pnoise(float x, float y, float z) {
  float[] xyz = new float[3];
  xyz[0] = x;
  xyz[1] = y;
  xyz[2] = z;
  float result = (1 + pnl.noise3(xyz)) / 2.0;
  if (result < 0 || result > 1.0) {
    println("Bad result: ", result);
  }
  return result;
}

float spnoise(float x, float y) {
  float[] xy = new float[2];
  xy[0] = x;
  xy[1] = y;
  return (1 + spread_up(pnl.noise2(xy))) / 2.0;
}

float spnoise(float x, float y, float z) {
  float[] xyz = new float[3];
  xyz[0] = x;
  xyz[1] = y;
  xyz[2] = z;
  float result = (1 + spread_up(pnl.noise3(xyz))) / 2.0;
  if (result < 0 || result > 1.0) {
    println("Bad result: ", result);
  }
  return result;
}

float splnoise(float x, float y) {
  float[] xy = new float[2];
  xy[0] = x;
  xy[1] = y;
  return (1 + spline_up(pnl.noise2(xy))) / 2.0;
}

float splnoise(float x, float y, float z) {
  float[] xyz = new float[3];
  xyz[0] = x;
  xyz[1] = y;
  xyz[2] = z;
  float result = (1 + spline_up(pnl.noise3(xyz))) / 2.0;
  if (result < 0 || result > 1.0) {
    println("Bad result: ", result);
  }
  return result;
}

class Map {
  float cells[];
  Map() {
    int i;
    this.cells = new float[WIDTH*HEIGHT];
    for (i = 0; i < WIDTH*HEIGHT; ++i) {
      this.cells[i] = 0;
    }
  }
  void add_up(Blob b) {
    int x, y, i;
    for (x = 0; x < WIDTH; ++x) {
      for (y = 0; y < HEIGHT; ++y) {
        i = x + y*WIDTH;
        this.cells[i] += b.thickness_at(x, y);
      }
    }
  }
  void bake() {
    int x, y, i;
    float max = 0;
    for (x = 0; x < WIDTH; ++x) {
      for (y = 0; y < HEIGHT; ++y) {
        i = x + y*WIDTH;
        if (this.cells[i] > max) {
          max = this.cells[i];
        }
      }
    }
    for (x = 0; x < WIDTH; ++x) {
      for (y = 0; y < HEIGHT; ++y) {
        i = x + y*WIDTH;
        this.cells[i] /= max;
      }
    }
  }
  void draw() {
    int x, y, i;
    float hue, sat, bri;
    float h;
    Point gradient;
    float [] v = new float[3];
    float [] normal;
    float [] lv;
    noFill();
    for (x = 0; x < WIDTH; ++x) {
      for (y = 0; y < HEIGHT; ++y) {
        i = x + y*WIDTH;
        h = this.cells[i];
        if (DRAW_COLOR) {
          if (h < SEA_LEVEL) {
            hue = 0.7;
            sat = 0.4;
          } else {
            hue = 0.06 + 0.24*h;
            sat = 0.3;
          }
        } else {
          hue = 0.0;
          sat = 0.0;
        }
        bri = h;
        if (SHADE_TERRAIN) {
          gradient = this.gradient(x, y);
          gradient.x *= 120;
          gradient.y *= 120;
          v[0] = (float) x;
          v[1] = (float) y;
          v[2] = h;
          normal = compute_normal(gradient);
          //println("normal: ", normal[0], normal[1], normal[2]);
          lv = vsub(LIGHT_POSITION, v);
          bri = 0.7 * h + 0.3 * (1 + dot(norm(normal), norm(lv))) / 2.0;
          //bri = (1 + dot(norm(normal), norm(lv))) / 2.0;
        }
        stroke(hue, sat, bri);
        point(x, y);
      }
    }
  }

  float get(int x, int y) {
    int i;
    if (x < 0) {
      x = 0;
    } else if (x >= WIDTH) {
      x = WIDTH - 1;
    }
    if (y < 0) {
      y = 0;
    } else if (y >= HEIGHT) {
      y = HEIGHT - 1;
    }
    i = x+y*WIDTH;
    return this.cells[i];
  }

  Point gradient(int x, int y) {
    float here, north, east, south, west, ne, se, sw, nw;
    float dx, dy;
    here = this.get(x, y);
    north = this.get(x, y-1);
    ne = this.get(x+1, y-1);
    east = this.get(x+1, y);
    se = this.get(x+1, y+1);
    south = this.get(x, y+1);
    sw = this.get(x-1, y+1);
    west = this.get(x-1, y);
    nw = this.get(x-1, y-1);
    dx = (
      0.5 * (ne - here) +
      (east - here) +
      0.5 * (se - here) +
      0.5 * (here - sw) +
      (here - west) +
      0.5 * (here - nw)
    ) / 4.0;
    dy = (
      0.5 * (se - here) +
      (south - here) +
      0.5 * (sw - here) +
      0.5 * (here - nw) +
      (here - north) +
      0.5 * (here - ne)
    ) / 4.0;
    return new Point(dx, dy);
  }
}

class Blob {
  int seed;
  int x, y; // center
  float r; // base radius
  float var; // how much the noise affects the radius
  float t; // base thickness
  Blob(int x, int y, float r, float var, float t,int seed) {
    this.x = x;
    this.y = y;
    this.r = r;
    this.var = var;
    this.t = t;
    this.seed = seed;
  }
  // radius as a function of direction:
  float radius(float dir) {
    float rns = RADIAL_NOISE_SCALE;
    float n = pnoise(rns*cos(dir), rns*sin(dir), this.seed*2.1);
    n += 0.4 * pnoise(rns*cos(dir), rns*sin(dir), this.seed*4.3);
    n /= 1.4;
    return this.r * (
      (1 - this.var) +
      2*this.var * n
    );
  }
  float thickness_at(float x, float y) {
    float dx, dy;
    dx = 0;
    dy = 0;
    if (DISTORT_BLOBS) {
      dx += splnoise(x*DNOISE_SCALE, y*DNOISE_SCALE, this.seed*3.2);
      dx += 0.4*splnoise(x*DNOISE_SCALE*2.1, y*DNOISE_SCALE*2.1, this.seed*4.2);
      dx *= DNOISE_STRENGTH/1.4;
      dy += splnoise(x*DNOISE_SCALE, y*DNOISE_SCALE, this.seed*6.8);
      dy += 0.4*splnoise(x*DNOISE_SCALE*2.1, y*DNOISE_SCALE*2.1, this.seed*9.1);
      dy *= DNOISE_STRENGTH/1.4;
      dx *= (
        (1 - DNOISE_ATTENUATE) +
        DNOISE_ATTENUATE * splnoise(
          x*DNOISE_SCALE*0.4,
          y*DNOISE_SCALE*0.4,
          this.seed*2.3
        )
      );
      dy *= (
        (1 - DNOISE_ATTENUATE) +
        DNOISE_ATTENUATE * splnoise(
          x*DNOISE_SCALE*0.4,
          y*DNOISE_SCALE*0.4,
          this.seed*4.9
        )
      );
      dx += SNOISE_STRENGTH * splnoise(
        x*SNOISE_SCALE,
        y*SNOISE_SCALE,
        this.seed*11.4
      );
      dy += SNOISE_STRENGTH * splnoise(
        x*SNOISE_SCALE,
        y*SNOISE_SCALE,
        this.seed*14.1
      );
    }
    dx += x;
    dy += y;
    return this.thickness_at_base(dx, dy);
  }
  float thickness_at_base(float x, float y) {
    float dir = atan2(y - this.y, x - this.x);
    float r = this.radius(dir);
    float d = sqrt(pow(x - this.x, 2) + pow(y - this.y, 2));
    float rinterp = 1 - (d / r);
    if (rinterp > 0) {
      if (BLOB_THICKNESS_PARABOLIC) {
        // parabolic interpolation
        rinterp = sqrt(rinterp);
      } else {
        // semi-parabolic interpolation
        rinterp = 0.5 * rinterp + 0.5 * sqrt(rinterp);
      }
    }
    float n = pnoise(x*TNOISE_SCALE, y*TNOISE_SCALE, this.seed);
    n += 0.4 * pnoise(
      x*TNOISE_SCALE*2,
      y*TNOISE_SCALE*2,
      this.seed*2
    );
    n /= 1.4;
    return max(0, rinterp * this.t * (0.4 + 0.6 * n));
  }
}

class Point {
  float x, y;
  Point(float ix, float iy) {
    this.x = ix;
    this.y = iy;
  }
  Point copy() {
    return new Point(this.x, this.y);
  }
  float distance(Point other) {
    return sqrt(pow(this.x - other.x, 2) + pow(this.y - other.y, 2));
  }
  void add(Point other) {
    this.x += other.x;
    this.y += other.y;
  }
}

Point jostle(Point there, Point here) {
  float d = here.distance(there) - 1.0;
  return new Point(
    (there.x - here.x) * d,
    (there.y - here.y) * d
  );
}

float[] compute_normal(Point gradient) {
  float[] n = new float[3];
  n[0] = -gradient.x;
  n[1] = -gradient.y;
  n[2] = 1;
  return n;
}

float dot(float[] a, float[] b) {
  float result = 0;
  result += a[0] * b[0];
  result += a[1] * b[1];
  result += a[2] * b[2];
  return result;
}

float mag(float[] v) {
  return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

float[] norm(float[] v) {
  float[] result = new float[3];
  float m = mag(v);
  result[0] = v[0] / m;
  result[1] = v[1] / m;
  result[2] = v[2] / m;
  return result;
}

// a minus b
float [] vsub(float[] a, float[] b) {
  float[] result = new float[3];
  result[0] = a[0] - b[0];
  result[1] = a[1] - b[1];
  result[2] = a[2] - b[2];
  return result;
}

class PseudoPoissonDistribution {
  int width;
  int height;
  int seed;
  ArrayList<Point> points;
  IntList ordering;
  int current_index;
  PseudoPoissonDistribution(int width, int height, int seed) {
    this.width = width;
    this.height = height;
    this.seed = seed;
    this.points = new ArrayList<Point>();
    this.ordering = new IntList();
    float x, y;
    int i;
    for (y = 0; y < this.height; ++y) {
       for (x = 0; x < this.width; ++x) {
        this.points.add(
          new Point(
            x + pnoise(x, y, this.seed),
            y + pnoise(x, y, this.seed + 20)
          )
        );
      }
    }
    for (i = 0; i < this.points.size(); ++i) {
      this.ordering.append(i);
      Point p = this.points.get(i);
    }
    this.ordering.shuffle();
    this.current_index = 0;
    this.relax();
  }
  
  Point get(int x, int y) {
    if (x < 0 || y < 0 || x >= this.width || y >= this.height) {
      return new Point(x, y);
    }
    int i = x + y*this.width;
    if (i < 0 || i > this.points.size()) {
      println("G bad");
      return new Point(0, 0);
    }
    return this.points.get(i);
  }

  void relax() {
    int x, y;
    Point here, north, east, south, west, offset;
    for (x = 0; x < this.width; ++x) {
      for (y = 0; y < this.height; ++y) {
        here = this.get(x, y);
        north = this.get(x, y-1);
        east = this.get(x+1, y);
        south = this.get(x, y+1);
        west = this.get(x-1, y);
        offset = new Point(0, 0);
        offset.add(jostle(north, here));
        offset.add(jostle(east, here));
        offset.add(jostle(south, here));
        offset.add(jostle(west, here));
        offset.x /= 4.0;
        offset.y /= 4.0;
        here.add(offset);
      }
    }
  }

  Point next() {
    this.current_index = (this.current_index + 1) % this.ordering.size();
    return this.points.get(this.ordering.get(this.current_index));
  }
}

class WorleyNoise {
  int seed;

  WorleyNoise(int seed) {
    this.seed = seed;
  }

  Point grid_point(float x, float y) {
    int ix = (int) floor(x);
    int iy = (int) floor(y);
    return new Point(
      ix + splnoise(ix*7.7, iy*5.4, this.seed*7.1),
      iy + splnoise(ix*4.2, iy*6.9, this.seed*8.3)
    );
  }

  float strength(float x, float y) {
    int ix = (int) floor(x);
    int iy = (int) floor(y);
    return 0.4 + 0.6 * splnoise(ix*8.4, iy*2.3, this.seed*4.5);
  }

  float value(float x, float y) {
    float[] strengths = new float[9];
    Point[] neighbors = new Point[9];
    neighbors[0] = grid_point(x, y);
    strengths[0] = strength(x, y);
    neighbors[1] = grid_point(x, y+1);
    strengths[1] = strength(x, y+1);
    neighbors[2] = grid_point(x+1, y);
    strengths[2] = strength(x+1, y);
    neighbors[3] = grid_point(x, y-1);
    strengths[3] = strength(x, y-1);
    neighbors[4] = grid_point(x-1, y);
    strengths[4] = strength(x-1, y);
    neighbors[5] = grid_point(x+1, y+1);
    strengths[5] = strength(x+1, y+1);
    neighbors[6] = grid_point(x+1, y-1);
    strengths[6] = strength(x+1, y-1);
    neighbors[7] = grid_point(x-1, y-1);
    strengths[7] = strength(x-1, y-1);
    neighbors[8] = grid_point(x-1, y+1);
    strengths[8] = strength(x-1, y+1);
    Point here = new Point(x, y);
    int i;
    float first = 0.0;
    float second = 0.0;
    float str;
    for (i = 0; i < neighbors.length; ++i) {
      str = (MAX_WORLEY_DIST - here.distance(neighbors[i])) * strengths[i];
      if (str > first) {
        second = first;
        first = str;
      } else if (str > second) {
        second = str;
      }
    }
    float result = first - second;
    result /= MAX_WORLEY_DIST;
    result = 1 - result;
    result = sqrt(result);
    result = sqrt(result);
    return result;
  }
}

Map THE_MAP;
ArrayList<ArrayList<Point>> TRAILS;

void setup() {
  noiseDetail(1,0.5);
  noiseSeed(17);
  colorMode(HSB, 1.0, 1.0, 1.0);
  size(WIDTH, HEIGHT);
  THE_MAP = new Map();
  int x, y, i;
  float str;
  float wr;
  if (TEST_NOISE) {
    for (x = 0; x < WIDTH; ++x) {
      for (y = 0; y < HEIGHT; ++y) {
        i = x + y*WIDTH;
        if (x < WIDTH/3) {
          THE_MAP.cells[i] = pnoise(x*0.05, y*0.05, 17.3);
        } else if (x < 2*WIDTH/3) {
          THE_MAP.cells[i] = splnoise(x*0.05, y*0.05, 17.3);
        } else {
          THE_MAP.cells[i] = spnoise(x*0.05, y*0.05, 17.3);
        }
      }
    }
  } else {
    if (ADD_BLOBS) {
      Blob b;
      Point p;
      PseudoPoissonDistribution ppois = new PseudoPoissonDistribution(
        (int) ((WIDTH+2*GRID_MARGINS)/PGRID_SCALE),
        (int) ((HEIGHT+2*GRID_MARGINS)/PGRID_SCALE),
        173
      );
      for (i = 0; i < N_BLOBS; ++i) {
        if (BLOB_PSEUDO_POISSON_MODE) {
          p = ppois.next();
          x = (int) ((p.x * PGRID_SCALE) - GRID_MARGINS);
          y = (int) ((p.y * PGRID_SCALE) - GRID_MARGINS);
        } else {
          x = (int) (random(-GRID_MARGINS, WIDTH + GRID_MARGINS));
          y = (int) (random(-GRID_MARGINS, HEIGHT + GRID_MARGINS));
        }
        b = new Blob(
          x,
          y,
          random(MIN_R, MAX_R),
          random(MIN_VAR, MAX_VAR),
          exp(random(MIN_LOG_T, MAX_LOG_T)),
          i + HASH[HASH[abs(x)%16] + abs(y)%16]
        );
        THE_MAP.add_up(b);
      }
      THE_MAP.bake();
    }
    if (ADD_WORLEY) {
      WorleyNoise wrn = new WorleyNoise(137);
      for (x = 0; x < WIDTH; ++x) {
        for (y = 0; y < HEIGHT; ++y) {
          i = x + y*WIDTH;
          str = WORLEY_STRENGTH * (0.4 + 0.6 * splnoise(x*0.015, y*0.015, 1.4));
          wr = wrn.value(x*0.02, y*0.02);
          //THE_MAP.cells[i] += str * wr + (1 - str) * 0.2;
          THE_MAP.cells[i] += str * wr;
        }
      }
      THE_MAP.bake();
    }
    if (ADD_PERLIN) {
      float nlow, nmid, nhigh, n;
      for (x = 0; x < WIDTH; ++x) {
        for (y = 0; y < HEIGHT; ++y) {
          i = x + y*WIDTH;
          nlow = splnoise(x*0.05, y*0.05, 17.3);
          nmid = splnoise(x*0.1, y*0.1, 31.7);
          nhigh = splnoise(x*0.4, y*0.4, 37.1);
          nlow *= (0.3 + 0.7 * splnoise(x*0.008, y*0.008, 73.1));
          nmid *= splnoise(x*0.03, y*0.03, 71.3);
          nhigh *= splnoise(x*0.02, y*0.02, 13.7);
          n = nlow + 0.5 * nmid + 0.25 * nhigh;
          n *= PERLIN_STRENGTH / 1.75;
          THE_MAP.cells[i] += n;
        }
      }
      THE_MAP.bake();
    }
    if (TEST_GRADIENT) {
      int j;
      TRAILS = new ArrayList<ArrayList<Point>>(N_TRAILS);
      for (i = 0; i < N_TRAILS; ++i) {
        TRAILS.add(new ArrayList<Point>(STEPS_PER_TRAIL));
        Point particle = new Point(random(WIDTH), random(HEIGHT));
        Point g;
        ArrayList<Point> trail = TRAILS.get(TRAILS.size() - 1);
        for (j = 0; j < STEPS_PER_TRAIL; ++j) {
          trail.add(particle.copy());
          g = THE_MAP.gradient((int) particle.x, (int) particle.y);
          particle.x -= g.x*1400;
          particle.y -= g.y*1400;
        }
      }
    }
  }
  noLoop();
  redraw();
}

void draw() {
  background(0.5, 0.5, 0.45);
  THE_MAP.draw();

  int i, j;
  Point lp, p;
  float t;
  ArrayList<Point> trail;
  if (TEST_GRADIENT) {
    for (i = 0; i < TRAILS.size(); ++i) {
      trail = TRAILS.get(i);
      lp = trail.get(0);
      for (j = 0; j < trail.size(); ++j) {
        t = ((float) j) / ((float) trail.size());
        stroke(0.06, 1.0, (1 - t));
        p = trail.get(j);
        line(lp.x, lp.y, p.x, p.y);
        lp = p;
      }
    }
  }
}

void keyPressed() {
  if (key == 'q') {
    exit();
  } else if (key == 'k') {
    SEA_LEVEL += SEA_LEVEL_STEP;
    if (SEA_LEVEL > 1) { SEA_LEVEL = 1; }
  } else if (key == 'j') {
    SEA_LEVEL -= SEA_LEVEL_STEP;
    if (SEA_LEVEL < 0) { SEA_LEVEL = 0; }
  } else if (key == 'c') {
    DRAW_COLOR = !DRAW_COLOR;
  } else if (key == 's') {
    SHADE_TERRAIN = !SHADE_TERRAIN;
  }
  redraw();
}
