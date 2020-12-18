#include "compiler.h"

#include <stdio.h>
#include <assert.h>

#include "lib/utarray.h"
#include "builtins.h"
#include "gc.h"

static UT_icd insn_icd = { sizeof(Insn), NULL, NULL, NULL };

static SpecialForm getsf(char *name);

static void emit(BODY *body, Insn insn) {
  utarray_push_back(body->insns, &insn);
  ++body->insn_count;
}

static void incstack(BODY *body) {
  if (++body->stack > body->max_stack) ++body->max_stack;
}

static size_t *resolve(BODY *body, char *name) {
  Binding *binding;
  HASH_FIND_STR(body->lexicon, name, binding);
  if (binding == NULL) {
    if (body->container == NULL) return NULL;
    size_t *outer = resolve(body->container, name);
    if (outer == NULL) return NULL;
    binding = malloc(sizeof(*binding));
    binding->name = strdup(name);
    binding->value = body->binding_count + body->closure_size++;
    HASH_ADD_STR(body->lexicon, name, binding);
    emit(body->container, (Insn) { REFLOCAL, outer });
    incstack(body->container);
  }
  size_t *value = malloc(sizeof(size_t));
  *value = binding->value;
  return value;
}

static void compileinto(BODY *body, VM *vm, LispObject *expr) {
  LispObject *first;
  SpecialForm sf;
  while ((expr != NULL && expr->type == CONS) &&
	 (first = car(vm, expr)) != NULL &&
	 (first->type == SYMBOL)) {
    if (sf = getsf(first->value.symbol.name)) {
      return sf(body, vm, cdr(vm, expr));
    } else {
      LispObject *macroe;
      macroe = assoc(vm, glref(vm, intern(vm, "__macros__")), first);
      if (macroe != NULL) {
	expr = apply1(vm, cdr(vm, macroe), cdr(vm, expr));
      } else {
	break;
      }
    }
  }
  if (expr == NULL) {
    emit(body, (Insn) { NULLCONST });
    incstack(body);
    return;
  }
  switch(expr->type) {
  case CONS: {
    size_t *argc = malloc(sizeof(size_t));
    *argc = 0;
    compileinto(body, vm, car(vm, expr));
    expr = cdr(vm, expr);
    while (expr != NULL) {
      ++*argc;
      compileinto(body, vm, car(vm, expr));
      expr = cdr(vm, expr);
    }
    emit(body, (Insn) { FUNCALL, argc });
    body->stack -= *argc;
    break;
  }
  case SYMBOL: {
    size_t *binding;
    char *name = expr->value.symbol.name;
    if ((binding = resolve(body, name)) != NULL) {
      emit(body, (Insn) { REFLOCAL, binding });
      incstack(body);
    } else {
      emit(body, (Insn) { SYMCONST, strdup(name) });
      emit(body, (Insn) { REFGLOBAL });
      incstack(body);
    }
    break;
  }
  case STRING: {
    emit(body, (Insn) { STRINGCONST, strdup(expr->value.string) });
    incstack(body);
    break;
  }
  case INTEGER: {
    long *value = malloc(sizeof(long));
    *value = expr->value.integer;
    emit(body, (Insn) { INTCONST, value });
    incstack(body);
    break;
  }
  default:
    fprintf(stderr, "Cannot compile object of type: %d", expr->type);
    exit(EXIT_FAILURE);
  }
}

static void compile_if(BODY *body, VM *vm, LispObject *expr) {
  LispObject *predexp = car(vm, expr); expr = cdr(vm, expr);
  LispObject *thenexp = car(vm, expr); expr = cdr(vm, expr);
  LispObject *elseexp = car(vm, expr); expr = cdr(vm, expr);
  assert(expr == NULL);

  size_t *postthen = malloc(sizeof(size_t));
  size_t *postelse = malloc(sizeof(size_t));;
  compileinto(body, vm, predexp);
  emit(body, (Insn) { IFNULL, postthen });
  compileinto(body, vm, thenexp);
  emit(body, (Insn) { GOTO, postelse });
  *postthen = body->insn_count;
  compileinto(body, vm, elseexp);
  *postelse = body->insn_count;
  emit(body, (Insn) { NOOP });
}

static void compile_do(BODY *body, VM *vm, LispObject *expr) {
  if (expr == NULL) { emit(body, (Insn) { NULLCONST }); return; }
 emitnext:
  compileinto(body, vm, car(vm, expr));
  expr = cdr(vm, expr);
  if (expr != NULL) {
    emit(body, (Insn) { POP });
    --body->stack;
    goto emitnext;
  }
}

static void finish(BODY *body, size_t max_locals, CompiledBody *compiled) {
  emit(body, (Insn) { RETURN });

  compiled->max_locals = max_locals;
  compiled->max_stack = body->max_stack;
  compiled->insn_count = body->insn_count;
  compiled->insns = malloc(sizeof(Insn) * body->insn_count);
  
  for (size_t i = 1; i <= body->insn_count; ++i) {
    compiled->insns[body->insn_count - i] = *(Insn *)utarray_back(body->insns);
    utarray_pop_back(body->insns);
  }
  utarray_free(body->insns);
}

static void compile_lambda1(BODY *mbody, VM *vm, LispObject *expr, char *name) {
  FuncConst *fc = malloc(sizeof(FuncConst));
  fc->name = name;

  UT_array *insns = NULL;
  utarray_new(insns, &insn_icd);

  BODY body = { insns, 0, NULL, mbody, 0, 0, 0, 0, NULL };

  if (name != NULL) {
    Binding *binding = malloc(sizeof(Binding));
    binding->value = body.binding_count++;
    binding->name = strdup(name);
    HASH_ADD_STR(body.lexicon, name, binding);
  }
  LispObject *spec = car(vm, expr); expr = cdr(vm, expr);
  for (; spec != NULL && spec->type != SYMBOL; spec = cdr(vm, spec)) {
    Binding *binding = malloc(sizeof(Binding));
    binding->value = body.binding_count++;
    LispObject *sym = car(vm, spec);
    assert(sym != NULL && sym->type == SYMBOL);
    binding->name = strdup(sym->value.symbol.name);
    HASH_ADD_STR(body.lexicon, name, binding);
  }
  if (fc->variadic = (spec != NULL)) {
    Binding *binding = malloc(sizeof(Binding));
    binding->value = body.binding_count++;
    binding->name = strdup(spec->value.symbol.name);
    HASH_ADD_STR(body.lexicon, name, binding);
  }

  compile_do(&body, vm, expr);

  Binding *binding, *tmp;
  HASH_ITER(hh, body.lexicon, binding, tmp) {
    HASH_DEL(body.lexicon, binding);
    free(binding->name);
    free(binding);
  }

  fc->closure_size = body.closure_size;

  CompiledBody *compiled;
  compiled = malloc(sizeof(*compiled));
  compiled->name = strdup(name == NULL ? "<anonymous>" : name);
  finish(&body, body.binding_count + body.closure_size, compiled);
  LispPrimitive prim = { .compiled_body = compiled };
  fc->body = new_obj(vm, &prim, COMPILED_BODY);
  compiled->child = body.closures;
  compiled->sibling = mbody->closures;
  mbody->closures = fc->body;

  emit(mbody, (Insn) { FUNCONST, fc });
  incstack(mbody);
}

static void compile_lambda(BODY *mbody, VM *vm, LispObject *expr) {
  compile_lambda1(mbody, vm, expr, NULL);
}

static void compile_lambda_star(BODY *body, VM *vm, LispObject *expr) {
  LispObject *spec = car(vm, expr); expr = cdr(vm, expr);
  assert(spec != NULL && spec->type == CONS);

  LispObject *name = car(vm, spec); spec = cdr(vm, spec);
  assert(name != NULL && name->type == SYMBOL);
  char *value = strdup(name->value.symbol.name);
  compile_lambda1(body, vm, cons(vm, spec, expr), value);
}

static void compile_define(BODY *body, VM *vm, LispObject *expr) {
  LispObject *nameexpr = car(vm, expr); expr = cdr(vm, expr);
  assert(nameexpr != NULL);

  switch (nameexpr->type) {
  case SYMBOL: {
    char *value = strdup(nameexpr->value.symbol.name);
    emit(body, (Insn) { SYMCONST, value });
    incstack(body);
    if (expr == NULL) {
      emit(body, (Insn) { NULLCONST });
      incstack(body);
    } else {
      assert(cdr(vm, expr) == NULL);
      compileinto(body, vm, car(vm, expr));
    }
    emit(body, (Insn) { DEFGLOBAL });
    break;
  }
  case CONS: {
    LispObject *name = car(vm, nameexpr);
    assert(name != NULL && name->type == SYMBOL);
    char *value = strdup(name->value.symbol.name);
    emit(body, (Insn) { SYMCONST, value });
    incstack(body);
    compile_lambda1(body, vm, cons(vm, cdr(vm, nameexpr), expr), strdup(value));
    emit(body, (Insn) { DEFGLOBAL });
    break;
  }
  default:
    fprintf(stderr, "First argument to define must be a symbol or a list");
    exit(EXIT_FAILURE);
  }
}

static SpecialForm getsf(char *name) {
  if (strcmp("if", name) == 0) {
    return compile_if;
  } else if (strcmp("do", name) == 0) {
    return compile_do;
  } else if (strcmp("lambda", name) == 0) {
    return compile_lambda;
  } else if (strcmp("lambda*", name) == 0) {
    return compile_lambda_star;
  } else if (strcmp("define", name) == 0) {
    return compile_define;
  }
  return NULL;
}

CompiledBody *compile(VM *vm, LispObject *expr) {
  UT_array *insns = NULL;
  utarray_new(insns, &insn_icd);

  BODY body = { insns, 0, NULL, NULL, 0, 0, 0, 0, NULL };
  compileinto(&body, vm, expr);

  CompiledBody *compiled;
  compiled = malloc(sizeof(*compiled));
  finish(&body, 0, compiled);

  compiled->name = strdup("<root>");
  return compiled;
}

void freebody(CompiledBody *body) {
  Insn *insn;
  for (size_t i = 0; i < body->insn_count; ++i) {
    insn = &body->insns[i];
    switch (insn->op) {
    case FUNCONST: {
      FuncConst *fc = insn->data;
      if (fc->name != NULL) free(fc->name);
      // don't free the body, the GC takes care of it
    }
    case FUNCALL: case RETFUNCALL: case GOTO: case IFNULL:
    case REFLOCAL: case STRINGCONST: case SYMCONST: case INTCONST:
      free(insn->data);
    }
  }
  free(body->insns);
  free(body->name);
  free(body);
}
