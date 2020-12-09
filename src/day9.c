#include <stdio.h>

#define PREAMBLE_LENGTH 25

// O(n^2) but PREAMBLE_LENGTH is small enough,
// no point optimizing with sets
int check_for_sum(unsigned long *numbers, unsigned long target) {
  int i, j;
  for (i = 0; i < PREAMBLE_LENGTH; ++i) {
    for (j = 0; j < PREAMBLE_LENGTH; ++j) {
      if (i != j && numbers[i] + numbers[j] == target) return 1;
    }
  }
  return 0;
}

unsigned long min(unsigned long *numbers, int length) {
  unsigned long v = numbers[0];
  for (int i = 1; i < length; ++i) {
    if (numbers[i] < v) v = numbers[i];
  }
  return v;
}

unsigned long max(unsigned long *numbers, int length) {
  unsigned long v = numbers[0];
  for (int i = 1; i < length; ++i) {
    if (numbers[i] > v) v = numbers[i];
  }
  return v;
}

void day9() {
  FILE *input = fopen("input/9.txt", "r");

  int numberCount;
  unsigned long n;
  for (numberCount = 0;
       fscanf(input, "%ld", &n) == 1;
       ++numberCount) {
  }

  int index;
  unsigned long allNumbers[numberCount];
  fseek(input, 0, SEEK_SET);
  for (index = 0;
       fscanf(input, "%ld", &allNumbers[index]) == 1;
       ++index) {
  }
  fclose(input);

  int slideIndex;
  unsigned long prevNumbers[PREAMBLE_LENGTH];
  for (slideIndex = 0; slideIndex < PREAMBLE_LENGTH; ++slideIndex) {
    prevNumbers[slideIndex] = allNumbers[slideIndex];
  }

  unsigned long invalid;
  index = slideIndex;
  // O(n) for n = numberCount.
  // check_for_sum can be considered O(1) because of the small PREAMBLE_LENGTH
  for (slideIndex = 0; index < numberCount; ++index) {
    if (!check_for_sum(prevNumbers, allNumbers[index])) {
      printf("Invalid: %ld\n", invalid = allNumbers[index]);
      break;
    }
    prevNumbers[slideIndex] = allNumbers[index];
    slideIndex = (slideIndex + 1) % PREAMBLE_LENGTH;
  }

  // part 2 is O(n)
  int startIndex = 0;
  int endIndex;
  unsigned long total = allNumbers[0];
  for (endIndex = 1; endIndex < numberCount; ++endIndex) {
    total += allNumbers[endIndex];
    while (total > invalid && startIndex < endIndex) {
      total -= allNumbers[startIndex++];
    }
    if (total == invalid) {
      unsigned long minValue = min(&allNumbers[startIndex], 1 + endIndex - startIndex);
      unsigned long maxValue = max(&allNumbers[startIndex], 1 + endIndex - startIndex);
      printf("Weakness: %ld + %ld = %ld\n",
             minValue, maxValue,
             minValue + maxValue);
      break;
    }
  }
}
