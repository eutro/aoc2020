#include <stdarg.h>

#include "gc.h"

LispObject *intern(VM *vm, char *name) {
  Symbol *sym;
  HASH_FIND_STR(vm->SYMBOLS, name, sym);
  if (sym == NULL) {
    char *symName = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(symName, name);
    LispPrimitive value = { .symbol = { .name = symName } };
    LispObject *obj = new_obj(vm, &value, SYMBOL);
    sym = &obj->value.symbol;
    sym->this = obj;
    HASH_ADD_KEYPTR(hh, vm->SYMBOLS, sym->name, strlen(sym->name), sym);
  }
  return sym->this;
}

LispObject *cons(VM *vm, LispObject *car, LispObject *cdr) {
  LispPrimitive value = { .cons = { car, cdr } };
  return new_obj(vm, &value, CONS);
}

LispObject *wrap_int(VM *vm, long v) {
  LispPrimitive value = { .integer = v };
  return new_obj(vm, &value, INTEGER);
}

LispObject *wrap_string(VM *vm, char *string) {
  LispPrimitive value = { .string = strdup(string) };
  return new_obj(vm, &value, STRING);
}

LispObject *wrap_native(VM *vm, NativeFunc func, char *name) {
  LispPrimitive value = { .func = { func, NATIVE, strdup(name) } };
  return new_obj(vm, &value, FUNC);
}

LispObject *wrap_closure(VM *vm, Closure closure, /* nullable */ char *name) {
  LispPrimitive value = { .func = {
      { .closure = closure }, CLOSURE, name == NULL ? NULL : strdup(name) }
  };
  return new_obj(vm, &value, FUNC);
}
