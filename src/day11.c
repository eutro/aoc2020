#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct seats {
  char *arr;
  int rowLen;
  int rowCount;
} SEATS;

char refseat(SEATS *seats, int x, int y) {
  if (x < 0 || y < 0 ||
      x >= seats->rowLen ||
      y >= seats->rowCount) return '\0';
  return seats->arr[(y * seats->rowLen) + x];
}

void setseat(SEATS *seats, int x, int y, char v) {
  if (x < 0 || y < 0 ||
      x >= seats->rowLen ||
      y >= seats->rowCount) return;
  seats->arr[(y * seats->rowLen) + x] = v;
}

void printseats(SEATS *seats) {
  int x, y;
  for (y = 0; y < seats->rowLen; ++y) {
    for (x = 0; x < seats->rowLen; ++x) {
      printf("%c", refseat(seats, x, y));
    }
    printf("\n");
  }
  printf("\n");
}

int countadj(SEATS *seats, int x, int y, char t) {
  int count = 0;
  int dx, dy;
  for (dx = -1; dx <= 1; ++dx) {
    for (dy = -1; dy <= 1; ++dy) {
      if (dx == 0 && dy == 0) continue;
      if (refseat(seats, x + dx, y + dy) == t) ++count;
    }
  }
  return count;
}

int countdiag(SEATS *seats, int x, int y, char t) {
  int count = 0;
  int dx, dy, tx, ty;
  char c;
  for (dx = -1; dx <= 1; ++dx) {
    for (dy = -1; dy <= 1; ++dy) {
      if (dx == 0 && dy == 0) continue;
      tx = x;
      ty = y;
      do {
        tx += dx;
        ty += dy;
        if ((c = refseat(seats, tx, ty)) == t) {
          ++count;
          break;
        }
      } while (c == '.');
    }
  }
  return count;
}

int countseats(SEATS *seats) {
  int seatCount = 0;
  int x, y;
  for (y = 0; y < seats->rowLen; ++y) {
    for (x = 0; x < seats->rowLen; ++x) {
      if (refseat(seats, x, y) == '#') ++seatCount;
    }
  }
  return seatCount;
}

int advance(SEATS *selected, SEATS *other,
            int (*count)(SEATS*, int, int, char),
            int tolerance) {
  int x, y;
  int changed = 0;
  for (x = 0; x < selected->rowLen; ++x) {
    for (y = 0; y < selected->rowCount; ++y) {
      switch (refseat(selected, x, y)) {
      case 'L':
        if (count(selected, x, y, '#') == 0) {
          setseat(other, x, y, '#');
          changed = 1;
        } else {
          setseat(other, x, y, 'L');
        }
        break;
      case '#':
        if (count(selected, x, y, '#') >= tolerance) {
          setseat(other, x, y, 'L');
          changed = 1;
        } else {
          setseat(other, x, y, '#');
        }
        break;
      }
    }
  }
  return changed;
}

void day11() {
  FILE *input = fopen("input/11.txt", "r");
  int rowLen = 0;
  int rowCount = 1;
  int c;
  while ((c = getc(input)) != EOF) {
    if (c == '\n') break;
    ++rowLen;
  }
  while ((c = getc(input)) != EOF) {
    if (c == '\n') ++rowCount;
  }

  char seatBuffA[rowLen * rowCount];
  char seatBuffB[rowLen * rowCount];
  char seatBuffC[rowLen * rowCount];

  fseek(input, 0, SEEK_SET);
  int i = 0;
  while ((c = getc(input)) != EOF) {
    if (c == '\n') continue;
    seatBuffA[i++] = c;
  }
  fclose(input);

  memcpy(seatBuffB, seatBuffA, sizeof(seatBuffB));
  memcpy(seatBuffC, seatBuffA, sizeof(seatBuffC));

  SEATS seatsA = {
    .arr = seatBuffA,
    .rowLen = rowLen,
    .rowCount = rowCount
  };

  SEATS seatsB = {
    .arr = seatBuffB,
    .rowLen = rowLen,
    .rowCount = rowCount
  };

  SEATS *selected = &seatsA;
  SEATS *other = &seatsB;
  SEATS *tmp;
  while (advance(selected, other, countadj, 4)) {
    tmp = selected;
    selected = other;
    other = tmp;
  }

  printf("Occupied: %d\n", countseats(selected));

  memcpy(seatBuffA, seatBuffC, sizeof(seatBuffA));
  memcpy(seatBuffB, seatBuffC, sizeof(seatBuffB));

  while (advance(selected, other, countdiag, 5)) {
    tmp = selected;
    selected = other;
    other = tmp;
  }

  printf("Occupied: %d\n", countseats(selected));
}
