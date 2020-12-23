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

  int target;
  for (int i = 0; i < rounds; ++i) {
    hand = cups->next;
    cups->next = hand->next->next->next;

    target = cups->val - 1;
  findtarget:
    if (target < CUP_MIN) target = maxc;
    dest = values[target - 1];
    if (dest == hand ||
        dest == hand->next ||
        dest == hand->next->next) {
      --target;
      goto findtarget;
    }

    hand->next->next->next = dest->next;
    dest->next = hand;

    cups = cups->next;
  }

  cups = values[0];
  free(values);
  return cups;
}

Cup *memv(Cup *list, int val) {
  if (list->val == val) return list;
  for (Cup *n = list->next; n != list; n = n->next) {
    if (n->val == val) return n;
  }
  return NULL;
}

void printend(Cup *start) {
  printf("Cups: ");
  for (Cup *cups = start->next; cups != start; cups = cups->next) {
    printf("%d", cups->val);
  }
  printf("\n");
}

void day23() {
  FILE *input = fopen("input/23.txt", "r");

  Cup *p2c, *p1c, *end;
  end = p2c = malloc(sizeof(Cup));
  int cupc = 0;

  end->val = getc(input) - '0';

  int c;
  while ((c = getc(input)) != '\n' && c != -1) {
    end = end->next = malloc(sizeof(Cup));
    end->val = c - '0';
    ++cupc;
  }
  end->next = NULL;
  fclose(input);

  STACK_COPY_WITH(p2c, next, p1c);
  for (cupc = MAXC_1; cupc < MAXC_2; ++cupc) {
    end = end->next = malloc(sizeof(Cup));
    end->val = cupc + 1;
  }
  end->next = p2c;

  for (end = p1c; end->next != NULL; end = end->next);
  end->next = p1c;

  printend(crabgame(p1c, TURNS_1, MAXC_1));
  end = p1c->next;
  p1c->next = NULL;
  STACK_CLEAR_WITH(end, next);

  p2c = crabgame(p2c, TURNS_2, MAXC_2)->next;
  printf("Cups: %d * %d = %ld\n",
         p2c->val, p2c->next->val,
         (long) p2c->val * (long) p2c->next->val);
  end = p2c->next;
  p2c->next = NULL;
  STACK_CLEAR_WITH(end, next);
}
