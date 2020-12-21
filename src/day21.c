#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "../lib/uthash.h"

#define INGREDIENT_COUNT 256

typedef struct mask mask;
typedef struct allergen Allergen;
typedef struct ingredient Ingredient;

// 256 bits of mask ohyes
struct mask {
  unsigned long long a, b, c, d;
};

struct ingredient {
  char *name;
  int id;
  Allergen *allergen;
  int occurences;

  UT_hash_handle hh;
};

struct allergen {
  char *name;
  mask possible;
  Ingredient *confirmed;

  UT_hash_handle hh;
};

static mask singleton(int id) {
  if (id <  64) return (mask) { 0, 0, 0, 1L << id };
  if (id < 128) return (mask) { 0, 0, 1L << (id - 64), 0 };
  if (id < 192) return (mask) { 0, 1L << (id - 128), 0, 0 };
  if (id < 256) return (mask) { 1L << (id - 192), 0, 0, 0 };
  fprintf(stderr, "%d out of range for mask\n", id);
  exit(EXIT_FAILURE);
}

static int getfirst(mask set) {
  if (set.d != 0) return ffsll(set.d) - 1;
  if (set.c != 0) return ffsll(set.c) + 64 - 1;
  if (set.b != 0) return ffsll(set.b) + 128 - 1;
  if (set.a != 0) return ffsll(set.a) + 192 - 1;
  return -1;
}

static bool issingleton(mask set) {
  unsigned long long *llp = NULL;
  if (set.d != 0) llp = &set.d;
  if (set.c != 0) { if (llp != NULL) return false; llp = &set.c; }
  if (set.b != 0) { if (llp != NULL) return false; llp = &set.b; }
  if (set.a != 0) { if (llp != NULL) return false; llp = &set.a; }
  return *llp && (!(*llp & (*llp - 1)));
}

static void intersect(mask *a, mask b) {
  a->a &= b.a;
  a->b &= b.b;
  a->c &= b.c;
  a->d &= b.d;
}

static void unionset(mask *a, mask b) {
  a->a |= b.a;
  a->b |= b.b;
  a->c |= b.c;
  a->d |= b.d;
}

static mask complement(mask set) {
  return (mask) { ~set.a, ~set.b, ~set.c, ~set.d };
}

#define emptyset ((mask) { 0, 0, 0, 0 })

Allergen *getallergen(Allergen **allergens, char *name) {
  Allergen *ret;
  HASH_FIND_STR(*allergens, name, ret);
  if (ret == NULL) {
    ret = malloc(sizeof(*ret));
    ret->name = strdup(name);
    ret->possible = complement(emptyset);
    ret->confirmed = NULL;
    HASH_ADD_STR(*allergens, name, ret);
  }
  return ret;
}

Ingredient *getingredient(Ingredient **ingredients, char *name) {
  static int INGREDIENTID = 0;
  Ingredient *ret;
  HASH_FIND_STR(*ingredients, name, ret);
  if (ret == NULL) {
    ret = malloc(sizeof(*ret));
    ret->name = strdup(name);
    ret->id = INGREDIENTID++;
    ret->allergen = NULL;
    ret->occurences = 0;
    HASH_ADD_STR(*ingredients, name, ret);
  }
  return ret;
}

static int ingcmp(const void *a, const void *b) {
  const Ingredient *ia = *(const Ingredient **)a;
  const Ingredient *ib = *(const Ingredient **)b;
  return strcmp(ia->allergen->name, ib->allergen->name);
}

void day21() {
  FILE *input = fopen("input/21.txt", "r");

  Ingredient *INGREDIENTS = NULL;
  Allergen *ALLERGENS = NULL;

  char name[32];
  mask possibleingredients;
  Allergen *allergen, *tmpal;
  Ingredient *ingredient, *tmping;
  Ingredient *ingrlookup[INGREDIENT_COUNT] = {};

  int c;
  while ((c = getc(input)) != '\n' && c != -1) {
    ungetc(c, input);

    possibleingredients = emptyset;
    while ((c = getc(input)) != '(') {
      ungetc(c, input);

      assert(fscanf(input, "%32s ", name) == 1);
      ingredient = getingredient(&INGREDIENTS, name);
      unionset(&possibleingredients, singleton(ingredient->id));
      ingrlookup[ingredient->id] = ingredient;
      ingredient->occurences++;
    }
    assert(fscanf(input, "contains ") == 0);
    while (fscanf(input, "%32s, ", name) == 1) {
      c = name[strlen(name) - 1];
      name[strlen(name) - 1] = '\0';
      allergen = getallergen(&ALLERGENS, name);
      intersect(&allergen->possible, possibleingredients);
      if (c == ')') break;
    }
    assert(getc(input) == '\n');
  }
  fclose(input);

  Allergen *UNRESOLVED = ALLERGENS;
  ALLERGENS = NULL;
  bool changed = true;
  mask resolved = emptyset;

  while (changed && UNRESOLVED != NULL) {
    changed = false;
    HASH_ITER(hh, UNRESOLVED, allergen, tmpal) {
      intersect(&allergen->possible, complement(resolved));
      if (issingleton(allergen->possible)) {
        unionset(&resolved, allergen->possible);
        allergen->confirmed = ingredient = ingrlookup[getfirst(allergen->possible)];
        ingredient->allergen = allergen;
        HASH_DEL(UNRESOLVED, allergen);
        HASH_ADD_STR(ALLERGENS, name, allergen);
        changed = true;
      }
    }
  }
  if (!changed) {
    fprintf(stderr, "Couldn't resolve every allergen.\n");
    exit(EXIT_FAILURE);
  }

  int allergencount = HASH_COUNT(ALLERGENS);
  Ingredient *dangerous[allergencount];
  int occurences = 0;
  int dangerouscount = 0;

  for (ingredient = INGREDIENTS; ingredient != NULL; ingredient = ingredient->hh.next) {
    if (ingredient->allergen == NULL) {
      occurences += ingredient->occurences;
    } else {
      dangerous[dangerouscount++] = ingredient;
    }
  }
  printf("Allergenless: %d occurences\n", occurences);

  assert(allergencount == dangerouscount);
  qsort(dangerous, dangerouscount, sizeof(char*), ingcmp);

  printf("Dangerous: ");
  for (int i = 0; i < dangerouscount; ++i) {
    if (i != 0) printf(",");
    printf("%s", dangerous[i]->name);
  }
  printf("\n");

  HASH_ITER(hh, INGREDIENTS, ingredient, tmping) {
    HASH_DEL(INGREDIENTS, ingredient);
    free(ingredient->name);
    free(ingredient);
  }
  HASH_ITER(hh, ALLERGENS, allergen, tmpal) {
    HASH_DEL(ALLERGENS, allergen);
    free(allergen->name);
    free(allergen);
  }
}
