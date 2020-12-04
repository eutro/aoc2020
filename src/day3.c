#include <stdio.h>

int traverse_slope(FILE *fp, int dx, int dy) {
  int trees = 0;
  int xPos = 0;
  int yPos = -1;

  int lineLength = 0;
  fseek(fp, 0, SEEK_SET);
  while (getc(fp) != '\n') ++lineLength;

  char line[lineLength];
  fseek(fp, 0, SEEK_SET);
  while (fscanf(fp, "%s", &line[0]) > 0) {
    ++yPos;
    if ((yPos % dy) > 0) continue;
    if (line[xPos] == '#') {
      ++trees;
    }
    xPos = (xPos + dx) % lineLength;
  }

  return trees;
}

void day3() {
  FILE *input = fopen("input/3.txt", "r");
  long a = traverse_slope(input, 7, 1);
  long b = traverse_slope(input, 5, 1);
  long c = traverse_slope(input, 3, 1);
  long d = traverse_slope(input, 1, 1);
  long e = traverse_slope(input, 1, 2);
  printf("Trees: %ld\n", c);
  printf("Trees: %ld * %ld * %ld * %ld * %ld = %ld\n",
         a, b, c, d, e,
         a * b * c * d * e);
  fclose(input);
}
