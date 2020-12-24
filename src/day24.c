#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include "../lib/uthash.h"

#define LIFETIME 100

typedef enum hexdir {
  E, SE, SW,
  W, NW, NE,

  HEXDIR_COUNT
} HEXDIR;

int dxs[HEXDIR_COUNT] = { 2, 1, -1, -2, -1, 1, };
int dys[HEXDIR_COUNT] = { 0, -1, -1, 0, 1, 1,  };

typedef struct vec2d {
  int x, y;
} V2D;

typedef struct tile {
  V2D pos;
  int neighbours;

  UT_hash_handle hh;
} TILE;

void offset(V2D *pos, HEXDIR dir) {
  pos->x += dxs[dir];
  pos->y += dys[dir];
}

bool readdir(FILE *input, HEXDIR *dest) {
  switch (getc(input)) {
  case 'e': *dest = E; break;
  case 'w': *dest = W; break;
  case 's':
    switch (getc(input)) {
    case 'e': *dest = SE; break;
    case 'w': *dest = SW; break;
    default:
      fprintf(stderr, "Unrecognised character\n");
      exit(EXIT_FAILURE);
    }
    break;
  case 'n':
    switch (getc(input)) {
    case 'e': *dest = NE; break;
    case 'w': *dest = NW; break;
    default:
      fprintf(stderr, "Unrecognised character\n");
      exit(EXIT_FAILURE);
    }
    break;
  case '\n':
  case -1:
    return false;
  default:
    fprintf(stderr, "Unrecognised character\n");
    exit(EXIT_FAILURE);
  }
  return true;
}

void fliphextile(TILE **tiles, V2D *pos) {
  TILE *tile;
  HASH_FIND(hh, *tiles, pos, sizeof(*pos), tile);
  if (tile == NULL) {
    tile = malloc(sizeof(TILE));
    tile->pos = *pos;
    HASH_ADD(hh, *tiles, pos, sizeof(V2D), tile);
  } else {
    HASH_DEL(*tiles, tile);
    free(tile);
  }
}

void refhextile(TILE **tiles, V2D *pos) {
  TILE *tile;
  HASH_FIND(hh, *tiles, pos, sizeof(*pos), tile);
  if (tile == NULL) {
    tile = malloc(sizeof(TILE));
    tile->pos = *pos;
    tile->neighbours = 0;
    HASH_ADD(hh, *tiles, pos, sizeof(V2D), tile);
  }
  tile->neighbours++;
}

// it's cellular automata again
void advancehex(TILE **tiles) {
  TILE *oldtile, *tile, *tmp, *newtiles = NULL;
  V2D pos;
  for (tile = *tiles; tile != NULL; tile = tile->hh.next) {
    for (HEXDIR dir = 0; dir < HEXDIR_COUNT; ++dir) {
      pos = tile->pos;
      offset(&pos, dir);
      refhextile(&newtiles, &pos);
    }
  }

  HASH_ITER(hh, newtiles, tile, tmp) {
    if (tile->neighbours == 2) continue;
    if (tile->neighbours == 1) {
      HASH_FIND(hh, *tiles, &tile->pos, sizeof(V2D), oldtile);
      if (oldtile != NULL) continue;
    }
    HASH_DEL(newtiles, tile);
    free(tile);
  }

  HASH_ITER(hh, *tiles, tile, tmp) {
    HASH_DEL(*tiles, tile);
    free(tile);
  }

  *tiles = newtiles;
}

void day24() {
  FILE *input = fopen("input/24.txt", "r");

  TILE *tiles = NULL;

  V2D pos;
  HEXDIR dir;
  int c;
  while ((c = getc(input)) != '\n' && c != -1) {
    ungetc(c, input);
    pos.x = 0;
    pos.y = 0;
    while (readdir(input, &dir)) {
      offset(&pos, dir);
    }
    fliphextile(&tiles, &pos);
  }
  fclose(input);

  printf("Black: %d\n", HASH_COUNT(tiles));

  for (int i = 0; i < LIFETIME; ++i) advancehex(&tiles);
  printf("Black: %d\n", HASH_COUNT(tiles));

  TILE *tile, *tmp;
  HASH_ITER(hh, tiles, tile, tmp) {
    HASH_DEL(tiles, tile);
    free(tile);
  }
}
