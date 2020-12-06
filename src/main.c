#include <stdio.h>
#include <stdlib.h>
#include "days.h"

int main(int argc, char **argv) {
  int dayCount = 6;
  int day;
  void (*days[])() = {
    day1, day2, day3, day4, day5,
    day6
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
