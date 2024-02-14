#ifndef VM_H
#define VM_H

#include "chunk.h"
#include "value.h"
#include "obj.h"
#include "table.h"

#define STACK_MAX 256

typedef struct {
	Chunk *chunk;
	uint8_t *ip;
	Value stack[STACK_MAX];
	Value *top;
	Obj *head;
	Table strings;
} VM; 

extern VM vm;

typedef enum {
	RESULT_OK,
	RESULT_COMPILE_ERROR,
	RESULT_RUNTIME_ERROR,
} InterpretResult;

void initVM();
void freeVM();
InterpretResult interpret(char*);

#endif
