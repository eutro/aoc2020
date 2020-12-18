#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

bool evalnext(FILE *fp, long *result);

bool evalexp(FILE *fp, long *result) {
  int c = getc(fp);
  switch (c) {
  case '(':
    return evalnext(fp, result);
  case '\n':
  case -1:
    return false;
  default:
    ungetc(c, fp);
    fscanf(fp, "%ld", result);
    return true;
  }
}

bool evalnext(FILE *fp, long *result) {
  if (!evalexp(fp, result)) return false;
  int c, op;
  long next;
  while (1) {
    c = getc(fp);
    switch (c) {
    case ')':
    case '\n':
    case -1:
      return true;
    }
    op = getc(fp); getc(fp);
    assert(evalexp(fp, &next));
    switch (op) {
    case '+':
      *result += next;
      break;
    case '*':
      *result *= next;
      break;
    default:
      assert(0);
    }
  }
}

void day18() {
  FILE *input = fopen("input/18.txt", "r");

  long total = 0;
  long result;
  while (evalnext(input, &result)) total += result;
  printf("Total: %ld\n", total);

  fclose(input);
}
