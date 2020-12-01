#include <stdio.h>

// O(n)
void aoc_1a() {
  int expenses[2021];

  for (int i = 0; i < 2021; ++i) {
    expenses[i] = 0;
  }

  FILE *input = fopen("input/1.txt", "r");
  int read, target;
  while (fscanf(input, "%d", &read) > 0) {
    target = 2020 - read;
    if (target < 0) {
      // all expenses are less than 2020 but better to be safe
      continue;
    } else if (expenses[target]) {
      printf("Product: %d * %d = %d\n", read, target, read * target);
      fclose(input);
      return;
    } else {
      expenses[read] = 1;
    }
  }

  fclose(input);
  printf("Couldn't find suitable pair.\n");
}

// O(n^2)
void aoc_1b() {
  int expenses[2021];

  for (int i = 0; i < 2021; ++i) {
    expenses[i] = 0;
  }

  FILE *input = fopen("input/1.txt", "r");

  int c, lineCount = 0;
  while ((c = getc(input)) != EOF) {
    if (c == '\n') {
      ++lineCount;
    }
  }
  fseek(input, 0, SEEK_SET);

  int read, i, j, target;
  int targets[lineCount];
  for (i = 0; i < lineCount && (fscanf(input, "%d", &read) > 0); i++) {
    for (j = 0; j < i; j++) {
      target = targets[j] - read;
      if (target < 0) {
        continue;
      } else if (expenses[target]) {
        printf("Product: %d * %d * %d = %d\n",
               read, target, (2020 - read - target),
               read * target * (2020 - read - target));
        fclose(input);
        return;
      } else {
        expenses[read] = 1;
      }
    }
    expenses[read] = 1;
    targets[i] = 2020 - read;
  }

  fclose(input);
  printf("Couldn't find suitable triple.\n");
}

int main() {
  printf("Day 1:\n");
  aoc_1a();
  aoc_1b();
  return 0;
}
