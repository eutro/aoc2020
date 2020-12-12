#include <stdio.h>
#include <stdlib.h>

typedef enum dir {
  NORTH,
  EAST,
  SOUTH,
  WEST,

  NESW_COUNT,

  LEFT,
  FORWARD,
  RIGHT
} DIR;

DIR get_dir(char c) {
  switch (c) {
  case 'N': return NORTH;
  case 'E': return EAST;
  case 'S': return SOUTH;
  case 'W': return WEST;
  case 'L': return LEFT;
  case 'R': return RIGHT;
  case 'F': return FORWARD;
  }
}

static int dxs[] = { 0, 1, 0, -1 };
static int dys[] = { 1, 0, -1, 0 };

void day12() {
  FILE *input = fopen("input/12.txt", "r");

  int x = 0;
  int y = 0;
  DIR facing = EAST;

  char c;
  int d;
  DIR dir;
  while (fscanf(input, "%c%d\n", &c, &d) == 2) {
    dir = get_dir(c);
    if (dir < NESW_COUNT) {
      x += dxs[dir] * d;
      y += dys[dir] * d;
    } else if (dir == FORWARD) {
      x += dxs[facing] * d;
      y += dys[facing] * d;
    } else {
      facing = (facing + (dir - FORWARD) * (d / 90)) % NESW_COUNT;
    }
  }
  printf("Distance: %d\n", abs(x) + abs(y));

  x = 0;
  y = 0;
  int wx = 10;
  int wy = 1;

  fseek(input, 0, SEEK_SET);
  while (fscanf(input, "%c%d\n", &c, &d) == 2) {
    dir = get_dir(c);
    if (dir < NESW_COUNT) {
      wx += dxs[dir] * d;
      wy += dys[dir] * d;
    } else if (dir == FORWARD) {
      x += wx * d;
      y += wy * d;
    } else {
      for (int i = 0; i < d; i += 90) {
        int tx = wx;
        wx = (dir - FORWARD) * wy;
        wy = (FORWARD - dir) * tx;
      }
    }
  }
  fclose(input);

  printf("Distance: %d\n", abs(x) + abs(y));
}
