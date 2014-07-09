// vim: syn=java

import perlin.*;

import java.util.Map;

Perlin pnl = new Perlin(this);

PImage biome_colors;

HashMap<Integer,Boolean> KEYS = new HashMap<Integer,Boolean>();

float BLOCK_SIZE = 4;

int COMPASS_X = 24;
int COMPASS_Y = 24;
int COMPASS_SIZE = 32;

float ZOOM = 1;
float ZOOM_FACTOR = 1.7;
float SEA_LEVEL = 0.5;
float SHORELINE = 0.005;

float TSCALE = 0.0005;
float HSCALE = 0.0005;
float ESCALE = 0.003;

float MOUNTAINS = 800;
float HILLS = 450;
float RIDGES = 120;
float BUMPS = 17;

float[] LIGHT_POSITION = { 20, -40, 10 };
float LIGHT_AMP = 1.5;

float[] PLAYER_POS = { 0, 0, 0 };
float PLAYER_ANGLE = -PI/2.0;
float MOVE_SPEED = 20;
float TURN_SPEED = PI/128.0;

float pnoise(float x, float y) {
  float[] xy = new float[2];
  xy[0] = x;
  xy[1] = y;
  return (1 + pnl.noise2(xy)) / 2.0;
}

float spread(float x) {
  float result;
  x = 2*x - 1;
  if (x >= 0) {
    result = (log(x+0.1) - log(0.1));
  } else {
    result = -(log(-x+0.1) - log(0.1));
  }
  result /= (log(1.1) - log(0.1));
  return (1 + result)/2.0;
}

float temperature(float x, float y) {
  float result = pnoise(x*TSCALE, y*TSCALE);
  result += 0.5 * pnoise(x*TSCALE*1.7, y*TSCALE*1.7);
  result /= 1.5;
  return result;
}

float humidity(float x, float y) {
  float result = pnoise(x*HSCALE+1000, y*HSCALE);
  result += 0.5 * pnoise(x*HSCALE*1.9+1000, y*HSCALE*1.9);
  result /= 1.5;
  return result;
}

float elevation(float x, float y) {
  float result = MOUNTAINS * pnoise(x*ESCALE*0.4+2000, y*ESCALE*0.4);
  result += HILLS * pnoise(x*ESCALE+2000, y*ESCALE);
  result += RIDGES * pnoise(x*ESCALE*2.4+2000, y*ESCALE*2.4);
  result += BUMPS * pnoise(x*ESCALE*4.1+2000, y*ESCALE*4.1);
  return result;
}

float shade(float x, float y, float z, float dx, float dy) {
  float[] pos = new float[3];
  float[] incidence = new float[3];
  float[] normal = new float[3];
  float len = 0;
  //println("pos: ", x, y, z);
  //println("dx/dy: ", dx, dy);
  pos[0] = x;
  pos[1] = y;
  pos[2] = z;

  normal[0] = -dx * LIGHT_AMP/ZOOM;
  normal[1] = -dy * LIGHT_AMP/ZOOM;
  normal[2] = 1;
  // normalize the normal:
  len = sqrt(
    normal[0]*normal[0] +
    normal[1]*normal[1] +
    normal[2]*normal[2]
  );
  normal[0] /= len;
  normal[1] /= len;
  normal[2] /= len;

  // the incident angle is the vector from the light position to the shading
  // position
  incidence[0] = pos[0] - LIGHT_POSITION[0];
  incidence[1] = pos[1] - LIGHT_POSITION[1];
  incidence[2] = pos[2] - LIGHT_POSITION[2];
  // normalize the incident direction:
  len = sqrt(
    incidence[0]*incidence[0] +
    incidence[1]*incidence[1] +
    incidence[2]*incidence[2]
  );
  incidence[0] /= len;
  incidence[1] /= len;
  incidence[2] /= len;

  // compute the dot product:
  return (
    normal[0]*incidence[0] +
    normal[1]*incidence[1] +
    normal[2]*incidence[2]
  );
}

void setup() {
  colorMode(HSB, 1.0, 1.0, 1.0);
  size(640, 480);
  noStroke();
  biome_colors = loadImage("biomes.png");
}

void draw() {
  background(0.5, 0.5, 0.45);
  int x, y;
  int relx, rely;
  int mapx, mapy;
  float temp;
  float humid;
  float elev;
  float sea_depth;
  int imgx, imgy;
  float dx, dy;
  float light;
  float max_elev = MOUNTAINS + HILLS + RIDGES + BUMPS;
  color sea_color;
  color land_color;
  for (x = 0; x < width; x += BLOCK_SIZE) {
    for (y = 0; y < height; y += BLOCK_SIZE) {
      /*
      mapx = floor(PLAYER_POS[0]/ZOOM - (width/2)) + x;
      mapy = floor(PLAYER_POS[1]/ZOOM - (height/2)) + y;
      */
      relx = x - (width/2);
      rely = y - (height/2);
      mapx = floor(PLAYER_POS[0]/ZOOM);
      mapy = floor(PLAYER_POS[1]/ZOOM);

      mapx += floor(relx*cos(PLAYER_ANGLE+PI/2.0));
      mapx += floor(rely*cos(PLAYER_ANGLE));

      mapy += floor(relx*sin(PLAYER_ANGLE+PI/2.0));
      mapy += floor(rely*sin(PLAYER_ANGLE));

      temp = temperature(mapx*ZOOM, mapy*ZOOM);
      temp = spread(temp);
      humid = humidity(mapx*ZOOM, mapy*ZOOM);
      humid = spread(humid);
      elev = elevation(mapx*ZOOM, mapy*ZOOM);
      dx = elev - elevation((mapx+1)*ZOOM, mapy*ZOOM);
      dy = elev - elevation(mapx*ZOOM, (mapy+1)*ZOOM);
      sea_depth = ((elev/max_elev)/SEA_LEVEL);
      sea_color = color(
        0.50 + 0.3 * (1 - sea_depth*sea_depth),
        0.8,
        0.3 + 0.5 * sea_depth
      );
      imgx = floor(temp * biome_colors.width);
      imgy = floor(humid * biome_colors.height);
      land_color = biome_colors.get(imgx, imgy);
      light = shade(mapx, mapy, elev, dx, dy);
      land_color = color(
        hue(land_color),
        saturation(land_color),
        0.7*(elev/max_elev) + 0.3*light
      );
      if (
        (elev/max_elev < SEA_LEVEL) &&
        (elev/max_elev > (SEA_LEVEL - SHORELINE))
      ) {
        fill(
          lerpColor(
            sea_color,
            land_color,
            (elev/max_elev - (SEA_LEVEL - SHORELINE)) / SHORELINE
          )
        );
      } else if (elev/max_elev < SEA_LEVEL) {
        fill(sea_color);
      } else {
        fill(land_color);
      }
      rect(x, y, BLOCK_SIZE, BLOCK_SIZE);
    }
  }
  // Draw a compass:
  stroke(0, 0, 0.3);
  fill(0, 0, 0.45);
  ellipse(width - COMPASS_X, height - COMPASS_Y, COMPASS_SIZE, COMPASS_SIZE);
  stroke(1, 1, 1);
  line(
    width - COMPASS_X,
    height - COMPASS_Y,
    width - COMPASS_X + (COMPASS_SIZE/2-3)*cos(PLAYER_ANGLE),
    height - COMPASS_Y + (COMPASS_SIZE/2-3)*sin(PLAYER_ANGLE)
  );
  noStroke();
  // continuous input:
  if (KEYS.get((int) 'w') != null && KEYS.get((int) 'w')) {
    PLAYER_POS[0] -= cos(PLAYER_ANGLE)*MOVE_SPEED;
    PLAYER_POS[1] -= sin(PLAYER_ANGLE)*MOVE_SPEED;
  }
  if (KEYS.get((int) 's') != null && KEYS.get((int) 's')) {
    PLAYER_POS[0] += cos(PLAYER_ANGLE)*MOVE_SPEED;
    PLAYER_POS[1] += sin(PLAYER_ANGLE)*MOVE_SPEED;
  }
  if (KEYS.get((int) 'a') != null && KEYS.get((int) 'a')) {
    PLAYER_ANGLE += TURN_SPEED;
  }
  if (KEYS.get((int) 'd') != null && KEYS.get((int) 'd')) {
    PLAYER_ANGLE -= TURN_SPEED;
  }
}

void keyPressed() {
  if (key == 'q') {
    exit();
  } else if (key == 'j') {
    ZOOM *= ZOOM_FACTOR;
  } else if (key == 'k') {
    ZOOM /= ZOOM_FACTOR;
  }
  KEYS.put((int) key, true);
}

void keyReleased() {
  KEYS.put((int) key, false);
}
