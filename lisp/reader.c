#include "reader.h"

#include <stdio.h>
#include <stdbool.h>
#include "gc.h"
#include "builtins.h"

static void incptr(READABLE *rd) {
  rd->advance(rd->value);
}

static char peek(READABLE *rd) {
  return rd->peek(rd->value);
}

static bool is_macro(char c) {
  return
    c == '(' ||
    c == ')' ||
    c == '"' ||
    c == '#' ||
    c == ';' ||
    c == '`' ||
    c == ',' ||
    c == '\'' ||
    c == '\0';
}

static bool is_ws(char c) {
  return
    c == ' ' ||
    c == '\t' ||
    c == '\n' ||
    c == '\r';
}

static void skip_ws(READABLE *buff) {
  while (is_ws(peek(buff))) {
    incptr(buff);
  }
}

static LispObject *read_list(VM *vm, READABLE *buff) {
  incptr(buff);
  skip_ws(buff);
  if (peek(buff) == ')') {
    incptr(buff);
    return NULL;
  }
  LispObject *list = cons(vm, read(vm, buff), NULL);
  LispObject *cell = list;
  skip_ws(buff);
  while (peek(buff) != ')') {
    if (peek(buff) == '.') {
      incptr(buff);
      setcdr(vm, cell, read(vm, buff));
      skip_ws(buff);
      if (peek(buff) != ')') {
	fprintf(stderr, "Unclosed improper list");
	exit(EXIT_FAILURE);
      }
    } else {
      cell = setcdr(vm, cell, cons(vm, read(vm, buff), NULL));
      skip_ws(buff);
    }
  }
  incptr(buff);
  return list;
}

static LispObject *parse_int(VM *vm, char *token) {
  long v = strtol(token, NULL, 0);
  if (v == 0 && strcmp(token, "0") != 0) {
    return NULL;
  }
  return wrap_int(vm, v);
}

static LispObject *read_token(VM *vm, READABLE *buff) {
  size_t prevsize = 8;
  char *token = malloc(sizeof(char) * prevsize);
  size_t len = 0;
  while (!is_ws(peek(buff)) && !is_macro(peek(buff))) {
    token[len] = peek(buff);
    incptr(buff);
    ++len;
    if (len >= prevsize) {
      token = realloc(token, (prevsize <<= 1));
    }
  }
  token[len] = '\0';

  LispObject *ret = parse_int(vm, token);
  if (ret != NULL) { free(token); return ret; }

  if (strcmp("nil", token) == 0) { free(token); return NULL; }

  ret = intern(vm, token);
  free(token);
  return ret;
}

static LispObject *read_string(VM *vm, READABLE *buff) {
  incptr(buff);
  size_t prevsize = 8;
  char *string = malloc(sizeof(char) * prevsize);
  size_t len = 0;
  while (peek(buff) != '"') {
    switch (peek(buff)) {
    case '\0':
      fprintf(stderr, "Unexpected EOF");
      exit(EXIT_FAILURE);
      break;
    default:
      string[len] = peek(buff);
      incptr(buff);
      ++len;
      if (len >= prevsize) {
	string = realloc(string, (prevsize <<= 1));
      }
    }
  }
  string[len] = '\0';
  incptr(buff);
  LispObject *ret = wrap_string(vm, string);
  free(string);
  return ret;
}

LispObject *read(VM *vm, READABLE *buff) {
  skip_ws(buff);
  switch (peek(buff)) {
  case '(':
    return read_list(vm, buff);
  case ')':
    fprintf(stderr, "Unmatched delimiter");
    exit(EXIT_FAILURE);
  case '"':
    return read_string(vm, buff);
  case ';': {
    char c;
    incptr(buff);
    while ((c = peek(buff)) != '\n' && c != '\0') incptr(buff);
    return read(vm, buff);
  }
  case '#': {
    incptr(buff);
    assert(peek(buff) == ';');
    incptr(buff);
    read(vm, buff);
    return read(vm, buff);
  }
  case ',': {
    incptr(buff);
    bool splice = peek(buff) == '@';
    if (splice) incptr(buff);
    return cons(vm,
		intern(vm,
		       splice ?
		       "unquote-splicing" :
		       "unquote"),
		cons(vm, read(vm, buff), NULL));
  }
  case '`' :
    incptr(buff);
    return cons(vm,
		intern(vm, "quasiquote"),
		cons(vm, read(vm, buff), NULL));
  case '\'' :
    incptr(buff);
    return cons(vm,
		intern(vm, "quote"),
		cons(vm, read(vm, buff), NULL));
  case '\0':
    fprintf(stderr, "Unexpected EOF");
    exit(EXIT_FAILURE);
  default:
    return read_token(vm, buff);
  }
}

char peekc(FILE *fp) {
  int c;
  ungetc((c = getc(fp)), fp);
  return c == EOF ? '\0' : (char) c;
}

void advancec(FILE *fp) {
  getc(fp);
}

char peekbuff(char **buff) {
  return **buff;
}

void incbuff(char **buff) {
  *buff += sizeof(**buff);
}
