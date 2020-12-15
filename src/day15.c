#include <stdio.h>
#include <assert.h>
#include "../lib/uthash.h"

#define MAX_TIME_1 2020
#define MAX_TIME_2 30000000

typedef struct mentioned {
  int number;
  int time;
  UT_hash_handle hh;
} Mentioned;

int mention(Mentioned **mentioned, int time, int number) {
  Mentioned *m;
  HASH_FIND_INT(*mentioned, &number, m);
  if (m == NULL) {
    m = malloc(sizeof(*m));
    m->time = time;
    m->number = number;
    HASH_ADD_INT(*mentioned, number, m);
    return 0;
  }
  int ret = time - m->time;
  m->time = time;
  return ret;
}

void day15() {
  FILE *input = fopen("input/15.txt", "r");

  Mentioned *mentioned = NULL;
  int number, time = 1;
  while (fscanf(input, "%d,", &number) == 1) {
    number = mention(&mentioned, time++, number);
  }
  fclose(input);

  while (time < MAX_TIME_1) {
    number = mention(&mentioned, time++, number);
  }
  printf("2020th: %d\n", number);

  while (time < MAX_TIME_2) {
    number = mention(&mentioned, time++, number);
  }
  printf("30000000th: %d\n", number);

  Mentioned *m, *tmp;
  HASH_ITER(hh, mentioned, m, tmp) {
    HASH_DEL(mentioned, m);
    free(m);
  }
}
