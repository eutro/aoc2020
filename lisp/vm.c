#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#include "gc.h"
#include "builtins.h"

static void pushstack(StackFrame *frame, LispObject *obj) {
  if (frame == NULL) return;
  if (frame->stack_index >= frame->body->max_stack) {
    fprintf(stderr, "Attempted to overflow operand stack");
    exit(EXIT_FAILURE);
  }
  frame->stack[frame->stack_index++] = obj;
}

static LispObject *popstack(StackFrame *frame) {
  if (frame == NULL) {
    fprintf(stderr, "Attempted to pop stack of null frame");
    exit(EXIT_FAILURE);
  }
  if (frame->stack_index == 0) {
    fprintf(stderr, "Attempted to pop empty operand stack");
    exit(EXIT_FAILURE);
  }
  return frame->stack[--frame->stack_index];
}

static void putlocal(StackFrame *frame, size_t index, LispObject *value) {
  if (frame == NULL) {
    fprintf(stderr, "Attempted to store local in null frame");
    exit(EXIT_FAILURE);
  }
  if (index >= frame->body->max_locals) {
    fprintf(stderr, "Local variable (%ld) out of bounds", index);
    exit(EXIT_FAILURE);
  }
  frame->locals[index] = value;
}

static void popframe(VM *vm) {
  StackFrame *top = vm->frame;
  vm->frame = top->below;
  if (vm->frame != NULL) vm->frame->above = NULL;
  free(top->locals);
  free(top->stack);
  free(top);
}

static void pushframe(VM *vm, CompiledBody *body) {
  StackFrame *frame;
  frame = malloc(sizeof(*frame));
  frame->locals = malloc(sizeof(LispObject **) * (body->max_locals + 1));
  frame->stack = malloc(sizeof(LispObject **) * (body->max_stack + 1));
  frame->stack_index = 0;
  frame->body = body;
  frame->insn = 0;

  frame->above = NULL;
  frame->below = vm->frame;
  vm->frame = frame;
}

static void funcall(VM *vm, LispObject *fobj, LispObject **args, size_t argc) {
  ASSERT_TYPE(fobj, FUNC);
  Func *f = &fobj->value.func;
  switch (f->type) {
  case NATIVE: {
    NativeFunc nf = f->value.native;
    pushstack(vm->frame, nf(vm, args, argc));
    break;
  }
  case CLOSURE: {
    Closure *closure = &f->value.closure;
    bool recur = f->name != NULL;
    bool variadic = closure->variadic;

    size_t min_arity =
      closure->body->value.compiled_body->max_locals
      - closure->size
      - recur
      - variadic;

    if (variadic) {
      if (argc < min_arity) {
	fprintf(stderr, "Bad arity (%ld) for function %s, expected at least %ld",
		argc, f->name == NULL ? "<anonymous>" : f->name, min_arity);
	exit(EXIT_FAILURE);
      }
    } else {
      if (argc != min_arity) {
	fprintf(stderr, "Bad arity (%ld) for function %s, expected %ld",
		argc, f->name == NULL ? "<anonymous>" : f->name, min_arity);
	exit(EXIT_FAILURE);
      }
    }
    pushframe(vm, closure->body->value.compiled_body);
    size_t l = 0, j = 0, i = 0;
    if (recur) {
      putlocal(vm->frame, l++, fobj);
    }
    while (i < min_arity) {
      putlocal(vm->frame, l++, args[i++]);
    }
    if (variadic) {
      putlocal(vm->frame, l++, list(vm, &args[i++], argc - min_arity));
    }
    while (j < closure->size) {
      putlocal(vm->frame, l++, closure->values[j++]);
    }
    break;
  }
  }
}

static void advanceframe(VM *vm) {
  StackFrame *top = vm->frame;
  if (top->insn >= top->body->insn_count) {
    fprintf(stderr, "Instruction (%ld) out of bounds", top->insn);
    exit(EXIT_FAILURE);
  }
  Insn *insn = &(top->body->insns)[top->insn];
  switch (insn->op) {
  case NOOP: break;
  case REFLOCAL: {
    size_t ref = *(size_t *)insn->data;
    if (ref >= top->body->max_locals) {
      fprintf(stderr, "Local variable (%ld) out of bounds", ref);
      exit(EXIT_FAILURE);
    }
    pushstack(top, top->locals[ref]);
    break;
  }
  case REFGLOBAL:
    pushstack(top, glref(vm, popstack(top)));
    break;
  case DEFGLOBAL: {
    LispObject *value;
    gldef(vm, popstack(top), value = popstack(top));
    pushstack(top, value);
    break;
  }
  case FUNCALL: {
    size_t argc = *(size_t*)insn->data;
    LispObject *args[argc];
    for (size_t i = 1; i <= argc; ++i) {
      args[argc - i] = popstack(top);
    }
    funcall(vm, popstack(top), args, argc);
    break;
  }
  case RETFUNCALL: {
    size_t argc = *(size_t*)insn->data;
    LispObject *args[argc];
    for (size_t i = 1; i <= argc; ++i) {
      args[argc - i] = popstack(top);
    }
    LispObject *f = popstack(top);
    popframe(vm);
    funcall(vm, f, args, argc);
    break;
  }
  case RETURN: {
    pushstack(top->below, popstack(top));
    popframe(vm);
    return;
  }
  case GOTO:
    top->insn = *(size_t *)insn->data;
    return;
  case IFNULL:
    if (popstack(top) == NULL) {
      top->insn = *(size_t *)insn->data;
      return;
    }
    break;
  case STRINGCONST:
    pushstack(top, wrap_string(vm, (char *)insn->data));
    break;
  case SYMCONST:
    pushstack(top, intern(vm, (char *)insn->data));
    break;
  case INTCONST:
    pushstack(top, wrap_int(vm, *(long *)insn->data));
    break;
  case NULLCONST:
    pushstack(top, NULL);
    break;
  case FUNCONST: {
    FuncConst *fc = (FuncConst *)insn->data;
    size_t size = fc->closure_size;

    LispObject **values;
    values = malloc(sizeof(*values) * size);

    size_t i = 1;
    for (; i <= size; ++i) {
      values[size - i] = popstack(top);
    }

    Closure closure = { fc->body, values, size, fc->variadic };
    LispObject *value = wrap_closure(vm, closure, fc->name);
    pushstack(top, value);
    break;
  }
  case POP:
    popstack(top);
    break;
  default:
    fprintf(stderr, "Unrecognised instruction %d", insn->op);
    exit(EXIT_FAILURE);
  }
  top->insn++;
}

VM new_vm() {
  return (VM) { NULL, NULL, NULL, NULL };
}

LispObject *run(VM *vm, CompiledBody *body) {
  CompiledBody base = { 1, 0, NULL, 0 };
  pushframe(vm, &base);

  StackFrame *bottom = vm->frame;
  pushframe(vm, body);
  while (vm->frame != bottom) advanceframe(vm);

  LispObject *ret = popstack(bottom);
  popframe(vm);
  return ret;
}

LispObject *callfun(VM *vm, LispObject *fobj, LispObject **args, size_t argc) {
  CompiledBody base = { 1, 0, NULL, 0 };
  pushframe(vm, &base);

  StackFrame *bottom = vm->frame;
  funcall(vm, fobj, args, argc);
  while (vm->frame != bottom) advanceframe(vm);

  LispObject *ret = popstack(bottom);
  popframe(vm);
  return ret;
}

void throw(VM *vm, char *message) {
  fprintf(stderr, "Error -- %s\n", message);
  StackFrame *frame = vm->frame;
  while (frame != NULL) {
    fprintf(stderr,
	    " at %s\n",
	    frame->body->name == NULL ?
	    "UNKNOWN" :
	    frame->body->name);
    frame = frame->below;
  }
}
