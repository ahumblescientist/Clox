#ifndef VALUE_H
#define VALUE_H

#include "memory.h"

typedef double Value;

typedef struct {
	size_t size;
	size_t index;
	Value *values;
} ValueArray;

void initValueArray(ValueArray *);
void writeValueArray(ValueArray *, Value);
void freeValueArray(ValueArray *);
void printValue(Value);

#endif
