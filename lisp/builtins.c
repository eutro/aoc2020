#include "builtins.h"

#include <stdio.h>
#include <string.h>
#include "compiler.h"
#include "disassembler.h"
#include "reader.h"

LispObject *glref(VM *vm, LispObject *sym) {
  ASSERT_TYPE(sym, SYMBOL);
  LispObject *entry = assoc(vm, vm->GLOBALS, sym);
  if (entry == NULL) {
    fprintf(stderr, "Symbol (%s) is undefined", sym->value.symbol.name);
    exit(EXIT_FAILURE);
  }
  return cdr(vm, entry);
}

LispObject *gldef(VM *vm, LispObject *sym, LispObject *val) {
  ASSERT_TYPE(sym, SYMBOL);
  LispObject *entry = assoc(vm, vm->GLOBALS, sym);
  if (entry == NULL) {
    entry = cons(vm, sym, val);
    vm->GLOBALS = cons(vm, entry, vm->GLOBALS);
  } else {
    setcdr(vm, entry, val);
  }
  return NULL;
}

LispObject *pr(VM *vm, LispObject *obj) {
  if (obj == NULL) {
    printf("nil");
    return NULL;
  }
  switch (obj->type) {
  case CONS:
    printf("(");
    LispObject *next;
    Cons *cell = &obj->value.cons;
    while (1) {
      pr(vm, cell->car);
      next = cell->cdr;
      if (next == NULL) break;
      if (next->type == CONS) {
	printf(" ");
	cell = &next->value.cons;
      } else {
	printf(" . ");
	pr(vm, next);
	break;
      }    
    }
    printf(")");
    break;
  case SYMBOL:
    printf("%s", obj->value.symbol.name);
    break;
  case INTEGER:
    printf("%ld", obj->value.integer);
    break;
  case STRING:
    printf("\"%s\"", obj->value.string);
    break;
  case FUNC:
    if (obj->value.func.type == NATIVE) {
      printf("<procedure*:%s>", obj->value.func.name);
    } else {
      Closure *closure = &obj->value.func.value.closure;
      if (closure->size > 0) {
	if (obj->value.func.name == NULL) {
	  printf("<closure:{");
	} else {
	  printf("<closure:%s:{", obj->value.func.name);
	}
	for (size_t i = 0; i < closure->size; ++i) {
	  if (i > 0) printf(", ");
	  pr(vm, closure->values[i]);
	}
	printf("}>");
      } else {
	if (obj->value.func.name == NULL) {
	  printf("<procedure>");
	} else {
	  printf("<procedure:%s>", obj->value.func.name);
	}
      }
    }
    break;
  }
  return NULL;
}

LispObject *prn(VM *vm, LispObject *obj) {
  pr(vm, obj);
  printf("\n");
  return NULL;
}

LispObject *assoc(VM *vm, LispObject *alist, LispObject *key) {
  LispObject *entry;
  while (alist != NULL) {
    entry = car(vm, alist);
    if (equal(vm, car(vm, entry), key)) {
      return entry;
    }
    alist = cdr(vm, alist);
  }
  return NULL;
}

LispObject *equal(VM *vm, LispObject *a, LispObject *b) {
  if (a == b) return wrap_int(vm, 1);

  if ((a == NULL) ||
      (b == NULL) ||
      (a->type != b->type)) {
    return NULL;
  }

  switch (a->type) {
  case CONS:
    if (!equal(vm, car(vm, a), car(vm, b)) ||
	!equal(vm, cdr(vm, a), cdr(vm, b))) {
      return NULL;
    }
    break;
  case SYMBOL: return NULL;
  case INTEGER:
    if (a->value.integer != b->value.integer) {
      return NULL;
    }
    break;
  case STRING:
    if (strcmp(a->value.string, b->value.string) != 0) {
      return NULL;
    }
    break;
  default:
    return NULL;
  }

  return wrap_int(vm, 1);
}

LispObject *car(VM *vm, LispObject *obj) {
  ASSERT_TYPE(obj, CONS);
  return obj->value.cons.car;
}

LispObject *cdr(VM *vm, LispObject *obj) {
  ASSERT_TYPE(obj, CONS);
  return obj->value.cons.cdr;
}

LispObject *setcar(VM *vm, LispObject *obj, LispObject *val) {
  ASSERT_TYPE(obj, CONS);
  return obj->value.cons.car = val;
}

LispObject *setcdr(VM *vm, LispObject *obj, LispObject *val) {
  ASSERT_TYPE(obj, CONS);
  return obj->value.cons.cdr = val;
}

LispObject *apply1(VM *vm, LispObject *f, LispObject *args) {
  ASSERT_TYPE(f, FUNC);

  size_t i = 0, argc = 0;
  LispObject *arg = args;
  LispObject *cargs[argc];

  while (arg != NULL) {
    ++argc;
    arg = cdr(vm, arg);
  }

  arg = args;
  while (arg != NULL) {
    cargs[i++] = car(vm, arg);
    arg = cdr(vm, arg);
  }

  return callfun(vm, f, cargs, argc);
}

LispObject *eval(VM *vm, LispObject *expr) {
  CompiledBody *body = compile(vm, expr);
  LispObject *result = run(vm, body);
  freebody(body);
  return result;
}

LispObject *list(VM *vm, LispObject **args, size_t argc) {
  if (argc == 0) return NULL;
  LispObject *ret = cons(vm, args[0], NULL);
  LispObject *last = ret;
  for (size_t i = 1; i < argc; ++i) {
    last = setcdr(vm, last, cons(vm, args[i], NULL));
  }
  return ret;
}

LispObject *concat1(VM *vm, LispObject *a, LispObject *b) {
  if (a == NULL) return b;
  if (b == NULL) return a;
  LispObject *ret = cons(vm, car(vm, a), NULL);
  LispObject *last = ret;
  a = cdr(vm, a);
  while (a != NULL) {
    last = setcdr(vm, last, cons(vm, car(vm, a), NULL));
    a = cdr(vm, a);
  }
  setcdr(vm, last, b);
  return ret;
}

LispObject *apply(VM *vm, LispObject **args, size_t argc) {
  switch (argc) {
  case 0: return NULL;
  case 1: return apply1(vm, args[1], NULL);
  }
  apply1(vm, args[0],
	 concat1(vm,
		 list(vm, &args[1], argc - 2),
		 args[argc - 1]));
}

LispObject *nilp(VM *vm, LispObject *obj) {
  return obj == NULL ? wrap_int(vm, 1) : NULL;
}

LispObject *consp(VM *vm, LispObject *obj) {
  return obj != NULL && obj->type == CONS ? wrap_int(vm, 1) : NULL;
}

LispObject *symbolp(VM *vm, LispObject *obj) {
  return obj != NULL && obj->type == SYMBOL ? wrap_int(vm, 1) : NULL;
}

LispObject *integerp(VM *vm, LispObject *obj) {
  return obj != NULL && obj->type == INTEGER ? wrap_int(vm, 1) : NULL;
}

LispObject *stringp(VM *vm, LispObject *obj) {
  return obj != NULL && obj->type == STRING ? wrap_int(vm, 1) : NULL;
}

LispObject *funcp(VM *vm, LispObject *obj) {
  return obj != NULL && obj->type == FUNC ? wrap_int(vm, 1) : NULL;
}

LispObject *globals(VM *vm, void *ignored) {
  return vm->GLOBALS;
}

LispObject *internn(VM *vm, LispObject *name) {
  ASSERT_TYPE(name, STRING);
  return intern(vm, name->value.string);
}

LispObject *load(VM *vm, LispObject *filename) {
  ASSERT_TYPE(filename, STRING);
  FILE *stdlibfile = fopen(filename->value.string, "r");
  if (stdlibfile == NULL) return NULL;
  READABLE buff = { stdlibfile, peekc, advancec };
  LispObject *expr;
  while (expr = read(vm, &buff)) eval(vm, expr);
  fclose(stdlibfile);
  return wrap_int(vm, 1);
}

LispObject *name(VM *vm, LispObject *sym) {
  ASSERT_TYPE(sym, SYMBOL);
  return wrap_string(vm, sym->value.symbol.name);
}

LispObject *addlisp(VM *vm, LispObject **args, size_t argc) {
  long total = 0;
  for (size_t i = 0; i < argc; ++i) {
    ASSERT_TYPE(args[i], INTEGER);
    total += args[i]->value.integer;
  }
  return wrap_int(vm, total);
}

LispObject *mullisp(VM *vm, LispObject **args, size_t argc) {
  long total = 1;
  for (size_t i = 0; i < argc; ++i) {
    ASSERT_TYPE(args[i], INTEGER);
    total *= args[i]->value.integer;
  }
  return wrap_int(vm, total);
}

wrapargs(prn, nprn, 1, args[0]);
wrapargs(assoc, nassoc, 2, args[0], args[1]);
wrapargs(equal, nequal, 2, args[0], args[1]);
wrapargs(car, ncar, 1, args[0]);
wrapargs(cdr, ncdr, 1, args[0]);
wrapargs(setcar, nsetcar, 2, args[0], args[1]);
wrapargs(setcdr, nsetcdr, 2, args[0], args[1]);
wrapargs(cons, ncons, 2, args[0], args[1]);
wrapargs(pr, npr, 1, args[0]);
wrapargs(eval, neval, 1, args[0]);
wrapargs(concat1, nconcat1, 2, args[0], args[1]);

wrapargs(nilp, nnilp, 1, args[0]);
wrapargs(consp, nconsp, 1, args[0]);
wrapargs(symbolp, nsymbolp, 1, args[0]);
wrapargs(integerp, nintegerp, 1, args[0]);
wrapargs(stringp, nstringp, 1, args[0]);
wrapargs(funcp, nfuncp, 1, args[0]);

wrapargs(globals, nglobals, 0, NULL);
wrapargs(internn, nintern, 1, args[0]);
wrapargs(load, nload, 1, args[0]);
wrapargs(name, nname, 1, args[0]);

void defbuiltins(VM *vm) {
  gldef(vm, intern(vm, "__macros__"), NULL);

  defnative(assoc, nassoc);
  defnative(equal?, nequal);
  defnative(car, ncar);
  defnative(cdr, ncdr);
  defnative(set-car!, nsetcar);
  defnative(set-cdr!, nsetcdr);
  defnative(pr, npr);
  defnative(cons, ncons);
  defnative(prn, nprn);
  defnative(eval, neval);
  defnative(list, list);
  defnative(apply, apply);
  defnative(concat1, nconcat1);

  defnative(func?, nfuncp);
  defnative(string?, nstringp);
  defnative(integer?, nintegerp);
  defnative(symbol?, nsymbolp);
  defnative(cons?, nconsp);
  defnative(nil?, nnilp);

  defnative(globals, nglobals);
  defnative(intern, nintern);
  defnative(load, nload);

  defnative(name, nname);

  defnative(+, addlisp);
  defnative(*, mullisp);
}
