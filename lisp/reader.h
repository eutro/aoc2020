#ifndef READER_H
#define READER_H

#include <stdio.h>

#include "vm.h"

typedef struct readable READABLE;
typedef char peek_f();
typedef void advn_f();

struct readable {
  void *value;
  peek_f *peek;
  advn_f *advance;
};

LispObject *read(VM *vm, READABLE *buff);
void advancec(FILE *fp);
char peekc(FILE *fp);
char peekbuff(char **buff);
void incbuff(char **buff);

#endif
