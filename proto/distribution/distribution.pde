
float resx = 20;
float resy = 20;

int fixed_radius = 0;
int maxr = 40;

float minres = 5;
float maxres = 500;

boolean show_init = false;
boolean all_connected = false;

float OFFSET_DISTANCE = 0.9;

float neutral_dist = 1.0;
float minndist = 0.15;
float maxndist = 3.0;

// The noise scale:
float ns = 0.5;
float maxns = 3;
float minns = 0.05;

int CONNECT_NORTH = 0x01;
int CONNECT_EAST = 0x02;
int CONNECT_SOUTH = 0x04;
int CONNECT_WEST = 0x08;

// A scrambled list of possible 1- and 2- connections:
int N_CONNECTIONS = 10;
int[] CONNECTIONS = {
  CONNECT_NORTH | CONNECT_EAST,
  CONNECT_EAST | CONNECT_SOUTH,
  CONNECT_WEST,
  CONNECT_EAST | CONNECT_WEST,
  CONNECT_SOUTH | CONNECT_WEST,
  CONNECT_NORTH,
  CONNECT_EAST,
  CONNECT_NORTH | CONNECT_WEST,
  CONNECT_NORTH | CONNECT_SOUTH,
  CONNECT_SOUTH
};


class Point {
  float x, y;
  Point(float ix, float iy) {
    x = ix;
    y = iy;
  }
  float distance(Point other) {
    return sqrt(pow(x - other.x, 2) + pow(y - other.y, 2));
  }
  void add(Point other) {
    x += other.x;
    y += other.y;
  }
}

void setup() 
{
  size(800, 600);
  colorMode(RGB, 255, 255, 255);  
  background(45);
  noiseSeed(0);
  noSmooth();
}

Point pinitial(Point in) {
      float offx = OFFSET_DISTANCE * 2.0 * (noise(in.x*ns, in.y*ns, 117) - 0.5);
      float offy = OFFSET_DISTANCE * 2.0 * (noise(in.x*ns, in.y*ns, 11356) - 0.5);
      return new Point(in.x + offx, in.y + offy);
}

Point jostle(Point there, Point here) {
      float d = 0;
      Point offset = new Point(0, 0);
      d = here.distance(there) - neutral_dist;
      offset.x += (there.x - here.x) * d;
      offset.y += (there.y - here.y) * d;
      return offset;
}

Point prelaxed(Point in) {
      Point here = pinitial(in);
      Point north = pinitial(new Point(in.x, in.y-1));
      Point south = pinitial(new Point(in.x, in.y+1));
      Point east = pinitial(new Point(in.x+1, in.y));
      Point west = pinitial(new Point(in.x-1, in.y));
      Point offset = new Point(0, 0);
      if (all_connected) {
        offset.add(jostle(north, here));
        offset.add(jostle(south, here));
        offset.add(jostle(east, here));
        offset.add(jostle(west, here));
        offset.x /= 4.0;
        offset.y /= 4.0;
      } else {
        int con = CONNECTIONS[
          ((int) (noise(in.x, in.y, 93823) * N_CONNECTIONS)) % N_CONNECTIONS
        ];
        int count = 0;
        if ((con & CONNECT_NORTH) != 0) { offset.add(jostle(north, here)); count += 1; }
        if ((con & CONNECT_EAST) != 0) { offset.add(jostle(east, here)); count += 1; }
        if ((con & CONNECT_SOUTH) != 0) { offset.add(jostle(south, here)); count += 1; }
        if ((con & CONNECT_WEST) != 0) { offset.add(jostle(west, here)); count += 1; }
        offset.x /= (float) count;
        offset.y /= (float) count;
      }
      here.add(offset);
      return here;
}

void draw() {
  background(45);
  noFill();
  strokeWeight(1);
  for (float i = 0; i < width/resx; i += 1) {
    for (float j = 0; j < height/resy; j += 1) {
      Point rel = prelaxed(new Point(i, j));
      Point ini = pinitial(new Point(i, j));

      stroke(60);
      line(i*resx, j*resy, ini.x*resx, ini.y*resy);

      float radius = resx;
      if (fixed_radius != 0) {
        radius = fixed_radius;
      }

      if (show_init) {
        stroke(255, 0, 0);
        ellipse(ini.x*resx, ini.y*resy, 0, 0);
        stroke(110);
        ellipse(ini.x*resx, ini.y*resy, radius, radius);
      } else {
        stroke(70);
        line(ini.x*resx, ini.y*resy, rel.x*resx, rel.y*resy);
        stroke(255, 0, 0);
        ellipse(ini.x*resx, ini.y*resy, 0, 0);
        if (all_connected) {
          stroke(128, 255, 128);
        } else {
          stroke(128, 128, 255);
        }
        ellipse(rel.x*resx, rel.y*resy, 0, 0);
        stroke(110);
        ellipse(rel.x*resx, rel.y*resy, radius, radius);
      }
      
    }
  }
  stroke(10);
  strokeWeight(2);
  fill(35);
  rect(20, height - 160, 240, 132);
  fill(180);
  textSize(16);
  int h = height - 140;
  int ls = 20;
  text("grid size (k/j): " + round(resx), 30, h); h += ls;
  text("noise scale (n/m): " + (round(ns*100)/100.0), 30, h); h += ls;
  text("neutral distance (l/h): " + (round(neutral_dist*100)/100.0), 30, h); h += ls;
  text("radius (f/d): " + fixed_radius, 30, h); h += ls;
  if (show_init) {
    text("showing initial points (s).", 30, h); h += ls;
  } else {
    text("showing relaxed points (s).", 30, h); h += ls;
  }
  if (all_connected) {
    text("completely connected (c).", 30, h); h += ls;
  } else {
    text("randomly connected (c).", 30, h); h += ls;
  }
}

void keyPressed() {
  if (key == 'k' && resx < maxres) {
    resx *= 1.5;
    resy *= 1.5;
  } else if (key == 'j' && resx > minres) {
    resx /= 1.5;
    resy /= 1.5;
  } else if (key == 'l' && neutral_dist < maxndist) {
    neutral_dist += 0.1;
  } else if (key == 'h' && neutral_dist > minndist) {
    neutral_dist -= 0.1;
  } else if (key == 'n' && ns > minns) {
    ns /= 1.5;
  } else if (key == 'm' && ns < maxns) {
    ns *= 1.5;
  } else if (key == 'f' && fixed_radius < maxr) {
    fixed_radius += 5;
  } else if (key == 'd' && fixed_radius > 0) {
    fixed_radius -= 5;
  } else if (key == 's') {
    show_init = !show_init;
  } else if (key == 'c') {
    all_connected = !all_connected;
  } else if (key == 'q') {
    exit();
  }
}
