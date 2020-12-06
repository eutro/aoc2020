#include <stdio.h>

#define PASS_LEN 10

void day5() {
  FILE *input = fopen("input/5.txt", "r");
  char pass[PASS_LEN + 1];
  int maxId = -1;
  int minId = 1 << PASS_LEN;
  int id, i;
  int passes[1 << PASS_LEN] = {};

  while (fscanf(input, "%s", &pass[0]) == 1) {
    id = 0;
    for (i = 0; i < PASS_LEN; ++i) {
      id = id << 1;
      if (pass[i] == 'B' || pass[i] == 'R') {
        ++id;
      }
    }
    passes[id] = 1;
    if (id > maxId) {
      maxId = id;
    }
    if (id < minId) {
      minId = id;
    }
  }
  
  fclose(input);
  printf("Max: %d\n", maxId);

  for (i = minId; i < maxId; ++i) {
    if (passes[i - 1] && !passes[i] && passes[i + 1]) {
      printf("Yours: %d\n", i);
      return;
    }
  }
  printf("You don't have a seat :(\n");
}
