#include <stdio.h>
#include <string.h>
#include "../lib/uthash.h"

typedef struct entry {
  char bag[32];
  int count;
  struct entry *nextContent;
  struct entry *nextWrapper;
  UT_hash_handle hh; // needed for the set
} ENTRY;

typedef struct {
  char bag[32];
  ENTRY *contents;
  ENTRY *wrappers;
  UT_hash_handle hh;
} RULE;

void bag_name(char *prefix, char *colour, char *to) {
  int i, j;
  for (i = 0; prefix[i] != '\0'; ++i) {
    to[i] = prefix[i];
  }
  to[i] = ' ';
  for (j = 0; colour[j] != '\0'; ++j) {
    ++i;
    to[i] = colour[j];
  }
  to[i + 1] = '\0';
}

void wrappers_loop(RULE *rules, ENTRY **bagSet, int *bagCount, char *bag) {
  RULE *rule;
  ENTRY *foundEntry;
  HASH_FIND_STR(rules, bag, rule);
  if (rule == NULL) {
    printf("Null rule\n");
    return;
  }
  ENTRY *entry = rule->wrappers;
  while (entry != NULL) {
    // printf("Entry: %s -- %s\n", rule->bag, entry->bag);
    HASH_FIND_STR(*bagSet, entry->bag, foundEntry);
    if (foundEntry == NULL) {
      HASH_ADD_STR(*bagSet, bag, entry);
      ++*bagCount;
      wrappers_loop(rules, bagSet, bagCount, entry->bag);
    }
    entry = entry->nextWrapper;
  }
}

int get_wrappers(RULE *rules, char *bag) {
  ENTRY *bagSet = NULL;
  int bagCount = 0;
  wrappers_loop(rules, &bagSet, &bagCount, bag);
  HASH_CLEAR(hh, bagSet);
  return bagCount;
}

int get_contents(RULE *rules, char *bag) {
  RULE *rule;
  HASH_FIND_STR(rules, bag, rule);
  if (rule == NULL) {
    printf("Null rule\n");
    return 0;
  }
  ENTRY *entry = rule->contents;
  int count = 0;
  while (entry != NULL) {
    //                           could cache but eh
    count += entry->count * (1 + get_contents(rules, entry->bag));
    entry = entry->nextContent;
  }
  return count;
}

RULE *find_or_make_rule(RULE **rules, char *bag) {
  RULE *rule;
  HASH_FIND_STR(*rules, bag, rule);
  if (rule == NULL) {
    rule = (RULE *)malloc(sizeof(*rule));
    strcpy(rule->bag, bag);
    rule->contents = NULL;
    rule->wrappers = NULL;
    HASH_ADD_STR(*rules, bag, rule);
  }
  return rule;
}

void day7() {
  FILE *input = fopen("input/7.txt", "r");

  RULE *rules = NULL;

  char inPrefix[16];
  char inColour[16];
  RULE *inRule;
  char inBag[32];

  char outPrefix[16];
  char outColour[16];
  RULE *outRule;
  char outBag[32];

  ENTRY *entry;
  int count;
  while (fscanf(input, "%s %s bags contain ", outPrefix, outColour) == 2) {
    bag_name(outPrefix, outColour, outBag);
    outRule = find_or_make_rule(&rules, outBag);
    while (1) {
      if (fscanf(input, "%d %s %s bag", &count, inPrefix, inColour) != 3) {
        fscanf(input, "no other bags.");
        break;
      }
      bag_name(inPrefix, inColour, inBag);

      entry = (ENTRY *)malloc(sizeof(*entry));
      entry->count = count;
      strcpy(entry->bag, outBag);

      inRule = find_or_make_rule(&rules, inBag);
      entry->nextWrapper = inRule->wrappers;
      inRule->wrappers = entry;

      entry = (ENTRY *)malloc(sizeof(*entry));
      entry->count = count;
      strcpy(entry->bag, inBag);

      entry->nextContent = outRule->contents;
      outRule->contents = entry;

      if (count != 1) {
        getc(input); // s
      }
      if (getc(input) == '.') { // or ','
        break;
      }
      getc(input); // ' '
    }
  }

  /*
  for (rule = rules; rule != NULL; rule=rule->hh.next) {
    printf("Rule: %s.\n", rule->bag);
    int wc = 0;
    entry = rule->wrappers;
    while (entry != NULL) {
      ++wc;
      printf(" <- * %d %s\n", entry->count, entry->bag);
      entry = entry->nextWrapper;
    }
    int ic = 0;
    entry = rule->contents;
    while (entry != NULL) {
      ++ic;
      printf(" -> %d * %s\n", entry->count, entry->bag);
      entry = entry->nextContent;
    }
  }
  */

  printf("Ways: %d\n", get_wrappers(rules, "shiny gold"));
  printf("Contents: %d\n", get_contents(rules, "shiny gold"));

  fclose(input);

  RULE *rule;
  RULE *nextRule;
  ENTRY *nextEntry;
  HASH_ITER(hh, rules, rule, nextRule) {
    entry = rule->wrappers;
    while (entry != NULL) {
      nextEntry = entry->nextWrapper;
      free(entry);
      entry = nextEntry;
    }
    entry = rule->contents;
    while (entry != NULL) {
      nextEntry = entry->nextContent;
      free(entry);
      entry = nextEntry;
    }
    HASH_DEL(rules, rule);
    free(rule);
  }
}
