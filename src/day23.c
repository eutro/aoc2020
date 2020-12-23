#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

#define CUP_MIN 1

#define MAXC_1 9
#define TURNS_1 100
#define MAXC_2 1000000
#define TURNS_2 10000000

typedef struct cup Cup;

struct cup {
  int val;
  Cup *next;
};

void printcups(Cup *cups) {
  printf("(%d)", cups->val);
  for (Cup *next = cups->next; next != cups; next = next->next) {
    printf(" %d", next->val);
  }
  printf("\n");
}

Cup *crabgame(Cup *cups, int rounds, int maxc) {
  Cup **values = malloc(sizeof(Cup *) * maxc);
  Cup *hand, *dest;

  values[cups->val - 1] = cups;
  for (hand = cups->next; hand != cups; hand = hand->next) {
    values[hand->val - 1] = hand;
  }

  for (int i = 0; i < rounds; ++i) {
    hand = cups->next;
    cups->next = hand->next->next->next;

    int target = cups->val;
    do {
      --target;
      if (target < CUP_MIN) target = maxc;
      dest = values[target - 1];
    } while (dest == hand ||
             dest == hand->next ||
             dest == hand->next->next);

    hand->next->next->next = dest->next;
    dest->next = hand;

    cups = cups->next;
  }

  cups = values[0];
  free(values);
  return cups;
}

void printp1(Cup *start) {
  printf("Cups: ");
  for (Cup *cups = start->next; cups != start; cups = cups->next) {
    printf("%d", cups->val);
  }
  printf("\n");
}

void printp2(Cup *start) {
  Cup *n = start->next;
  printf("Stars: under %d * %d = %ld\n",
         n->val, n->next->val,
         (long) n->val * (long) n->next->val);
}

void freecups(Cup *cups) {
  Cup *start = cups->next;
  cups->next = NULL;
  STACK_CLEAR_WITH(start, next);
}

void day23() {
  FILE *input = fopen("input/23.txt", "r");

  Cup *p2c, *p1c, *end;
  end = p2c = malloc(sizeof(Cup));

  end->val = getc(input) - '0';

  // read into two cyclic linked lists
  int cupc = 1;
  int c;
  while ((c = getc(input)) != '\n' && c != -1) {
    end = end->next = malloc(sizeof(Cup));
    end->val = c - '0';
    ++cupc;
  }
  end->next = NULL;
  fclose(input);

  STACK_COPY_WITH(p2c, next, p1c);
  for (; cupc < MAXC_2; ++cupc) {
    end = end->next = malloc(sizeof(Cup));
    end->val = cupc + 1;
  }
  end->next = p2c;

  for (end = p1c; end->next != NULL; end = end->next);
  end->next = p1c;

  printp1(crabgame(p1c, TURNS_1, MAXC_1));
  freecups(p1c);

  printp2(crabgame(p2c, TURNS_2, MAXC_2));
  freecups(p2c);
}
