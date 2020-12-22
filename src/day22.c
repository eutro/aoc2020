#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "../lib/uthash.h"

#define ARRAYDEQUE_SIZE 64

typedef struct arraydeque {
  int values[ARRAYDEQUE_SIZE];
  int min, max;
} ArrayDeque;

static int pollf(ArrayDeque *d) {
  assert(d->min != d->max);
  return d->values[d->min++];
}

static int pollb(ArrayDeque *d) {
  assert(d->min != d->max);
  return d->values[--d->max];
}

static int dsize(ArrayDeque *d) {
  return d->max - d->min;
}

void printdeck(ArrayDeque *d) {
  for (int i = d->min; i < d->max; ++i) {
    if (i != d->min) printf(", ");
    printf("%d", d->values[i]);
  }
  printf("\n");
}

static void pushb(ArrayDeque *d, int v) {
  if (d->max >= ARRAYDEQUE_SIZE) {
    if (d->min == 0) {
      fprintf(stderr, "Array Deque full!\n");
      exit(EXIT_FAILURE);
    }
    memmove(d->values, &d->values[d->min], sizeof(int) * dsize(d));
    d->max -= d->min;
    d->min = 0;
  }
  d->values[d->max++] = v;
}

static ArrayDeque *copyd(ArrayDeque *d) {
  ArrayDeque *r = malloc(sizeof(*d));
  memcpy(r->values, &d->values[d->min], sizeof(int) * dsize(d));
  r->min = 0;
  r->max = dsize(d);
  return r;
}

int combat(ArrayDeque *p1, ArrayDeque *p2) {
  while (dsize(p1) > 0 && dsize(p2) > 0) {
    int c1 = pollf(p1);
    int c2 = pollf(p2);
    if (c1 > c2) {
      pushb(p1, c1);
      pushb(p1, c2);
    } else if (c2 > c1) {
      pushb(p2, c2);
      pushb(p2, c1);
    } else {
      fprintf(stderr, "Tie, how?\n");
      exit(EXIT_FAILURE);
    }
  }
}

void fillstate(ArrayDeque *p1, ArrayDeque *p2, int *dest) {
  int i = 0;
  memcpy(&dest[i], &p1->values[p1->min], sizeof(int) * dsize(p1));
  i += dsize(p1);
  dest[i++] = -1;
  memcpy(&dest[i], &p2->values[p2->min], sizeof(int) * dsize(p2));
}

typedef struct stateset {
  // the two hands concatenated with a -1 between
  int *hands;
  UT_hash_handle hh;
} StateSet;

int recursivecombat(ArrayDeque *p1, ArrayDeque *p2) {
  // static int depth = 0;
  // printf("Depth: %d\n", ++depth);
  StateSet *state, *ostate, *states = NULL;
  int winner;
  while (true) {
    if (dsize(p1) == 0) {
      winner = 1;
      break;
    }
    if (dsize(p2) == 0) {
      winner = 0;
      break;
    }

    size_t keylen = sizeof(int) * (dsize(p1) + 1 + dsize(p2));
    int *key = malloc(keylen);
    fillstate(p1, p2, key);
    HASH_FIND(hh, states, key, keylen, ostate);
    if (ostate != NULL) {
      free(key);
      winner = 0;
      break;
    }
    state = malloc(sizeof(*state));
    state->hands = key;
    HASH_ADD_KEYPTR(hh, states, key, keylen, state);

    int c1 = pollf(p1);
    int c2 = pollf(p2);
    if (dsize(p1) >= c1 && dsize(p2) >= c2) {
      ArrayDeque *p1r = copyd(p1);
      ArrayDeque *p2r = copyd(p2);
      winner = recursivecombat(p1r, p2r);
      free(p1r);
      free(p2r);
      if (winner == 0) {
        pushb(p1, c1);
        pushb(p1, c2);
      } else {
        pushb(p2, c2);
        pushb(p2, c1);
      }
    } else if (c1 > c2) {
      pushb(p1, c1);
      pushb(p1, c2);
    } else if (c2 > c1) {
      pushb(p2, c2);
      pushb(p2, c1);
    } else {
      fprintf(stderr, "Tie, how?\n");
      exit(EXIT_FAILURE);
    }
  }

  HASH_ITER(hh, states, state, ostate) {
    HASH_DEL(states, state);
    free(state->hands);
    free(state);
  }

  // --depth;
  return winner;
}

ArrayDeque *readdeck(FILE *fp) {
  ArrayDeque *d = malloc(sizeof(ArrayDeque));
  d->min = d->max = 0;
  assert(fscanf(fp, "Player %*d:\n") == 0);
  int i;
  while (fscanf(fp, "%d\n", &i) == 1) pushb(d, i);
  return d;
}

// destructive
long countscore(ArrayDeque *d) {
  long score = 0;
  for (int i = 1; dsize(d) > 0; ++i) {
    score += pollb(d) * i;
  }
  return score;
}

void day22() {
  int winner;
  ArrayDeque *p1, *p2, *p1r, *p2r;
  FILE *input = fopen("input/22.txt", "r");
  p1 = readdeck(input);
  p2 = readdeck(input);
  fclose(input);
  p1r = copyd(p1);
  p2r = copyd(p2);

  combat(p1, p2);
  winner = dsize(p2) > 0;
  printf("Victory: %ld for Player %d\n", countscore(winner ? p2 : p1), winner + 1);
  free(p1);
  free(p2);

  recursivecombat(p1r, p2r);
  winner = dsize(p2r) > 0;
  printf("Victory: %ld for Player %d\n", countscore(winner ? p2r : p1r), winner + 1);
  free(p1r);
  free(p2r);
}
