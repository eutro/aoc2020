#ifndef COMPILER_H
#define COMPILER_H

#include "vm.h"

#include "lib/uthash.h"
#include "lib/utarray.h"

typedef struct body BODY;
typedef void (*SpecialForm)(BODY *body, VM *vm, LispObject *expr);
typedef struct binding Binding;

struct binding {
  char *name;
  size_t value;
  UT_hash_handle hh;
};

struct body {
  UT_array *insns;
  size_t insn_count;
  Binding *lexicon;
  BODY *container;
  size_t max_stack;
  size_t stack;
  size_t binding_count;
  size_t closure_size;
  LispObject *closures;
};

CompiledBody *compile(VM *vm, LispObject *expr);
void freebody(CompiledBody *body);

#endif
