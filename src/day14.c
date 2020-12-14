#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "../lib/uthash.h"

#define MASK_SIZE 36

typedef unsigned long NUM;

typedef struct mask {
  bool mask[MASK_SIZE];
  bool maskMask[MASK_SIZE];
} MASK;

typedef struct mem_addr {
  NUM address;
  NUM value;
  UT_hash_handle hh;
} ADDRESS;

void setmem(ADDRESS **mem, NUM address, NUM value) {
  ADDRESS *addr;
  HASH_FIND(hh, *mem, &address, sizeof(address), addr);
  if (addr == NULL) {
    addr = malloc(sizeof(*addr));
    addr->address = address;
    HASH_ADD(hh, *mem, address, sizeof(address), addr);
  }
  addr->value = value;
}

void printmask(MASK *mask) {
  for (long i = 0; i < MASK_SIZE; ++i) {
    if (mask->maskMask[i]) {
      if (mask->mask[i]) {
        printf("1");
      } else {
        printf("0");
      }
    } else {
      printf("X");
    }
  }
  printf("\n");
}

NUM applymask(MASK *mask, NUM value) {
  for (long i = 0; i < MASK_SIZE; ++i) {
    if (mask->maskMask[i]) {
      if (mask->mask[i]) {
        value |= 1L << (MASK_SIZE - i - 1);
      } else {
        value &= ~(1L << (MASK_SIZE - i - 1));
      }
    }
  }
  return value;
}

unsigned long long runprogram(FILE *input, void doset(ADDRESS**, MASK*, NUM, NUM)) {
  MASK mask = {
    .mask = {},
    .maskMask = {}
  };
  ADDRESS *mem = NULL;

  int c;
  while (getc(input) != EOF) {
    switch (getc(input)) {
    case 'a': {
      assert(fscanf(input, "sk = ") == 0);
      int i = 0;
      while ((c = getc(input)) != '\n') {
        switch (c) {
        case '1':
          mask.mask[i] = true;
          mask.maskMask[i] = true;
          break;
        case '0':
          mask.mask[i] = false;
          mask.maskMask[i] = true;
          break;
        case 'X':
          mask.maskMask[i] = false;
          break;
        }
        ++i;
      }
      // printmask(&mask);
      break;
    }
    case 'e': {
      NUM address;
      NUM value;
      assert(fscanf(input, "m[%lu] = %lu\n", &address, &value) == 2);
      doset(&mem, &mask, address, value);
    }
    }
  }

  unsigned long long sum = 0;
  ADDRESS *addr, *tmp;
  HASH_ITER(hh, mem, addr, tmp) {
    sum += addr->value;
    HASH_DEL(mem, addr);
    free(addr);
  }
  return sum;
}

void setp1(ADDRESS **mem, MASK *mask, NUM address, NUM value) {
  setmem(mem, address, applymask(mask, value));
}

void setp2(ADDRESS **mem, MASK *mask, NUM address, NUM value) {
  int i, j = 0, floating = 0;
  for (i = 0; i < MASK_SIZE; ++i) {
    if (mask->maskMask[i]) {
      if (mask->mask[i]) {
        address |= 1L << (MASK_SIZE - i - 1);
      }
    } else {
      ++floating;
    }
  }

  int bits[floating];
  for (i = 0; i < MASK_SIZE; ++i) {
    if (!mask->maskMask[i]) {
      bits[j] = i;
      ++j;
    }
  }

  NUM masked_addr = address;
  for (size_t perm = 0; perm < (1 << floating); ++perm) {
    NUM address = masked_addr;
    for (j = 0; j < floating; ++j) {
      if (perm & (1L << j)) {
        address ^= (1L << (MASK_SIZE - bits[j] - 1));
      }
    }
    setmem(mem, address, value);
  }
}

void day14() {
  FILE *input = fopen("input/14.txt", "r");

  printf("Sum: %llu\n", runprogram(input, setp1));
  fseek(input, 0, SEEK_SET);
  printf("Sum: %llu\n", runprogram(input, setp2));

  fclose(input);
}
