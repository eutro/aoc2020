#include <stdio.h>
#include <assert.h>

#include "gc.h"
#include "builtins.h"
#include "reader.h"
#include "compiler.h"
#include "disassembler.h"

void repl(VM *vm) {
  READABLE buff = { stdin, peekc, advancec };
  LispObject *expr;
  while (1) {
    printf("> ");
    expr = read(vm, &buff);
    if (expr == NULL) return;
    prn(vm, eval(vm, expr));
    gc(vm);
  }
}

/*
int main() {
  VM vm = new_vm();
  defbuiltins(&vm);
  repl(&vm);
  vm.GLOBALS = NULL;
  gc(&vm);
  assert(count_objects(&vm) == 0);
  return 0;
}
*/
