#ifndef BUILTINS_H
#define BUILTINS_H

#include <assert.h>

#include "vm.h"

LispObject *glref(VM *vm, LispObject *sym);
LispObject *gldef(VM *vm, LispObject *sym, LispObject *val);

LispObject *pr(VM *vm, LispObject *obj);
LispObject *prn(VM *vm, LispObject *obj);
LispObject *assoc(VM *vm, LispObject *alist, LispObject *key);
LispObject *equal(VM *vm, LispObject *a, LispObject *b);
LispObject *car(VM *vm, LispObject *obj);
LispObject *cdr(VM *vm, LispObject *obj);
LispObject *setcar(VM *vm, LispObject *obj, LispObject *val);
LispObject *setcdr(VM *vm, LispObject *obj, LispObject *val);
LispObject *apply1(VM *vm, LispObject *f, LispObject *args);
LispObject *eval(VM *vm, LispObject *expr);
LispObject *list(VM *vm, LispObject **args, size_t argc);
LispObject *apply(VM *vm, LispObject **args, size_t argc);

LispObject *nilp(VM *vm, LispObject *obj);
LispObject *consp(VM *vm, LispObject *obj);
LispObject *symbolp(VM *vm, LispObject *obj);
LispObject *integerp(VM *vm, LispObject *obj);
LispObject *stringp(VM *vm, LispObject *obj);
LispObject *funcp(VM *vm, LispObject *obj);

void defbuiltins(VM *vm);

#define wrapargs(FUNC, NAME, ARGCOUNT, INVOKE...)                       \
  LispObject * NAME(VM *vm, LispObject **args, size_t argc) {           \
    ASSERTVM(argc == ARGCOUNT, "Bad arity, expected " #ARGCOUNT);       \
    FUNC (vm, INVOKE);                                                  \
  }

#define defnative(NAME, NNAME)					\
  gldef(vm, intern(vm, #NAME), wrap_native(vm, NNAME, #NAME));

#endif
