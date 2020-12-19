#ifndef UTIL_H
#define UTIL_H

#include <assert.h>
#include <stdlib.h>

#define STACK_POP_WITH(STACK, BELOW)            \
  do {                                          \
    assert(STACK != NULL);                      \
    __typeof__(STACK) __stack_tmp = STACK;      \
    STACK = STACK->BELOW;                       \
    free(__stack_tmp);                          \
  } while (0)

#define STACK_POP(STACK) STACK_POP_WITH(STACK, below)

#define STACK_PUSH_WITH(STACK, BELOW)                           \
  do {                                                          \
    __typeof__(STACK) __new_stack = malloc(sizeof(*STACK));     \
    __new_stack->BELOW = STACK;                                 \
    STACK = __new_stack;                                        \
  } while (0)

#define STACK_PUSH(STACK) STACK_PUSH_WITH(STACK, below);

#define STACK_COPY_WITH(STACK, BELOW, TO)                               \
  if (STACK == NULL) {                                                  \
    TO = NULL;                                                          \
  } else {                                                              \
    __typeof__(STACK) __bottom_stack = TO = malloc(sizeof(*STACK));     \
    __typeof__(STACK) __bottom_stack_below;                             \
    *__bottom_stack = *STACK;                                           \
    while (__bottom_stack->BELOW != NULL) {                             \
      __bottom_stack_below = malloc(sizeof(*STACK));                    \
      *__bottom_stack_below = *__bottom_stack->BELOW;                   \
      __bottom_stack->BELOW = __bottom_stack_below;                     \
      __bottom_stack = __bottom_stack_below;                            \
    }                                                                   \
  }

#define STACK_COPY(STACK, TO) STACK_COPY_WITH(STACK, below, TO)

#define STACK_CLEAR_WITH(STACK, BELOW) while (STACK != NULL) STACK_POP_WITH(STACK, BELOW)

#define STACK_CLEAR(STACK) STACK_CLEAR_WITH(STACK, below);

#endif
