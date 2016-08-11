// vim: syn=java

int THE_GRID[][];

int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 800;

int GRID_SIZE = 100;
int BLOB_SIZE = 1200;
int BLOB_COUNT = 3;
//String MODE = "overwrite";
String MODE = "exclude";

color COLORS[] = {
  color(0, 0, 0.2),
  color(0.03, 0.9, 0.8),
  color(0.24, 0.9, 0.8),
  color(0.55, 0.9, 0.8),
};

class Pos {
  int x;
  int y;

  Pos(int x, int y) {
    this.x = x % GRID_SIZE;
    this.y = y % GRID_SIZE;
    while (this.x < 0) { this.x += GRID_SIZE; }
    while (this.y < 0) { this.y += GRID_SIZE; }
  }

  @Override
  int hashCode() {
    return x + GRID_SIZE * y;
  }

  @Override
  boolean equals(Object obj) {
    if (!(obj instanceof Pos)) {
      return false;
    }
    Pos rhs = (Pos) obj;
    if (rhs.x == this.x && rhs.y == this.y) {
      return true;
    }
    return false;
  }
}

void blobfill(int grid[][], int x, int y, int size, int hue) {
  int total = 0;
  int i, dx, dy;
  Pos here, neighbor;
  ArrayList<Pos> q = new ArrayList<Pos>();
  ArrayList<Pos> shuffled = null;
  IntList indices = null;
  HashMap<Pos,Boolean> done = new HashMap<Pos,Boolean>();
  here = new Pos(x, y);
  q.add(here);
  done.put(here, true);
  while (q.size() > 0) {
    here = q.remove(0);
    //println("Processing: (" + here.x + ", " + here.y + ")");
    if (total < size) {
      for (dx = -1; dx <= 1; ++dx) {
        for (dy = -1; dy <= 1; ++dy) {
          if (abs(dx) + abs(dy) == 2) {
            continue;
          }
          neighbor = new Pos(here.x + dx, here.y + dy);
          if (MODE == "exclude" && grid[neighbor.x][neighbor.y] != 0) {
            continue;
          }
          //println("Considering: (" + neighbor.x + ", " + neighbor.y + ")");
          if (done.get(neighbor) == null) {
            //println("Adding: (" + neighbor.x + ", " + neighbor.y + ")");
            done.put(neighbor, true);
            q.add(neighbor);
          }
        }
      }
    }
    grid[here.x][here.y] = hue;
    total += 1;
    if (total % 5 == 0) {
      indices = new IntList();
      for (i = 0; i < q.size(); ++i) {
        indices.append(i);
      }
      indices.shuffle();
      shuffled = new ArrayList<Pos>();
      for (i = 0; i < q.size(); ++i) {
        shuffled.add(q.get(indices.get(i)));
      }
      q = shuffled;
    }
  }
}

void put_blobs() {
  int i, j, x, y;
  THE_GRID = new int[GRID_SIZE][GRID_SIZE];
  for (i = 0; i < GRID_SIZE; ++i) {
    for (j = 0; j < GRID_SIZE; ++j) {
      THE_GRID[i][j] = 0;
    }
  }

  for (i = 0; i < BLOB_COUNT; ++i) {
    x = (int) random(0, GRID_SIZE-1);
    y = (int) random(0, GRID_SIZE-1);
    while (THE_GRID[x][y] != 0) {
      x = (int) random(0, GRID_SIZE-1);
      y = (int) random(0, GRID_SIZE-1);
    }
    blobfill(THE_GRID, x, y, BLOB_SIZE, i+1);
  }
}

void new_colors(int count) {
  int i;
  color base;
  COLORS = new color[count+1];
  base = color(random(1.0), random(0.7, 0.9), random(0.4, 0.8));
  COLORS[0] = color(0, 0, 0.2);
  for (i = 1; i < count+1; ++i) {
    COLORS[i] = color(
      hue(base) + random(-0.05, 0.05),
      saturation(base) + random(-0.1, 0.1),
      brightness(base) + random(-0.15, 0.15)
    );
  }
}

void setup() {
  size(800, 800);
  WINDOW_WIDTH = 800;
  WINDOW_HEIGHT = 800;

  randomSeed(17);
  noiseSeed(17);

  colorMode(HSB, 1.0, 1.0, 1.0);

  BLOB_COUNT = (int) (random(4, 7));
  new_colors(BLOB_COUNT);
  put_blobs();

  noLoop();
  redraw();
}

void draw() {
  int i, j;
  float x, y, gw, gh, d;
  int quantum = (int) ((0.9 * WINDOW_WIDTH) / GRID_SIZE);
  int left = WINDOW_WIDTH/2 - (quantum * GRID_SIZE/2);
  int top = WINDOW_HEIGHT/2 - (quantum * GRID_SIZE/2);
  background(0, 0, 0);
  stroke(0, 0, 0);
  for (i = 0; i < GRID_SIZE; ++i) {
    for (j = 0; j < GRID_SIZE; ++j) {
      fill(COLORS[THE_GRID[i][j]]);
      rect(
        left + i*quantum,
        top + j*quantum,
        quantum,
        quantum
      );
    }
  }
}

void keyPressed() {
  if (key == 'q') {
    exit();
  } else if (key == 'r') {
    BLOB_COUNT = (int) (random(3, 7));
    new_colors(BLOB_COUNT);
    put_blobs();
    redraw();
  }
}
