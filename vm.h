#ifndef VM_H
#define VM_H

#include "chunk.h"
#include "value.h"

#define STACK_MAX 256

typedef struct {
	Chunk *chunk;
	uint8_t *ip;
	Value stack[STACK_MAX];
	Value *top;
} VM; 

typedef enum {
	RESULT_OK,
	RESULT_COMPILE_ERROR,
	RESULT_RUNTIME_ERROR,
} InterpretResult;

void initVM();
void freeVM();
InterpretResult interpret(char*);
void push(Value);
Value pop();

#endif
