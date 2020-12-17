#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "../lib/uthash.h"

typedef struct vec4d {
  int x, y, z, w;
} V4D;

typedef struct cell {
  V4D pos;
  int val;

  UT_hash_handle hh;
} CELL;

typedef struct cube {
  CELL *positions;
} CUBE;

void incval(CUBE *cube, V4D *pos) {
  CELL *cell;
  HASH_FIND(hh, cube->positions, pos, sizeof(V4D), cell);
  if (cell == NULL) {
    cell = malloc(sizeof(*cell));
    cell->pos = *pos;
    cell->val = 1;
    HASH_ADD(hh, cube->positions, pos, sizeof(V4D), cell);
    return;
  }
  ++cell->val;
}

void readcube(FILE *src, CUBE *cube) {
  int c;
  V4D pos = { 0, 0, 0, 0 };
  while ((c = getc(src)) != -1) {
    switch (c) {
    case '.': ++pos.x; break;
    case '#': incval(cube, &pos); ++pos.x; break;
    case '\n': pos.x = 0; ++pos.y; break;
    }
  }
}

void freepositions(CUBE *cube) {
  CELL *cell, *tmp;
  HASH_ITER(hh, cube->positions, cell, tmp) {
    HASH_DEL(cube->positions, cell);
    free(cell);
  }
}

void advancecube(CUBE *cube, bool incw) {
  CUBE newcube = { NULL };
  CELL *cell;
  V4D pos;
  int dx, dy, dz, dw;
  for (cell = cube->positions; cell != NULL; cell = cell->hh.next) {
    for (dx = -1; dx <= 1; ++dx) {
      for (dy = -1; dy <= 1; ++dy) {
        for (dz = -1; dz <= 1; ++dz) {
          if (incw) {
            for (dw = -1; dw <= 1; ++dw) {
              if (dz == 0 && dy == 0 && dx == 0 && dw == 0) continue;
              pos = (V4D) {
                cell->pos.x + dx,
                cell->pos.y + dy,
                cell->pos.z + dz,
                cell->pos.w + dw,
              };
              incval(&newcube, &pos);
            }
          } else {
            if (dz == 0 && dy == 0 && dx == 0) continue;
            pos = (V4D) {
              cell->pos.x + dx,
              cell->pos.y + dy,
              cell->pos.z + dz,
              0,
            };
            incval(&newcube, &pos);
          }
        }
      }
    }
  }

  CELL *tmp;
  HASH_ITER(hh, newcube.positions, cell, tmp) {
    if (cell->val == 3) continue;
    else if (cell->val == 2) {
      CELL *old;
      HASH_FIND(hh, cube->positions, &cell->pos, sizeof(V4D), old);
      if (old != NULL) continue;
    }
    HASH_DEL(newcube.positions, cell);
    free(cell);
  }

  freepositions(cube);
  cube->positions = newcube.positions;
}

int countactive(CUBE *cube) {
  int count = 0;
  CELL *cell;
  for (cell = cube->positions; cell != NULL; cell = cell->hh.next) {
    ++count;
  }
  return count;
}

void day17() {
  FILE *input = fopen("input/17.txt", "r");
  CUBE cube = { NULL };
  readcube(input, &cube);

  for (int i = 0; i < 6; ++i) advancecube(&cube, false);
  printf("Active: %d\n", countactive(&cube));

  freepositions(&cube);
  assert(cube.positions == NULL);

  fseek(input, 0, SEEK_SET);
  readcube(input, &cube);
  fclose(input);

  for (int i = 0; i < 6; ++i) advancecube(&cube, true);
  printf("Active: %d\n", countactive(&cube));

  freepositions(&cube);
}
