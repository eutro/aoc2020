#include "vm.h"
#include "compiler.h"

static void freeobj(VM *vm, LispObject *obj) {
  switch (obj->type) {
  case SYMBOL: {
    Symbol *sym = &obj->value.symbol;
    HASH_DEL(vm->SYMBOLS, sym);
    free(sym->name);
    break;
  }
  case STRING:
    free(obj->value.string);
    break;
  case FUNC: {
    if (obj->value.func.type == CLOSURE) {
      Closure *closure = &obj->value.func.value.closure;
      free(closure->values);
    }
    if (obj->value.func.name != NULL) free(obj->value.func.name);
    break;
  }
  case COMPILED_BODY:
    freebody(obj->value.compiled_body);
    break;
  }
  free(obj);
}

static void markref(LispObject *obj) {
  if (obj == NULL || obj->reachable) return;
  obj->reachable = 1;
  switch (obj->type) {
  case CONS:
    markref(obj->value.cons.car);
    markref(obj->value.cons.cdr);
    break;
  case FUNC:
    if (obj->value.func.type == CLOSURE) {
      Closure *closure = &obj->value.func.value.closure;
      for (size_t i = 0; i < closure->size; ++i) {
	markref(closure->values[i]);
      }
      markref(closure->body);
    }
    break;
  case COMPILED_BODY:
    markref(obj->value.compiled_body->child);
    markref(obj->value.compiled_body->sibling);
    break;
  }
}

void gc(VM *vm) {
  markref(vm->GLOBALS);
  StackFrame *frame = vm->frame;
  size_t i;
  while (frame != NULL) {
    for (i = 0; i < frame->body->max_stack; ++i) {
      markref(frame->stack[i]);
    }
    for (i = 0; i < frame->body->max_locals; ++i) {
      markref(frame->locals[i]);
    }
    frame = frame->below;
  }

  LispObject *obj = vm->OBJECTS;
  LispObject *next;
  vm->OBJECTS = NULL;

  while (obj != NULL) {
    next = obj->nextAlloc;
    if (obj->reachable) {
      obj->nextAlloc = vm->OBJECTS;
      vm->OBJECTS = obj;
      obj->reachable = false;
    } else {
      freeobj(vm, obj);
    }
    obj = next;
  }
}

LispObject *new_obj(VM *vm, LispPrimitive *value, PrimitiveType type) {
  LispObject *new = malloc(sizeof(LispObject));
  new->value = *value;
  new->type = type;

  new->nextAlloc = vm->OBJECTS;
  new->reachable = false;
  vm->OBJECTS = new;
  return new;
}

size_t count_objects(VM *vm) {
  size_t count = 0;
  LispObject *obj = vm->OBJECTS;
  while (obj != NULL) {
    ++count;
    obj = obj->nextAlloc;
  }
  return count;
}
