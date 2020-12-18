#ifndef LISP_H
#define LISP_h

#include <stdbool.h>

#include "lib/uthash.h"

typedef struct vm VM;

typedef struct cons Cons;
typedef struct symbol Symbol;
typedef struct func Func;
typedef union func_primitive FuncPrimitive;
typedef enum func_type FuncType;

typedef struct lisp_object LispObject;
typedef union lisp_primitive LispPrimitive;
typedef enum primitive_type PrimitiveType;

typedef LispObject *(*NativeFunc)(VM *vm, LispObject **args, size_t argc);
typedef struct closure Closure;
typedef struct compiled_body CompiledBody;

struct cons {
  LispObject *car;
  LispObject *cdr;
};

struct symbol {
  char *name;
  LispObject *this; // this->value.symbol.this...
  UT_hash_handle hh;
};

struct closure {
  LispObject *body;
  LispObject **values;
  size_t size;
  bool variadic;
};

union func_primitive {
  NativeFunc native;
  Closure closure;
};

enum func_type {
  NATIVE,
  CLOSURE,
};

struct func {
  FuncPrimitive value;
  FuncType type;
  char *name;
};

union lisp_primitive {
  Cons cons;
  Symbol symbol;
  Func func;
  long integer;
  char *string;
  CompiledBody *compiled_body;
};

enum primitive_type {
  CONS,
  SYMBOL,
  INTEGER,
  STRING,
  FUNC,
  COMPILED_BODY,
};

struct lisp_object {
  LispPrimitive value;
  PrimitiveType type;

  // GC stuff
  LispObject *nextAlloc;
  bool reachable;
  // --
};

LispObject *intern(VM *vm, char *name);
LispObject *cons(VM *vm, LispObject *car, LispObject *cdr);
LispObject *wrap_int(VM *vm, long v);
LispObject *wrap_string(VM *vm, char *string);
LispObject *wrap_native(VM *vm, NativeFunc func, char *name);
LispObject *wrap_closure(VM *vm, Closure closure, char *name);

#endif
