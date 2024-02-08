#ifndef VALUE_H
#define VALUE_H

#include "memory.h"

typedef enum {
	VAL_BOOL,
	VAL_NUMBER,
	VAL_NIL,
} ValueType;

typedef struct {
	ValueType type;
	union {
		uint8_t boolean;
		double number;
	} as;
} Value;

typedef struct {
	size_t size;
	size_t index;
	Value *values;
} ValueArray;



#define BOOL_VAL(val) ((Value){VAL_BOOL, {.boolean=val}})
#define NUMBER_VAL(val) ((Value){VAL_NUMBER, {.number=val}})
#define NIL_VAL ((Value){VAL_NIL, {.number=0}})

#define IS_BOOL(v) (v.type == VAL_BOOL)
#define IS_NIL(v) (v.type == VAL_NIL)
#define IS_NUMBER(v) (v.type == VAL_NUMBER)

#define AS_NUMBER(v) ((double)(v.as.number))
#define AS_BOOL(v) (v.as.boolean)

void initValueArray(ValueArray *);
void writeValueArray(ValueArray *, Value);
void freeValueArray(ValueArray *);
void printValue(Value);

#endif
