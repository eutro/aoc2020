#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include "../lib/uthash.h"

#define IMAGE_SIZE 10
#define MIDDLE_SIZE (IMAGE_SIZE - 2)
#define PUZZLE_SIZE 3

#define NBLBINPAT "%c%c%c%c"
#define NBLBINFMT(nibble)      \
  (nibble & 0x08 ? '#' : '.'), \
  (nibble & 0x04 ? '#' : '.'), \
  (nibble & 0x02 ? '#' : '.'), \
  (nibble & 0x01 ? '#' : '.')

#define TWOBINFMT(twobit)      \
  (twobit & 0x02 ? '#' : '.'), \
  (twobit & 0x01 ? '#' : '.')

#define TENBITPAT "%c%c" NBLBINPAT NBLBINPAT
#define TENBITFMT(short) TWOBINFMT(short >> 8), NBLBINFMT(short >> 4), NBLBINFMT(short)

typedef enum side {
  UP, RIGHT, DOWN, LEFT,

  SIDE_COUNT
} Side;

// 64 bits
typedef unsigned long Middle;

typedef struct image {
  short sides[SIDE_COUNT];
  Middle middle;
} Image;

typedef struct tile {
  Image im;
  int id;

  UT_hash_handle hh;
} Tile;

static short REVTEN[1 << IMAGE_SIZE] = {};
static bool REVTEN_INIT = false;

short revten(short n) {
  if (!REVTEN_INIT) {
    for (int i = 0; i < IMAGE_SIZE; ++i) {
      for (int j = 0; j < (1 << IMAGE_SIZE); ++j) {
        REVTEN[j] |= ((j & (1 << i)) != 0) << (IMAGE_SIZE - i - 1);
      }
    }
    REVTEN_INIT = true;
  }
  return REVTEN[n];
}

void readimage(FILE *fp, Image *to) {
  *to = (Image) { { 0, 0, 0, 0 }, 0 };
  int i, j;
  to->sides[UP] |= to->sides[LEFT] = getc(fp) == '#';
  for (i = 1; i < IMAGE_SIZE - 1; ++i) {
    to->sides[UP] <<= 1;
    to->sides[UP] |= getc(fp) == '#';
  }
  to->sides[UP] <<= 1;
  to->sides[UP] |= to->sides[RIGHT] = getc(fp) == '#';
  assert(getc(fp) == '\n');

  for (i = 1; i < IMAGE_SIZE - 1; ++i) {
    to->sides[LEFT] <<= 1;
    to->sides[LEFT] |= getc(fp) == '#';
    for (j = 1; j < IMAGE_SIZE - 1; ++j) {
      to->middle <<= 1;
      to->middle |= getc(fp) == '#';
    }
    to->sides[RIGHT] <<= 1;
    to->sides[RIGHT] |= getc(fp) == '#';
    assert(getc(fp) == '\n');
  }

  to->sides[LEFT] <<= 1;
  to->sides[LEFT] |= to->sides[DOWN] = getc(fp) == '#';
  for (i = 1; i < IMAGE_SIZE - 1; ++i) {
    to->sides[DOWN] <<= 1;
    to->sides[DOWN] |= getc(fp) == '#';
  }
  i = getc(fp) == '#';
  to->sides[DOWN] <<= 1;
  to->sides[DOWN] |= i;
  to->sides[RIGHT] <<= 1;
  to->sides[RIGHT] |= i;
  assert(getc(fp) == '\n');

  to->sides[LEFT] = revten(to->sides[LEFT]);
  to->sides[DOWN] = revten(to->sides[DOWN]);
}

void printmiddlerow(unsigned char row) {
  printf(NBLBINPAT""NBLBINPAT, NBLBINFMT(row >> 4), NBLBINFMT(row));
}

void printmiddle(Middle m) {
  for (int r = 7; r >= 0; --r) {
    printmiddlerow((unsigned char) (m >> (r * 8)));
    printf("\n");
  }
}

void printtile(Image *tile) {
  printf("U: "TENBITPAT" L: "TENBITPAT" D: "TENBITPAT" R: "TENBITPAT"\n",
         TENBITFMT(tile->sides[UP]),
         TENBITFMT(tile->sides[LEFT]),
         TENBITFMT(tile->sides[DOWN]),
         TENBITFMT(tile->sides[RIGHT]));
  printmiddle(tile->middle);
}

Middle flipvert(Middle x) {
  return
    ((x << 56)) |
    ((x << 40) & 0x00ff000000000000) |
    ((x << 24) & 0x0000ff0000000000) |
    ((x <<  8) & 0x000000ff00000000) |
    ((x >>  8) & 0x00000000ff000000) |
    ((x >> 24) & 0x0000000000ff0000) |
    ((x >> 40) & 0x000000000000ff00) |
    ((x >> 56));
}

Middle flipdiag(Middle x) {
  const Middle k1 = 0x5500550055005500;
  const Middle k2 = 0x3333000033330000;
  const Middle k4 = 0x0f0f0f0f00000000;
  Middle t;
  t  = k4 & (x ^ (x << 28));
  x ^=       t ^ (t >> 28);
  t  = k2 & (x ^ (x << 14));
  x ^=       t ^ (t >> 14);
  t  = k1 & (x ^ (x <<  7));
  x ^=       t ^ (t >>  7);
  return x;
}

// cw
Middle rotate1(Middle m) {
  return flipdiag(flipvert(m));
}

Middle rotatemiddle(Middle m, Side rotation) {
  switch (rotation) {
  case LEFT: m = rotate1(m);
  case DOWN: m = rotate1(m);
  case RIGHT: m = rotate1(m);
  }
  return m;
}

Middle flipmiddle(Middle m) {
  m =
    (((m & 0x8080808080808080) >> 7) |
     ((m & 0x4040404040404040) >> 5) |
     ((m & 0x2020202020202020) >> 3) |
     ((m & 0x1010101010101010) >> 1) |
     ((m & 0x0808080808080808) << 1) |
     ((m & 0x0404040404040404) << 3) |
     ((m & 0x0202020202020202) << 5) |
     ((m & 0x0101010101010101) << 7));
  return m;
  //  (((m & 0xFF00000000000000) >> (7 * 8)) |
  //   ((m & 0x00FF000000000000) >> (5 * 8)) |
  //   ((m & 0x0000FF0000000000) >> (3 * 8)) |
  //   ((m & 0x000000FF00000000) >> (1 * 8)) |
  //   ((m & 0x00000000FF000000) << (1 * 8)) |
  //   ((m & 0x0000000000FF0000) << (3 * 8)) |
  //   ((m & 0x000000000000FF00) << (5 * 8)) |
  //   ((m & 0x00000000000000FF) << (7 * 8)));
}

void fliptile(Tile *tile) {
  // printf("Flipping: %d\n", tile->id);
  // printtile(&tile->im);
  for (Side s = 0; s < SIDE_COUNT; ++s) {
    tile->im.sides[s] = revten(tile->im.sides[s]);
  }
  short tmp = tile->im.sides[LEFT];
  tile->im.sides[LEFT] = tile->im.sides[RIGHT];
  tile->im.sides[RIGHT] = tmp;
  tile->im.middle = flipmiddle(tile->im.middle);
  // printf("---\n");
  // printtile(&tile->im);
}

void rotatetile(Tile *tile, Side count) {
  tile->im.middle = rotatemiddle(tile->im.middle, count);
  short tmp, last;
  for (Side i = 0; i < count; ++i) {
    last = tile->im.sides[0];
    for (Side s = 1; s <= SIDE_COUNT; ++s) {
      tmp = tile->im.sides[s % SIDE_COUNT];
      tile->im.sides[s % SIDE_COUNT] = last;
      last = tmp;
    }
  }
}

static int dxs[SIDE_COUNT] = { 0, 1, 0, -1 };
static int dys[SIDE_COUNT] = { -1, 0, 1, 0 };
static Side opposites[SIDE_COUNT] = { DOWN, LEFT, UP, RIGHT };
static char *sidenames[SIDE_COUNT] = { "UP", "RIGHT", "DOWN", "LEFT" };

typedef struct tilepair {
  Tile *lhs, *rhs;
} TilePair;

bool checkfit(Tile *solved[PUZZLE_SIZE][PUZZLE_SIZE],
              int x, int y) {
  Tile *other, *this = solved[x][y];
  int nx, ny;
  for (Side side = 0; side < SIDE_COUNT; ++side) {
    nx = x + dxs[side]; if (nx < 0 || nx >= PUZZLE_SIZE) continue;
    ny = y + dys[side]; if (ny < 0 || ny >= PUZZLE_SIZE) continue;
    other = solved[nx][ny];
    if (other == NULL) continue;
    if (other->im.sides[opposites[side]] != revten(this->im.sides[side])) return false;
  }
  return true;
}

void printsolved(Tile *solved[PUZZLE_SIZE][PUZZLE_SIZE]) {
  for (int y = 0; y < PUZZLE_SIZE; ++y) {
    for (int x = 0; x < PUZZLE_SIZE; ++x) {
      printf("%4d ", solved[x][y] == NULL ? -1 : solved[x][y]->id);
    }
    printf("\n");
  }
}

void printpuzzle(Tile *solved[PUZZLE_SIZE][PUZZLE_SIZE]) {
  for (int y = 0; y < PUZZLE_SIZE; ++y) {
    for (int r = 7; r >= 0; --r) {
      for (int x = 0; x < PUZZLE_SIZE; ++x) {
        printmiddlerow((unsigned char) (solved[x][y]->im.middle >> (r * 8)));
      }
      printf("\n");
    }
  }
}

void trypermute(Tile *solved[PUZZLE_SIZE][PUZZLE_SIZE],
                int x, int y) {
  Tile *this = solved[x][y];
  for (int i = 0; i < 2; ++i) {
    if (checkfit(solved, x, y)) return;
    for (Side rot = 0; rot < SIDE_COUNT; ++rot) {
      rotatetile(this, 1);
      if (checkfit(solved, x, y)) return;
    }
    fliptile(this);
  }
  fprintf(stderr, "Couldn't find permutation that fits (%d, %d)\n", x, y);
  printsolved(solved);
  exit(EXIT_FAILURE);
}

void solvefrom(Tile *solved[PUZZLE_SIZE][PUZZLE_SIZE],
               TilePair lookup[1 << IMAGE_SIZE],
               int x, int y) {
  int nx, ny;
  Tile *neighbour, *this = solved[x][y];
  for (Side side = 0; side < SIDE_COUNT; ++side) {
    nx = x + dxs[side]; if (nx < 0 || nx >= PUZZLE_SIZE) continue;
    ny = y + dys[side]; if (ny < 0 || ny >= PUZZLE_SIZE) continue;
    if (solved[nx][ny] != NULL) continue;

    TilePair pair = lookup[this->im.sides[side]];
    if (pair.lhs == NULL) continue;
    neighbour = pair.lhs == this ? pair.rhs : pair.lhs;
    if (neighbour == NULL) continue;
    solved[nx][ny] = neighbour;
    trypermute(solved, nx, ny);
    solvefrom(solved, lookup, nx, ny);
  }
}

void day20() {
  FILE *input = fopen("input/20.txt", "r");
  int i;
  Side s;
  Tile *tmp, *tile, *tiles = NULL;

  int side_counts[1 << IMAGE_SIZE] = {};
  TilePair lookup[1 << IMAGE_SIZE] = {};

  while (fscanf(input, "Tile %d:\n", &i) == 1) {
    tile = malloc(sizeof(Tile));
    tile->id = i;
    readimage(input, &tile->im);
    getc(input);
    HASH_ADD_INT(tiles, id, tile);

    for (s = 0; s < SIDE_COUNT; ++s) {
      side_counts[tile->im.sides[s]]++;
      side_counts[revten(tile->im.sides[s])]++;

      if (lookup[tile->im.sides[s]].lhs == NULL) {
        lookup[tile->im.sides[s]].lhs = tile;
      } else {
        lookup[tile->im.sides[s]].rhs = tile;
      }

      if (lookup[revten(tile->im.sides[s])].lhs == NULL) {
        lookup[revten(tile->im.sides[s])].lhs = tile;
      } else {
        lookup[revten(tile->im.sides[s])].rhs = tile;
      }
    }
  }
  fclose(input);

  Tile *corners[4] = {};
  Tile *edges[(PUZZLE_SIZE - 2) * 4] = {};

  long cornerp = 1;
  int cornercount = 0;
  int edgecount = 0;
  int edgesides;
  HASH_ITER(hh, tiles, tile, tmp) {
    edgesides = 0;
    for (s = 0; s < SIDE_COUNT; ++s) {
      edgesides += side_counts[tile->im.sides[s]] == 1;
      assert(side_counts[tile->im.sides[s]] < 3);
    }
    if (edgesides == 0) continue;

    if (edgesides == 2) {
      corners[cornercount++] = tile;
      cornerp *= tile->id;
    } else if (edgesides == 1) {
      edges[edgecount++] = tile;
    } else {
      fprintf(stderr, "Too edgy: %d\n", edgesides);
      exit(EXIT_FAILURE);
    }
  }
  assert(cornercount == 4);
  printf("Corner-product: %ld\n", cornerp);

  Tile *solved[PUZZLE_SIZE][PUZZLE_SIZE] = {};
  solved[0][0] = corners[0]; // put a random corner at 0, 0
  while (side_counts[corners[0]->im.sides[UP]] != 1 ||
         side_counts[corners[0]->im.sides[LEFT]] != 1) {
    rotatetile(corners[0], 1);
  }

  solvefrom(solved, lookup, 0, 0);

  // TODO count sea monsters and stuff
}
