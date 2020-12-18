#ifndef VM_H
#define VM_H

#include "lisp.h"

typedef struct stack_frame StackFrame;
typedef struct insn Insn;
typedef struct compiled_body CompiledBody;
typedef struct func_const FuncConst;
typedef enum opcode Opcode;

struct vm {
  StackFrame *frame;

  LispObject *OBJECTS;

  LispObject *GLOBALS;

  Symbol *SYMBOLS;
};

struct compiled_body {
  size_t max_stack;
  size_t max_locals;
  Insn *insns;
  size_t insn_count;
  char *name;
  LispObject *child;
  LispObject *sibling;
};

struct stack_frame {
  LispObject **locals;
  LispObject **stack;
  size_t stack_index;

  CompiledBody *body;
  size_t insn;

  StackFrame *above;
  StackFrame *below;
};

struct func_const {
  LispObject *body;
  size_t closure_size;
  char *name;
  bool variadic;
};

enum opcode {
  NOOP,
  REFLOCAL,
  REFGLOBAL,
  DEFGLOBAL,
  FUNCALL,
  RETFUNCALL,
  RETURN,
  GOTO,
  IFNULL,
  STRINGCONST,
  SYMCONST,
  INTCONST,
  NULLCONST,
  FUNCONST,
  POP,
};

struct insn {
  Opcode op;
  void *data;
};

VM new_vm();
LispObject *run(VM *vm, CompiledBody *body);
LispObject *callfun(VM *vm, LispObject *fobj, LispObject **args, size_t argc);
void throw(VM *vm, char *message);

#define ASSERTVM(EXPR, MESSAGE)			\
  do {						\
    if (!(EXPR)) {				\
      throw(vm, MESSAGE);			\
      exit(EXIT_FAILURE);			\
    }						\
  } while (0);

#define ASSERT_TYPE(OBJ, TYPE)				\
  do {							\
    ASSERTVM(OBJ != NULL && OBJ->type == TYPE,		\
	     "Bad type, expected " #TYPE		\
	     "\n at " __FILE__);			\
  } while (0)

#endif
