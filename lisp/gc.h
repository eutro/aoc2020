#ifndef GC_H
#define GC_H

#include "vm.h"

void gc(VM *vm);
LispObject *new_obj(VM *vm, LispPrimitive *value, PrimitiveType type);
size_t count_objects(VM *vm);

#endif
