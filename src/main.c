#include <stdio.h>
#include <stdlib.h>
#include "days.h"

int main(int argc, char **argv) {
  int dayCount = 25;
  int day;
  void (*days[])() = {
    day1, day2, day3, day4, day5,
    day6, day7, day8, day9, day10,
    day11, day12, day13, day14, day15,
    day16, day17, day18, day19, day20,
    day21, day22, day23, day24, day25,
    // fin
  };
  if (argc > 1) {
    int i;
    for (i = 1; i < argc; ++i) {
      day = atoi(argv[i]);
      if (day > dayCount) {
        printf("Unknown day: %d\n", day);
      } else {
        printf("Day %d:\n", day);
        (days[day - 1])();
      }
    }
  } else {
    for (day = 0; day < dayCount; ++day) {
      printf("Day %d:\n", day + 1);
      (days[day])();
    }
  }
  return 0;
}
