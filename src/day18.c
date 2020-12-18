#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "../lisp/reader.h"
#include "../lisp/vm.h"
#include "../lisp/builtins.h"
#include "../lisp/gc.h"
#include "../lisp/repl.h"

bool evalnext(FILE *fp, long *result);

bool evalexp(FILE *fp, long *result) {
  int c = getc(fp);
  switch (c) {
  case '(':
    return evalnext(fp, result);
  case '\n':
  case -1:
    return false;
  default:
    ungetc(c, fp);
    fscanf(fp, "%ld", result);
    return true;
  }
}

bool evalnext(FILE *fp, long *result) {
  if (!evalexp(fp, result)) return false;
  int c, op;
  long next;
  while (1) {
    c = getc(fp);
    switch (c) {
    case ')':
    case '\n':
    case -1:
      return true;
    }
    op = getc(fp); getc(fp);
    assert(evalexp(fp, &next));
    switch (op) {
    case '+':
      *result += next;
      break;
    case '*':
      *result *= next;
      break;
    default:
      assert(0);
    }
  }
}

void day18() {
  FILE *input = fopen("input/18.txt", "r");

  long total = 0;
  long result;
  while (evalnext(input, &result)) total += result;
  printf("Total: %ld\n", total);

  char *tmp, *src = "(load \"lisp/core.lc\")";
  VM vm = new_vm();
  defbuiltins(&vm);
  READABLE readable = { &src, peekbuff, incbuff };
  eval(&vm, read(&vm, &readable));
  src =
    "(define (rearrange-with op delegate expr)"
    "  (let ((rearranged (map delegate (split-on (partial equal? op) expr))))"
    "    (if (cdr rearranged)"
    "        (cons op rearranged)"
    "        (car rearranged))))";
  eval(&vm, read(&vm, &readable));
    src =
      "(define (rearrange expr)"
      "  (cond"
      "    ((not (list? expr)) expr)"
      "    ((cdr expr)"
      "     (rearrange-with '* (partial rearrange-with '+ rearrange) expr))"
      "    (else (rearrange (car expr)))))";
  eval(&vm, read(&vm, &readable));

  total = 0;

  fseek(input, 0, SEEK_SET);
  char *buf = NULL;
  size_t sz;
  int len;
  while ((len = getline(&buf, &sz, input)) > 1) {
    src = malloc(sizeof(char) * 64 + sz);
    strcpy(src, "(eval (rearrange '(");
    strcat(src, buf);
    strcat(src, ")))");
    tmp = src;
    total += eval(&vm, read(&vm, &readable))->value.integer;
    free(tmp);
    free(buf);
    buf = NULL;
  }
  free(buf);
  printf("Total: %ld\n", total);

  fclose(input);

  vm.GLOBALS = NULL;
  gc(&vm);
}
