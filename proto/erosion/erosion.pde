// vim: syn=java

import perlin.*;

Perlin pnl = new Perlin(this);

Map THE_MAP;
Map MAP_TWO;

int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;

int MAP_WIDTH = 8;// 80;
int MAP_HEIGHT = 6;// 60;

float SEA_LEVEL = 0.0;

float GRID_RESOLUTION = 100.0;

float SCALE = 0.1;
float NOISE_SCALE_TINY =  0.014;
float NOISE_SCALE_SMALL = 0.042;
float NOISE_SCALE_MEDIUM = 0.104;
float NOISE_SCALE_LARGE = 0.225;

float UNTANGLE_OFFSET = 10.0;


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


float[] colormap(float h) {
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
  return result;
}


class Map {
  float xs[];
  float ys[];
  float zs[];
  int width;
  int height;
  float cx, cy, cz;
  Map(int width, int height) {
    int i, j;
    this.width = width;
    this.height = height;
    this.xs = new float[width*height];
    this.ys = new float[width*height];
    this.zs = new float[width*height];
    this.cx = ((this.width-1) * GRID_RESOLUTION) / 2.0;
    this.cy = ((this.height-1) * GRID_RESOLUTION) / 2.0;
    this.cz = 0.0;
    for (i = 0; i < width; ++i) {
      for (j = 0; j < height; ++j) {
        this.xs[j*this.width + i] = i * GRID_RESOLUTION;
        this.ys[j*this.width + i] = j * GRID_RESOLUTION;
        this.zs[j*this.width + i] = 0.0;
      }
    }
  }

  void update_center() {
    int i;
    this.cx = 0;
    this.cy = 0;
    this.cz = 0;
    for (i = 0; i < this.width*this.height; ++i) {
      this.cx += this.xs[i];
      this.cy += this.ys[i];
      this.cz += this.zs[i];
    }
    this.cx /= (float) (this.width*this.height);
    this.cy /= (float) (this.width*this.height);
    this.cz /= (float) (this.width*this.height);
  }

  void stretch(float width, float height) {
    // Stretches the grid to a specified minimum width/height
    this.update_center();
    float min_x, max_x, min_y, max_y;
    min_x = this.cx - width/2.0;
    max_x = this.cx + width/2.0;
    min_y = this.cy - height/2.0;
    max_y = this.cy + height/2.0;
    int i, j;
    // First, pull all edge points straight outwards to meed the desired
    // minimum dimensions as necessary:
    for (i = 0; i < this.width; ++i) {
      if (this.ys[i] < min_y) {
        this.ys[i] = min_y;
      }
      if (this.ys[(this.height - 1)*this.width + i] > max_y) {
        this.ys[(this.height - 1)*this.width + i] = max_y;
      }
    }
    for (j = 0; j < this.height; ++j) {
      if (this.xs[j*this.width] < min_x) {
        this.xs[j*this.width] = min_x;
      }
      if (this.xs[j*this.width + this.width-1] > max_x) {
        this.xs[j*this.width + this.width-1] = max_x;
      }
    }
    // Next, settle down the inner points holding edge points in place:
    // TODO: Settle!
  }

  void rustle(float strength) {
    // Randomly moves each grid point a little bit. Strength should be in
    // [0, 1] and represents the max perturbation as a percentage of the
    // default between-point distance.
    float s = strength*GRID_RESOLUTION;
    float x, y;
    int i, j;
    for (i = 0; i < this.width; ++i) {
      for (j = 0; j < this.height; ++j) {
        x = this.xs[j*this.width+i];
        y = this.ys[j*this.width+i];
        this.xs[j*this.width+i] += s * pnoise(
          x*NOISE_SCALE_TINY,
          y*NOISE_SCALE_TINY
        );
        this.ys[j*this.width+i] += s * pnoise(
          x*NOISE_SCALE_TINY + 10000.0,
          y*NOISE_SCALE_TINY
        );
      }
    }
  }

  void untangle(boolean fierce) {
    // Untangles the graph by expanding it rightwards and downwards as
    // necessary. If "fierce" is specified, untangles based on the maximum
    // diagonal principle as well as the adjacent overlapping principle.
    float x, y, dx, dy;
    int i, j, idx;
    for (i = 0; i < this.width; ++i) {
      for (j = 0; j < this.height; ++j) {
        idx = j*this.width + i;
        x = this.xs[idx];
        y = this.ys[idx];
        if (i < this.width - 1) {
          dx = this.xs[idx + 1] - x;
          dy = this.ys[idx + 1] - y;
          if (dx < UNTANGLE_OFFSET) {
            this.xs[idx + 1] = x + UNTANGLE_OFFSET;
            dx = UNTANGLE_OFFSET;
          }
          if (dy > dx && fierce) {
            this.ys[idx + 1] = y + dx;
          }
          if (dy < -dx && fierce) {
            this.ys[idx + 1] = y - dx;
          }
        }
        if (j < this.height - 1) {
          dx = this.xs[idx + this.width] - x;
          dy = this.ys[idx + this.width] - y;
          if (dy < UNTANGLE_OFFSET) {
            this.ys[idx + this.width] = y + UNTANGLE_OFFSET;
            dy = UNTANGLE_OFFSET;
          }
          if (dx > dy && fierce) {
            this.xs[idx + this.width] = x + dy;
          }
          if (dx < -dy && fierce) {
            this.xs[idx + this.width] = x - dy;
          }
        }
      }
    }
  }

  void render_wireframe(float scale) {
    int i, j;
    for (i = 0; i < this.width; ++i) {
      for (j = 0; j < this.height; ++j) {
        if (i > 0) { // west
          line(
            scale * this.xs[j*this.width+i],
            scale * this.ys[j*this.width+i],
            scale * this.xs[j*this.width+i-1],
            scale * this.ys[j*this.width+i-1]
          );
        }
        if (i < this.width - 1) { // east
          line(
            scale * this.xs[j*this.width+i],
            scale * this.ys[j*this.width+i],
            scale * this.xs[j*this.width+i+1],
            scale * this.ys[j*this.width+i+1]
          );
        }
        if (j > 0) { // north
          line(
            scale * this.xs[j*this.width+i],
            scale * this.ys[j*this.width+i],
            scale * this.xs[(j-1)*this.width+i],
            scale * this.ys[(j-1)*this.width+i]
          );
        }
        if (j < this.height - 1) { // south
          line(
            scale * this.xs[j*this.width+i],
            scale * this.ys[j*this.width+i],
            scale * this.xs[(j+1)*this.width+i],
            scale * this.ys[(j+1)*this.width+i]
          );
        }
      }
    }
  }

  void render() {
    // TODO: HERE!
  }
}

void setup() {
  randomSeed(17);
  noiseSeed(17);
  // TODO: some way of seeding the Perlin library?
  colorMode(HSB, 1.0, 1.0, 1.0);
  size(WINDOW_WIDTH, WINDOW_HEIGHT);
  THE_MAP = new Map(MAP_WIDTH, MAP_HEIGHT);
  MAP_TWO = new Map(MAP_WIDTH, MAP_HEIGHT);
  THE_MAP.rustle(3.8);
  MAP_TWO.rustle(3.8);
  MAP_TWO.untangle(true);
  noLoop();
  redraw();
}


void draw() {
  background(0.6, 0.8, 0.25);
  stroke(0.0, 0.0, 1.0);
  // Center the map:
  pushMatrix();
  THE_MAP.update_center();
  translate(-THE_MAP.cx*SCALE, -THE_MAP.cy*SCALE);
  translate(WINDOW_WIDTH/2.0, WINDOW_HEIGHT/4.0);
  // And draw it:
  THE_MAP.render_wireframe(SCALE);
  popMatrix();
  // Map two gets drawn with an offset:
  pushMatrix();
  THE_MAP.update_center();
  translate(-MAP_TWO.cx*SCALE, -MAP_TWO.cy*SCALE);
  translate(WINDOW_WIDTH/2.0, 3.0 * WINDOW_HEIGHT/4.0);
  // And draw it:
  MAP_TWO.render_wireframe(SCALE);
  popMatrix();
}

void keyPressed() {
  if (key == 'q') {
    exit();
  } else if (key == 'k') {
    SCALE *= 1.5;
  } else if (key == 'j') {
    SCALE /= 1.5;
  }
  redraw();
}
