#include <stdio.h>

void aoc_1a() {
  int c;
  int floor = 0;
  FILE *input = fopen("input/1.txt", "r");

  while ((c = getc(input)) != EOF) {
    if (c == '(') {
      ++floor;
    } else if (c == ')') {
      --floor;
    }
  }

  fclose(input);

  printf("Floor: %d\n", floor);
}

void aoc_1b() {
  int c;
  int floor = 0;
  FILE *input = fopen("input/1.txt", "r");

  int index = 0;
  while ((c = getc(input)) != EOF) {
    ++index;
    if (c == '(') {
      ++floor;
    } else if (c == ')') {
      --floor;
      if (floor < 0) {
	printf("Basement: %d\n", index);
	break;
      }
    }
  }

  fclose(input);
}

int main() {
  printf("Day 1:\n");
  aoc_1a();
  aoc_1b();
  return 0;
}
