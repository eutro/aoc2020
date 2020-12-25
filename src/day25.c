#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "../lib/uthash.h"

#define MODULO 20201227
#define SUBJECT 7

unsigned long reverseloop(unsigned long n) {
  while (n % SUBJECT != 0) {
    n += MODULO;
  }
  return n / SUBJECT;
}

int countloops(unsigned long key) {
  int count = 0;
  while (key != 7) {
    key = reverseloop(key);
    ++count;
  }
  return count;
}

void day25() {
  FILE *input = fopen("input/25.txt", "r");
  char *buf = NULL;
  size_t bufsz;

  getline(&buf, &bufsz, input);
  int cardloops = countloops(atoi(buf));
  free(buf);
  buf = NULL;
  getline(&buf, &bufsz, input);

  int doorkey = atoi(buf);
  free(buf);
  buf = NULL;
  fclose(input);

  unsigned long ekey = doorkey;
  for (int i = 0; i < cardloops; ++i) {
    ekey *= doorkey;
    ekey %= MODULO;
  }
  printf("Encryption-key: %lu\n", ekey);
}
