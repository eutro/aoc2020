#include <stdio.h>
#include <string.h>

typedef enum {
  NOP,
  JMP,
  ACC
} OP;

OP get_op(char *code) {
  switch (code[0]) {
  case 'n':
    return NOP;
  case 'j':
    return JMP;
  case 'a':
    return ACC;
  }
  return -1;
}

typedef struct insn {
  OP op;
  int val;
} INSN;

int check_terminates(INSN *instructions, int lines, int *accumulator) {
  *accumulator = 0;
  int index = 0;
  int visited[lines];
  memset(visited, 0, sizeof(*visited) * lines);
  while (index < lines) {
    if (visited[index]) {
      return 0;
    }
    visited[index] = 1;
    switch (instructions[index].op) {
    case NOP:
      ++index;
      break;
    case JMP:
      index += instructions[index].val;
      break;
    case ACC:
      *accumulator += instructions[index].val;
      ++index;
      break;
    }
  }
  return 1;
}

void day8() {
  FILE *input = fopen("input/8.txt", "r");
  int lines = 0;
  int c;
  char code[4];
  char sign;
  int val;
  while (fscanf(input, "%3s %c%d", code, &sign, &val) == 3) {
    ++lines;
  }
  fseek(input, 0, SEEK_SET);

  INSN instructions[lines];
  int index = 0;
  while (fscanf(input, "%3s %c%d", code, &sign, &val) == 3) {
    instructions[index].op = get_op(code);
    instructions[index].val = (sign == '-') ? -val : val;
    ++index;
  }
  fclose(input);

  int accumulator;
  if (!check_terminates(instructions, lines, &accumulator)) {
    printf("Loop: %d\n", accumulator);
  }

  for (index = 0; index < lines; ++index) {
    switch (instructions[index].op) {
    case NOP:
      instructions[index].op = JMP;
      if (check_terminates(instructions, lines, &accumulator)) {
        printf("Terminated: nop->jmp@%d %d\n", index, accumulator);
        return;
      }
      instructions[index].op = NOP;
      break;
    case JMP:
      instructions[index].op = NOP;
      if (check_terminates(instructions, lines, &accumulator)) {
        printf("Terminated: jmp->nop@%d %d\n", index, accumulator);
        return;
      }
      instructions[index].op = JMP;
      break;
    case ACC:
      continue;
    }
  }
}
