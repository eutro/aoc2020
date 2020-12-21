#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>

#include "../lib/utarray.h"

#define MAX_LIMIT 1000
#define MAX_FIELDS 32

typedef int FieldSet;
typedef int Field;
typedef unsigned int Value;
typedef FieldSet FieldDefs[MAX_LIMIT];
typedef char *Namelookup[MAX_FIELDS];

typedef struct ticket {
  Value values[MAX_FIELDS];
} Ticket;

static bool setempty(FieldSet s) {
  return s == 0;
}

static FieldSet singleton(Field f) {
  return 1 << f;
}

static Field getfirst(FieldSet set) {
  return ffs(set) - 1;
}

static bool issingleton(FieldSet set) {
  return set && (!(set & (set - 1)));
}

static FieldSet checkfield(FieldDefs defs, Value value) {
  return defs[value];
}

bool readdef(FILE *fp, FieldDefs defs, Namelookup names, Field field) {
  char *buf = NULL;
  size_t sz;

  getline(&buf, &sz, fp);

  if (strcmp(buf, "\n") == 0) {
    free(buf);
    return false;
  }

  size_t i;
  for (i = 0; buf[i] != ':'; ++i) {
    assert(buf[i] != '\0');
  }
  buf[i] = '\0';
  names[field] = strdup(buf);

  i += 2;

  FieldSet fieldset = singleton(field);
  assert(getfirst(fieldset) == field);

  int j, start, end;
  assert(sscanf(&buf[i], "%d-%d", &start, &end) == 2);
  for (j = start; j <= end; ++j) {
    defs[j] |= fieldset;
  }

  while (buf[i++] != ' ');
  while (buf[i++] != ' ');

  assert(sscanf(&buf[i], "%d-%d", &start, &end) == 2);
  for (j = start; j <= end; ++j) {
    defs[j] |= fieldset;
  }

  free(buf);
  return true;
}

void day16() {
  FieldDefs defs = {};
  Namelookup names = {};
  Field fieldi = 0;

  FILE *input = fopen("input/16.txt", "r");

  while (readdef(input, defs, names, fieldi)) ++fieldi;
  assert(fscanf(input, "your ticket:\n") == 0);

  Ticket myticket;
  memset(&myticket.values, 0, sizeof(myticket.values));
  Field fieldcount = 0;
  while (fscanf(input, "%d,", &myticket.values[fieldcount++]) == 1);

  assert(fscanf(input, "nearby tickets:\n") == 0);

  UT_icd ticketicd = { sizeof(Ticket), NULL, NULL, NULL };
  UT_array *nearbytickets = NULL;
  utarray_new(nearbytickets, &ticketicd);

  // let m be the number of fields on a ticket
  // let n be the number of nearby tickets
  // let v be the number of fieldid tickets

  Ticket *nearticket;
  Value value;
  int c;
  bool invalid;
  size_t invalid_score = 0;
  // collecting fieldid tickets is O(nm), it can't get better
  while ((c = getc(input)) != -1) {
    ungetc(c, input);

    nearticket = malloc(sizeof(*nearticket));
    invalid = false;
    fieldcount = 0;
    while (fscanf(input, "%d", &value) == 1) {
      // checking whether a given field is invalid is O(1)
      if (setempty(checkfield(defs, value))) {
        invalid_score += value;
        invalid = true;
      }
      nearticket->values[fieldcount++] = value;
      if (getc(input) != ',') break;
    }
    if (!invalid) {
      utarray_push_back(nearbytickets, nearticket);
    }
    free(nearticket);
  }
  fclose(input);
  printf("Score: %ld\n", invalid_score);

  FieldSet possible;
  Ticket *current;
  FieldSet possiblefields[fieldcount];
  memset(possiblefields, 0, sizeof(possiblefields));
  // this part is O(vm)
  for (fieldi = 0; fieldi < fieldcount; ++fieldi) {
    possible = checkfield(defs, myticket.values[fieldi]);
    current = NULL;
    while (current = utarray_next(nearbytickets, current)) {
      possible &= checkfield(defs, current->values[fieldi]);
    }
    possiblefields[fieldi] = possible;
  }

  bool allresolved;
  bool changed;
  FieldSet resolved = 0;
  // this part is O(m^2)
  // all set operations are assumed to be O(1)
  // the inner loop occurs v times,
  // the outer loop occurs at most v times, if each loop resolves exactly one field
  do {
    allresolved = true;
    changed = false;
    for (fieldi = 0; fieldi < fieldcount; ++fieldi) {
      if (issingleton(possiblefields[fieldi])) {
        resolved |= possiblefields[fieldi];
        changed = true;
      } else {
        possiblefields[fieldi] &= ~resolved;
        if (issingleton(possiblefields[fieldi])) {
          resolved |= possiblefields[fieldi];
          changed = true;
        } else {
          allresolved = false;
        }
      }
    }
  } while (changed && !allresolved);

  unsigned long product = 1;
  char *name;
  // O(m) at the end for multiplying the fields
  // taking strstr and ffs to be O(1)
  for (fieldi = 0; fieldi < fieldcount; ++fieldi) {
    if (issingleton(possiblefields[fieldi])) {
      name = names[getfirst(possiblefields[fieldi])];
      if (strstr(name, "departure") == name) {
        product *= myticket.values[fieldi];
      }
    }
  }
  printf("Product: %ld\n", product);

  utarray_free(nearbytickets);
  for (fieldi = 0; fieldi < fieldcount; ++fieldi) {
    free(names[fieldi]);
  }
}
