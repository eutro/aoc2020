#include <stdio.h>
#include <string.h>
#include "../lib/uthash.h"

#define ENTRY_COUNT 64

typedef struct entry {
  char container[32];
  int count;
  struct entry *nextContent;
  struct entry *nextWrapper;
  UT_hash_handle hh;
} ENTRY;

typedef struct {
  char container[32];
  ENTRY *contents;
  ENTRY *wrappers;
  UT_hash_handle hh;
} RULE;

void make_container(char *prefix, char *colour, char *to) {
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
    // printf("Entry: %s -- %s\n", rule->container, entry->container);
    HASH_FIND_STR(*bagSet, entry->container, foundEntry);
    if (foundEntry == NULL) {
      HASH_ADD_STR(*bagSet, container, entry);
      ++*bagCount;
      wrappers_loop(rules, bagSet, bagCount, entry->container);
    }
    entry = entry->nextWrapper;
  }
}

int get_wrappers(RULE *rules, char *bag) {
  ENTRY *bagSet = NULL;
  int bagCount = 0;
  wrappers_loop(rules, &bagSet, &bagCount, bag);
  return bagCount;
}

int get_containers(RULE *rules, char *bag) {
  // printf("Traversing %s\n", bag);
  RULE *rule;
  HASH_FIND_STR(rules, bag, rule);
  if (rule == NULL) {
    printf("Null rule\n");
    return 0;
  }
  ENTRY *entry = rule->contents;
  int count = 1;
  while (entry != NULL) {
    // printf("Entry: %s -> %s\n", rule->container, entry->container);
    // could cache but eh
    count += entry->count * get_containers(rules, entry->container);
    entry = entry->nextContent;
  }
  return count;
}

RULE *find_or_make_rule(RULE **rules, char *prefix, char *colour) {
  RULE *rule;
  char container[32];
  make_container(prefix, colour, container);
  HASH_FIND_STR(*rules, container, rule);
  if (rule == NULL) {
    rule = (RULE *)malloc(sizeof(*rule));
    strcpy(rule->container, container);
    rule->contents = NULL;
    rule->wrappers = NULL;
    HASH_ADD_STR(*rules, container, rule);
  }
  return rule;
}

void day7() {
  FILE *input = fopen("input/7.txt", "r");

  RULE *rules = NULL;

  char outPrefix[16];
  char outColour[16];
  char inPrefix[16];
  char inColour[16];
  int count, i, j;
  RULE *rule;
  RULE *outRule;
  ENTRY *entry;
  while (fscanf(input, "%s %s bags contain ", outPrefix, outColour) == 2) {
    outRule = find_or_make_rule(&rules, outPrefix, outColour);
    for (i = 0; i < ENTRY_COUNT; ++i) {
      if (fscanf(input, "%d %s %s bag", &count, inPrefix, inColour) != 3) {
        fscanf(input, "no other bags.");
        break;
      }
      entry = (ENTRY *)malloc(sizeof(*entry));
      entry->count = count;
      make_container(outPrefix, outColour, entry->container);

      rule = find_or_make_rule(&rules, inPrefix, inColour);
      entry->nextWrapper = rule->wrappers;
      rule->wrappers = entry;

      entry = (ENTRY *)malloc(sizeof(*entry));
      entry->count = count;
      make_container(inPrefix, inColour, entry->container);

      entry->nextContent = outRule->contents;
      outRule->contents = entry;

      if (count != 1) {
        getc(input);
      }
      if (getc(input) == '.') { // or ','
        ++i;
        break;
      }
      getc(input); // ' '
    }
  }

  /*
  for (rule = rules; rule != NULL; rule=rule->hh.next) {
    printf("Rule: %s.\n", rule->container);
    int wc = 0;
    entry = rule->wrappers;
    while (entry != NULL) {
      ++wc;
      printf(" <- * %d %s\n", entry->count, entry->container);
      entry = entry->nextWrapper;
    }
    int ic = 0;
    entry = rule->contents;
    while (entry != NULL) {
      ++ic;
      printf(" -> %d * %s\n", entry->count, entry->container);
      entry = entry->nextContent;
    }
  }
  */

  printf("Ways: %d\n", get_wrappers(rules, "shiny gold"));
  printf("Contents: %d\n", get_containers(rules, "shiny gold") - 1);
  fclose(input);
}
