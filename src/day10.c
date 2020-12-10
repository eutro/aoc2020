#include <stdio.h>
#include <string.h>

void day10() {
  FILE *input = fopen("input/10.txt", "r");

  int maxValue = -1;
  int n;

  while (fscanf(input, "%d", &n) == 1) {
    if (n > maxValue) {
      maxValue = n;
    }
  }

  int valueSet[maxValue + 1];
  memset(valueSet, 0, (maxValue + 1) * sizeof(int));

  fseek(input, 0, SEEK_SET);
  while (fscanf(input, "%d", &n) == 1) {
    if (valueSet[n]) {
      printf("Duplicate key? %d\n", n);
    }
    valueSet[n] = 1;
  }
  fclose(input);

  int j;
  int jumps[3] = { 0, 0, 1 };
  int lastJoltage = 0;
  for (j = 0; j <= maxValue; ++j) {
    if (valueSet[j]) {
      // printf("%d -> %d\n", lastJoltage, j);
      ++jumps[j - lastJoltage - 1];
      lastJoltage = j;
    }
  }
  printf("Jumps: %d * %d = %d\n",
         jumps[0], jumps[2],
         jumps[0] * jumps[2]);

  unsigned long fib[maxValue + 1];
  memset(&fib[1], 0, maxValue * sizeof(unsigned long));
  fib[0] = 1;
  if (valueSet[1]) fib[1] = fib[0];
  if (valueSet[2]) fib[2] = fib[0] + fib[1];
  for (j = 3; j <= maxValue; ++j) {
    if (valueSet[j]) {
      fib[j] = fib[j - 3] + fib[j - 2] + fib[j - 1];
    }
  }
  printf("Arrangements: %ld\n", fib[maxValue]);
}
