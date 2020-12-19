#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "util.h"

#define RULE_COUNT 256
#define CHECK_COUNT 512

typedef struct rule Rule;

typedef struct rulejxt {
  Rule *lhs, *rhs;
} RuleJct;

typedef struct ruleidx {
  int idx;
  Rule *next;
} RuleIdx;

typedef union ruleprim {
  RuleJct jct;
  RuleIdx idx;
  char raw;
} RRule;

typedef enum rule_type { RAW, IDX, JCT } RuleType;

struct rule {
  RRule val;
  RuleType type;
};

char nextchar(char **buf) {
  char c = **buf;
  *buf = *buf + sizeof(**buf);
  return c;
}

int scanint(char **buf) {
  int n = 0;
  while ('0' <= **buf && **buf <= '9') {
    n *= 10;
    n += nextchar(buf) - '0';
  }
  return n;
}

Rule *readrule(char **buf) {
  while (nextchar(buf) != ' ');
  int idx;
  Rule *last, *ret; last = ret = malloc(sizeof(Rule));

  if (**buf == '"') {
    ret->val.raw = (*buf)[1];
    ret->type = RAW;
    *buf = &(*buf)[2];
    return ret;
  }

  ret->val.idx.idx = scanint(buf);
  ret->val.idx.next = NULL;
  ret->type = IDX;

  while (nextchar(buf) != '\n') {
    if (**buf == '|') {
      nextchar(buf);
      Rule *lhs = ret;
      ret = malloc(sizeof(Rule));
      ret->val.jct.lhs = lhs;
      ret->val.jct.rhs = readrule(buf);
      ret->type = JCT;
      return ret;
    }
    last = last->val.idx.next = malloc(sizeof(Rule));
    last->val.idx.idx = scanint(buf);
    last->val.idx.next = NULL;
    last->type = IDX;
  }
  return ret;
}

void printrule(Rule *rule, Rule **rules) {
 start:
  switch (rule->type) {
  case RAW:
    printf("%c", rule->val.raw);
    break;
  case IDX:
    printrule(rules[rule->val.idx.idx], rules);
    rule = rule->val.idx.next;
    if (rule != NULL) goto start;
    break;
  case JCT:
    printf("(");
    printrule(rule->val.jct.lhs, rules);
    printf("|");
    printrule(rule->val.jct.rhs, rules);
    printf(")");
    break;
  }
}

void printruleln(Rule *rule, Rule **rules) {
  printrule(rule, rules);
  printf("\n");
}

typedef struct rulestack_data {
  Rule *next;

  struct rulestack_data *below;
} RulestackData;

typedef struct backtrack_data {
  int idx;
  Rule *alt;
  RulestackData *rulestack;

  struct backtrack_data *below;
} BacktrackData;

int matchrule1(char *buf, Rule *rule, Rule **rules) {
  int idx = 0;
  BacktrackData *backtrack = NULL;
  RulestackData *rulestack = NULL;

 yourcomputerandfixthealgorithm:
  while (rule != NULL) {
    switch (rule->type) {
    case RAW:
      // printf("%c", rule->val.raw);
      if (buf[idx] == rule->val.raw) {
        ++idx;
        rule = rulestack->next;
        STACK_POP(rulestack);
      } else {
        STACK_POP(rulestack);
        if (backtrack == NULL) {
          // printf("!\n");
          STACK_CLEAR(rulestack);
          return false;
        } else {
          STACK_CLEAR(rulestack);
          idx = backtrack->idx;
          rule = backtrack->alt;
          rulestack = backtrack->rulestack;
          STACK_POP(backtrack);
          // printf("<\n");
          // for (int i = 0; i < idx; ++i) printf(" ");
        }
      }
      break;
    case JCT:
      STACK_PUSH(backtrack);
      backtrack->idx = idx;
      backtrack->alt = rule->val.jct.rhs;
      STACK_COPY(rulestack, backtrack->rulestack);
      rule = rule->val.jct.lhs;
      break;
    case IDX:
      STACK_PUSH(rulestack);
      rulestack->next = rule->val.idx.next;
      rule = rules[rule->val.idx.idx];
      break;
    }
  }
  while (rulestack != NULL) {
    rule = rulestack->next;
    STACK_POP(rulestack);
    goto yourcomputerandfixthealgorithm;
  }
  // printf("*\n");

  while (backtrack != NULL) {
    STACK_CLEAR(backtrack->rulestack);
    STACK_POP(backtrack);
  }
  return idx;
}

bool matchrule(char *buf, Rule *rule, Rule **rules) {
  return buf[matchrule1(buf, rule, rules)] == '\0';
}

void freerule(Rule *rule) {
 somegarbagecollectedlanguage:
  switch (rule->type) {
  case RAW:
    free(rule);
    break;
  case IDX: {
    Rule *tmp = rule->val.idx.next;
    free(rule);
    if ((rule = tmp) != NULL) goto somegarbagecollectedlanguage;
    break;
  }
  case JCT: {
    freerule(rule->val.jct.lhs);
    Rule *tmp = rule->val.jct.rhs;
    free(rule);
    rule = tmp;
    goto somegarbagecollectedlanguage;
  }
  }
}

void day19() {
  FILE *input = fopen("input/19.txt", "r");

  char *tmp, *buf = NULL;
  size_t bufsz;

  int idx;
  Rule *rules[RULE_COUNT] = {};
  while (getline(&buf, &bufsz, input) > 1) {
    assert(sscanf(buf, "%d", &idx) == 1);
    tmp = buf;
    rules[idx] = readrule(&tmp);
    free(buf);
    buf = NULL;
  }
  free(buf);

  char *checks[CHECK_COUNT];
  int len = 0;
  int i = 0;
  buf = NULL;
  while ((len = getline(&buf, &bufsz, input)) > 1) {
    buf[len - 1] = '\0';
    checks[i++] = buf;
    buf = NULL;
  }
  fclose(input);
  free(buf);
  checks[i] = NULL;
  int checkc = i;

  // printruleln(rules[0], rules);

  int matched = 0;
  for (i = 0; i < checkc; ++i) {
    // printf("%s\n", checks[i]);
    if (matchrule(checks[i], rules[0], rules)) ++matched;
  }
  printf("Matched: %d\n", matched);

  freerule(rules[8]);
  freerule(rules[11]);
  buf =
    "8: 42 | 42 8\n"
    "11: 42 31 | 42 11 31\n";
  rules[8] = readrule(&buf);
  rules[11] = readrule(&buf);

  matched = 0;
  for (i = 0; i < checkc; ++i) {
    if (matchrule(checks[i], rules[0], rules)) ++matched;
  }
  printf("Matched: %d\n", matched);

  for (i = 0; rules[i] != NULL; ++i) freerule(rules[i]);
  for (i = 0; checks[i] != NULL; ++i) free(checks[i]);
}
