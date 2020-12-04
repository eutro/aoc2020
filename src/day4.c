#include <stdio.h>
#include <string.h>

int field_to_index(char *field) {
  switch (field[0]) {
  case 'b':
    return 0; // birth year
  case 'i':
    return 1; // issue year
  case 'h':
    switch (field[1]) {
    case 'g':
      return 3; // height
    case 'c':
      return 4; // hair colour
    default:
      return -1;
    }
  case 'e':
    switch (field[1]) {
    case 'y':
      return 2; // expiration year
    case 'c':
      return 5; // eye colour
    default:
      return -1;
    }
  case 'p':
    return 6; // passport id
  case 'c':
    return 7; // country id
  default:
    return -1;
  }
}

int check_newline(FILE *fp) {
  int c;
  if ((c = getc(fp)) != '\n') {
    ungetc(c, fp);
    return 0;
  }
  if ((c = getc(fp)) != '\n') {
    ungetc(c, fp);
    return 0;
  }
  return 1;
}

int check_byr(char *value) {
  // (Birth Year) - four digits; at least 1920 and at most 2002.
  if (strlen(value) != 4) return 0;
  int year;
  return sscanf(value, "%d", &year) == 1 &&
    year >= 1920 && year <= 2002;
}

int check_iyr(char *value) {
  // (Issue Year) - four digits; at least 2010 and at most 2020.
  if (strlen(value) != 4) return 0;
  int year;
  return sscanf(value, "%d", &year) == 1 &&
    year >= 2010 && year <= 2020;
}

int check_eyr(char *value) {
  // (Expiration Year) - four digits; at least 2020 and at most 2030.
  if (strlen(value) != 4) return 0;
  int year;
  return sscanf(value, "%d", &year) == 1 &&
    year >= 2020 && year <= 2030;
}

int check_hgt(char *value) {
  // (Height) - a number followed by either cm or in:
  //  cm, the number must be at least 150 and at most 193.
  //  in, the number must be at least 59 and at most 76.
  int height, i, j;
  int len = strlen(value) - 2;
  for (i = 0; i < len; ++i) {
    if (value[i] < '0' || value[i] > '9') return 0;
  }
  if (sscanf(value, "%d", &height) != 1) return 0;
  if (!strcmp(&value[i], "cm")) {
    return height >= 150 && height <= 193;
  } else if (!strcmp(&value[i], "in")) {
    return height >= 59 && height <= 76;
  }
  return 0;
}

int check_hcl(char *value) {
  // (Hair Color) - a # followed by exactly six characters 0-9 or a-f.
  if (strlen(value) != 7) return 0;
  if (value[0] != '#') return 0;
  int i;
  for (i = 1; i < 7; ++i) {
    if (!(value[i] >= '0' && value[i] <= '9') &&
        !(value[i] >= 'a' && value[i] <= 'f')) return 0;
  }
  return 1;
}

int check_ecl(char *value) {
  // (Eye Color) - exactly one of: amb blu brn gry grn hzl oth.
  return !strcmp(value, "amb")
    || !strcmp(value, "blu")
    || !strcmp(value, "brn")
    || !strcmp(value, "gry")
    || !strcmp(value, "grn")
    || !strcmp(value, "hzl")
    || !strcmp(value, "oth");
}

int check_pid(char *value) {
  // (Passport ID) - a nine-digit number, including leading zeroes.
  if (strlen(value) != 9) return 0;
  int i;
  for (i = 0; i < 9; ++i) {
    if (value[i] < '0' || value[i] > '9') return 0;
  }
  return 1;
}

int check_cid(char *value) {
  // (Country ID) - ignored, missing or not.
  return 1;
}

void day4() {
  FILE *input = fopen("input/4.txt", "r");
  char field[4];
  char value[64];
  int (*validators[])(char*) = {
    check_byr,
    check_iyr,
    check_eyr,
    check_hgt,
    check_hcl,
    check_ecl,
    check_pid,
    check_cid
  };
  int counter = 0;
  int validatedCounter = 0;
  int total = 0;
  int mask = 0;
  int validatedMask = 0;
  int checkMask = 0x7F;
  int scanned, index;

  do {
    scanned = fscanf(input, "%3s:%s", &field[0], &value[0]);
    if (scanned != -1) {
      index = field_to_index(&field[0]);
      if (index == -1) {
        printf("Not a valid field: %s\n", &field[0]);
      } else {
        mask |= 1 << index;
        if ((validators[index])(&value[0])) {
          validatedMask |= 1 << index;
        }
      }
    }
    if (check_newline(input) || scanned == -1) {
      if ((mask & checkMask) == checkMask) {
        ++counter;
        if ((validatedMask & checkMask) == checkMask) {
          ++validatedCounter;
        }
      }
      mask = 0;
      validatedMask = 0;
      ++total;
    }
  } while (scanned > 0);

  fclose(input);
  printf("Valid: %d out of %d\n", counter, total);
  printf("Checked: %d out of %d\n", validatedCounter, total);
}
