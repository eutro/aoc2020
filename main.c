#include <stdio.h>
#include <string.h>

// O(n)
void aoc_1a() {
  int expenses[2021] = {};
  FILE *input = fopen("input/1.txt", "r");
  int expense, target;
  while (fscanf(input, "%d", &expense) > 0) {
    target = 2020 - expense;
    if (target < 0) {
      // all expenses are less than 2020 but better to be safe
      continue;
    } else if (expenses[target]) {
      printf("Product: %d * %d = %d\n", expense, target, expense * target);
      fclose(input);
      return;
    } else {
      expenses[expense] = 1;
    }
  }

  fclose(input);
  printf("Couldn't find suitable pair.\n");
}

// O(n^2)
void aoc_1b() {
  int expenses[2021] = {};
  FILE *input = fopen("input/1.txt", "r");
  int c, lineCount = 0;
  while ((c = getc(input)) != EOF) {
    if (c == '\n') {
      ++lineCount;
    }
  }
  fseek(input, 0, SEEK_SET);

  int expense, i, j, target;
  int targets[lineCount];
  for (i = 0; i < lineCount && (fscanf(input, "%d", &expense) > 0); i++) {
    for (j = 0; j < i; j++) {
      target = targets[j] - expense;
      if (target < 0) {
        continue;
      } else if (expenses[target]) {
        printf("Product: %d * %d * %d = %d\n",
               expense, target, (2020 - expense - target),
               expense * target * (2020 - expense - target));
        fclose(input);
        return;
      }
    }
    if (expense <= 2020) {
      expenses[expense] = 1;
      targets[i] = 2020 - expense;
    }
  }

  fclose(input);
  printf("Couldn't find suitable triple.\n");
}

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

int aoc_3_traverseSlope(FILE *fp, int dx, int dy) {
  int trees = 0;
  int xPos = 0;
  int yPos = -1;
  char line[64];

  fseek(fp, 0, SEEK_SET);
  while (fscanf(fp, "%s", &line[0]) > 0) {
    ++yPos;
    if ((yPos % dy) > 0) continue;
    if (line[xPos] == '#') {
      ++trees;
    }
    xPos = (xPos + dx) % strlen(line);
  }

  return trees;
}

void aoc_3a() {
  FILE *input = fopen("input/3.txt", "r");
  printf("Trees: %d\n", aoc_3_traverseSlope(input, 3, 1));
  fclose(input);
}

void aoc_3b() {
  FILE *input = fopen("input/3.txt", "r");
  long d = aoc_3_traverseSlope(input, 7, 1);
  long c = aoc_3_traverseSlope(input, 5, 1);
  long b = aoc_3_traverseSlope(input, 3, 1);
  long a = aoc_3_traverseSlope(input, 1, 1);
  long e = aoc_3_traverseSlope(input, 1, 2);
  printf("Trees: %ld * %ld * %ld * %ld * %ld = %ld",
         a, b, c, d, e,
         a * b * c * d * e);
  fclose(input);
}

int main() {
  printf("Day 1:\n");
  aoc_1a();
  aoc_1b();
  printf("Day 2:\n");
  aoc_2a();
  aoc_2b();
  printf("Day 3:\n");
  aoc_3a();
  aoc_3b();
  return 0;
}
