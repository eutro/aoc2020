#include <stdio.h>

void aoc_2a() {
  FILE *input = fopen("input/2.txt", "r");
  int count = 0;
  int total = 0;
  int c, minc, maxc, checkedCount;
  char checked;

  while (fscanf(input, "%d-%d %c: ", &minc, &maxc, &checked) > 0) {
    checkedCount = 0;
    while ((c = getc(input)) != '\n') {
      if (c == checked) {
        ++checkedCount;
      }
    }
    if (checkedCount >= minc &&
        checkedCount <= maxc) {
      ++count;
    }
    ++total;
  }

  fclose(input);
  printf("Valid: %d out of %d\n", count, total);
}

void aoc_2b() {
  FILE *input = fopen("input/2.txt", "r");
  int count = 0;
  int total = 0;
  int c, firstIndex, secondIndex, index, seenOnce;
  char checked;

  while (fscanf(input, "%d-%d %c: ", &firstIndex, &secondIndex, &checked) == 3) {
    index = 1;
    seenOnce = 0;
    while ((c = getc(input)) != '\n') {
      if ((index == firstIndex || index == secondIndex)
          && c == checked) {
        seenOnce = !seenOnce;
      }
      ++index;
    }
    if (seenOnce) {
      ++count;
    }
    ++total;
  }

  fclose(input);
  printf("Valid: %d out of %d\n", count, total);
}

void day2() {
  aoc_2a();
  aoc_2b();
}
