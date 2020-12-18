#include <stdio.h>

#include "vm.h"

void disassemble(CompiledBody *body) {
  printf("max_stack: %ld\n", body->max_stack);
  printf("max_locals: %ld\n", body->max_locals);
  printf("insn_count: %ld\n", body->insn_count);
  for (size_t i = 0; i < body->insn_count; ++i) {
    printf("0x%04lx: ", i);
    Insn *insn = &body->insns[i];
    switch (insn->op) {
    case NOOP: printf("NOOP\n"); break;
    case REFLOCAL: printf("REFLOCAL %ld\n", *(size_t *)insn->data); break;
    case REFGLOBAL: printf("REFGLOBAL\n"); break;
    case DEFGLOBAL: printf("DEFGLOBAL\n"); break;
    case FUNCALL: printf("FUNCALL %ld\n", *(size_t *)insn->data); break;
    case RETFUNCALL: printf("RETFUNCALL %ld\n", *(size_t *)insn->data); break;
    case RETURN: printf("RETURN\n"); break;
    case GOTO: printf("GOTO 0x%04lx\n", *(size_t *)insn->data); break;
    case IFNULL: printf("IFNULL 0x%04lx\n", *(size_t *)insn->data); break;
    case STRINGCONST: printf("STRINGCONST \"%s\"\n", (char *)insn->data); break;
    case SYMCONST: printf("SYMCONST %s\n", (char *)insn->data); break;
    case INTCONST: printf("INTCONST %ld\n", *(long *)insn->data); break;
    case NULLCONST: printf("NULLCONST\n"); break;
    case FUNCONST:
      printf("FUNCONST %ld\n", ((FuncConst *)insn->data)->closure_size);
      printf("0x%04lx ----\n", i);
      disassemble(((FuncConst *)insn->data)->body->value.compiled_body);
      printf("0x%04lx ----\n", i);
      break;
    case POP: printf("POP\n"); break;
    }
  }
}
