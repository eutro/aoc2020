#include <stdio.h>

#define FULL_MASK (1 << 27) - 1

int count_set_bits(int n) {
    int count = 0;
    while (n) {
        count += n & 1;
        n >>= 1;
    }
    return count;
}

void day6() {
  FILE *input = fopen("input/6.txt", "r");
  int counter = 0;
  int allCounter = 0;
  int mask = 0;
  int allMask = FULL_MASK;
  int privateMask = 0;
  int i;
  char line[32];

  while (fgets(line, sizeof(line), input)) {
    if (line[0] == '\n') {
      counter += count_set_bits(mask);
      allCounter += count_set_bits(allMask);
      mask = 0;
      allMask = FULL_MASK;
    } else {
      for (i = 0; line[i] != '\n'; ++i) {
        privateMask |= 1 << (line[i] - 'a');
      }
      allMask &= privateMask;
      mask |= privateMask;
      privateMask = 0;
    }
  }
  counter += count_set_bits(mask);
  allCounter += count_set_bits(allMask);

  fclose(input);
  printf("Questions: %d\n", counter);
  printf("All: %d\n", allCounter);
}
