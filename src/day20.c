#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define IMAGE_SIZE 10
#define MIDDLE_SIZE (IMAGE_SIZE - 2)
#define PUZZLE_SIZE 12
#define PICTURE_SIZE (MIDDLE_SIZE * PUZZLE_SIZE)

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
typedef unsigned long long Middle;

typedef struct image {
  short sides[SIDE_COUNT];
  Middle middle;
} Image;

typedef struct tile {
  Image im;
  int id;
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

void printtile(Tile *tile) {
  printf("U: "TENBITPAT" R: "TENBITPAT" D: "TENBITPAT" L: "TENBITPAT"\n",
         TENBITFMT(tile->im.sides[UP]),
         TENBITFMT(tile->im.sides[RIGHT]),
         TENBITFMT(tile->im.sides[DOWN]),
         TENBITFMT(tile->im.sides[LEFT]));
  printmiddle(tile->im.middle);
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
  static const Middle k1 = 0x5500550055005500;
  static const Middle k2 = 0x3333000033330000;
  static const Middle k4 = 0x0f0f0f0f00000000;
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

// flips horizontally
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
}

void fliptile(Tile *tile) {
  // printf("Flipping: %d\n", tile->id);
  // printtile(tile);
  for (Side s = 0; s < SIDE_COUNT; ++s) {
    tile->im.sides[s] = revten(tile->im.sides[s]);
  }
  short tmp = tile->im.sides[LEFT];
  tile->im.sides[LEFT] = tile->im.sides[RIGHT];
  tile->im.sides[RIGHT] = tmp;
  tile->im.middle = flipmiddle(tile->im.middle);
  // printf("---\n");
  // printtile(tile);
}

void rotatetile(Tile *tile, Side count) {
  // printf("Rotating: %d\n", tile->id);
  // printtile(tile);
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
  // printf("---\n");
  // printtile(tile);
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

void printpicture(bool picture[PICTURE_SIZE][PICTURE_SIZE]) {
  for (int y = 0; y < PICTURE_SIZE; ++y) {
    for (int x = 0; x < PICTURE_SIZE; ++x) {
      printf("%c", picture[x][y] ? '#' : '.');
    }
    printf("\n");
  }
}

void flippicture(bool picture[PICTURE_SIZE][PICTURE_SIZE]) {
  bool tmp[PICTURE_SIZE];
  int tx;
  for (int x = 0; x < PICTURE_SIZE / 2; ++x) {
    tx = PICTURE_SIZE - x - 1;
    memcpy(tmp,         picture[x],  PICTURE_SIZE * sizeof(bool));
    memcpy(picture[x],  picture[tx], PICTURE_SIZE * sizeof(bool));
    memcpy(picture[tx], tmp,         PICTURE_SIZE * sizeof(bool));
  }
}

// cw
void rotatepicture(bool picture[PICTURE_SIZE][PICTURE_SIZE]) {
  bool copied[PICTURE_SIZE][PICTURE_SIZE];
  for (int x = 0; x < PICTURE_SIZE; ++x) {
    for (int y = 0; y < PICTURE_SIZE; ++y) {
      copied[y][PICTURE_SIZE - x - 1] = picture[x][y];
    }
  }
  memcpy(picture, copied, PICTURE_SIZE * PICTURE_SIZE * sizeof(bool));
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

void middlecpy(bool picture[PICTURE_SIZE][PICTURE_SIZE],
               Middle m, int x, int y) {
  for (int bit = 0; bit < (MIDDLE_SIZE * MIDDLE_SIZE); ++bit) {
    picture[x * MIDDLE_SIZE + (bit % MIDDLE_SIZE)][y * MIDDLE_SIZE + (bit / MIDDLE_SIZE)]
      = (m & (1L << ((MIDDLE_SIZE * MIDDLE_SIZE) - bit - 1))) != 0;
  }
}

#define MONSTER_HEIGHT 3
#define MONSTER_WIDTH 20

bool checkmonsterat(bool picture[PICTURE_SIZE][PICTURE_SIZE],
                    int x, int y) {
  if (x + MONSTER_WIDTH > PICTURE_SIZE) return false;
  if (x + MONSTER_HEIGHT > PICTURE_SIZE) return false;
  static char *monster[] = {
    "                  # ",
    "#    ##    ##    ###",
    " #  #  #  #  #  #   ",
  };
  for (int mx = 0; mx < MONSTER_WIDTH; ++mx) {
    for (int my = 0; my < MONSTER_HEIGHT; ++my) {
      if (monster[my][mx] == '#') {
        if (!picture[x + mx][y + my]) return false;
      }
    }
  }

  for (int mx = 0; mx < MONSTER_WIDTH; ++mx) {
    for (int my = 0; my < MONSTER_HEIGHT; ++my) {
      if (monster[my][mx] == '#') {
        picture[x + mx][y + my] = false;
      }
    }
  }
  return true;
}

int checkmonster(bool picture[PICTURE_SIZE][PICTURE_SIZE]) {
  bool copied[PICTURE_SIZE][PICTURE_SIZE] = {};
  memcpy(copied, picture, PICTURE_SIZE * PICTURE_SIZE * sizeof(bool));

  bool found = false;
  int count = 0;
  for (int x = 0; x < PICTURE_SIZE; ++x) {
    for (int y = 0; y < PICTURE_SIZE; ++y) {
      found |= checkmonsterat(copied, x, y);
      if (copied[x][y]) ++count;
    }
  }
  return found ? count : -1;
}

void day20() {
  FILE *input = fopen("input/20.txt", "r");
  int i;
  Side s;
  Tile *tile;
  Tile tiles[PUZZLE_SIZE * PUZZLE_SIZE];
  int tilecounter = 0;

  int side_counts[1 << IMAGE_SIZE] = {};
  TilePair lookup[1 << IMAGE_SIZE] = {};

  while (fscanf(input, "Tile %d:\n", &i) == 1) {
    tile = &tiles[tilecounter++];
    tile->id = i;
    readimage(input, &tile->im);
    getc(input);

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

  long cornerp = 1;
  int cornercount = 0;
  int edgecount = 0;
  int edgesides;
  for (tilecounter = 0; tilecounter < PUZZLE_SIZE * PUZZLE_SIZE; ++tilecounter) {
    tile = &tiles[tilecounter];
    edgesides = 0;
    for (s = 0; s < SIDE_COUNT; ++s) {
      edgesides += side_counts[tile->im.sides[s]] == 1;
      assert(side_counts[tile->im.sides[s]] < 3);
    }
    if (edgesides == 0) continue;

    if (edgesides == 2) {
      corners[cornercount++] = tile;
      cornerp *= tile->id;
    } else if (edgesides != 1) {
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

  bool picture[PICTURE_SIZE][PICTURE_SIZE] = {};
  for (int y = 0; y < PUZZLE_SIZE; ++y) {
    for (int x = 0; x < PUZZLE_SIZE; ++x) {
      middlecpy(picture, solved[x][y]->im.middle, x, y);
    }
  }

  int nonmonster;
  for (int i = 0; i < 2; ++i) {
    if ((nonmonster = checkmonster(picture)) != -1) break;
    for (int r = 0; r < SIDE_COUNT; ++r) {
      rotatepicture(picture);
      if ((nonmonster = checkmonster(picture)) != -1) goto finishp2;
    }
    flippicture(picture);
  }
 finishp2:
  printf("Roughness: %d pixels\n", nonmonster);
}
