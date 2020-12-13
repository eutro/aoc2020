#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

typedef __int128_t vlong;

// https://en.wikipedia.org/wiki/Extended_Euclidean_algorithm
vlong egcd(vlong x, vlong y, vlong *a, vlong *b) {
  vlong quot, tmp,
    or = x, r = y,
    os = 1, s = 0,
    ot = 0, t = 1;

  while (r != 0) {
    quot = or / r;
    tmp = r, r = or - quot * r, or = tmp;
    tmp = s, s = os - quot * s, os = tmp;
    tmp = t, t = ot - quot * t, ot = tmp;
  }

  *a = os, *b = ot;
  return or;
}

vlong mod(vlong a, vlong n) {
  a = a % n;
  return a < 0 ? a + n : a;
}

// https://en.wikipedia.org/wiki/Chinese_remainder_theorem
void crt(vlong a1, vlong n1,
         vlong a2, vlong n2,
         vlong *a, vlong *n) {
  vlong m1, m2;
  assert(egcd(n1, n2, &m1, &m2) == 1); // must be coprime
  *a = (a1 * m2 * n2) + (a2 * m1 * n1);
  *n = n1 * n2;
  *a = mod(*a, *n); // keep an the smallest above 0
}

void day13() {
  FILE *input = fopen("input/13.txt", "r");

  int minTs;
  assert(fscanf(input, "%d\n", &minTs) == 1);

  int c, id, wait, minBus;
  int minWait = INT_MAX;
  do {
    if (fscanf(input, "%d", &id) == 1) {
      wait = id - (minTs % id);
      if (wait < minWait) {
        minBus = id;
        minWait = wait;
      }
    }
    if ((c = getc(input)) == 'x') {
      c = getc(input);
    }
  } while (c != EOF);
  printf("Departure: %d (id) * %d (mins) = %d\n",
         minBus, minWait,
         minBus * minWait);

  fseek(input, 0, SEEK_SET);
  assert(fscanf(input, "%*d\n%d,", &id) == 1);

  vlong offset = 1;
  vlong a = 0;
  vlong n = id;
  do {
    if (fscanf(input, "%d", &id) == 1) {
      crt(a, n, -offset, id, &a, &n);
    }
    ++offset;
    if ((c = getc(input)) == 'x') {
      c = getc(input);
    }
  } while (c != EOF);
  fclose(input);

  printf("Timestamp: %ld (mod %ld)\n", (long) a, (long) n);
}
