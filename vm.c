#include "vm.h"
#include "debug.h"
#include "compiler.h"
#include <stdarg.h>
#include "value.h"
#include "obj.h"
#include <string.h>

VM vm;

#define DEBUG_ENABLED

static void push(Value v) {
	*(vm.top++) = v;
}

static Value pop() {
	return *(--vm.top);
}

static Value peek(int d) {
	return *(vm.top-d-1);
}


void resetStack() {
	vm.top = &vm.stack[0];
}

void initVM() {
	vm.head = NULL;
	resetStack();
}

void freeVM() {
	freeObjects();
}

static void runtimeError(char *format, ...)  {
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
	size_t instruction = vm.ip - vm.chunk->bytecode - 1;
	size_t line = vm.chunk->lines[instruction];
	printf("[line %zu] in script\n", line);
	resetStack();
}

static uint8_t isFalsey(Value v) {
	if(v.type == VAL_NIL) return 1;
	if(v.type != VAL_BOOL) return 0;
	return !AS_BOOL(v);
}

static void concatenate() {
	ObjString *y = AS_STRING(pop());
	ObjString *x = AS_STRING(pop());
	size_t length = y->length + x->length;
	char *chars = ALLOCATE(char, length+1);
	memcpy(chars, x->chars, x->length);
	memcpy(chars+x->length, y->chars, y->length);
	chars[length] = '\0';
	ObjString *ret = takeString(chars, length);
	push(OBJ_VAL(ret));
}

static uint8_t read() {
	return *(vm.ip++);
}

Value readConstant() {
	return vm.chunk->constants.values[read()];
}

InterpretResult run() {
#define BINARY_OP(val, operator) do {\
	if(!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {\
		runtimeError("operands must be of type number\n");\
		return RESULT_RUNTIME_ERROR;\
	}\
	double x = AS_NUMBER(pop());\
	double y = AS_NUMBER(pop());\
	push(val(y operator x));\
} while(0)
	while(1) {
#ifdef DEBUG_ENABLED
		printf("=====stack====\n");
		for(Value *slot=vm.stack; slot < vm.top;slot++) {
			printf("[ ");
			printValue(*slot);
			printf(" ]");
		}
		printf("\n");
		printf("=====stack====\n");
		printOpcode(vm.chunk, (size_t)(vm.ip - vm.chunk->bytecode));
#endif
		uint8_t instruction = read();
		switch(instruction) {
			case OP_RETURN:
				return RESULT_OK;
			case OP_CONSTANT: push(readConstant()); break;
			case OP_NEGATE:
				if(!IS_NUMBER(peek(0))) {
					runtimeError("Operand must be a number");
					return RESULT_RUNTIME_ERROR;
				}
				push(NUMBER_VAL(-AS_NUMBER(pop())));
				break;	
			case OP_ADD: {
				if(IS_STRING(peek(0)) && IS_STRING(peek(1))) {
					concatenate(); 
				} else if(IS_NUMBER(peek(0)) && IS_NUMBER(peek(0))) {
					BINARY_OP(NUMBER_VAL, +);
				} else {
					runtimeError("Operands must be two numbers or two strnigs.");
					return RESULT_RUNTIME_ERROR;
				}
			}
			break;
			case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
			case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
			case OP_DIVIDE: BINARY_OP(NUMBER_VAL, /); break;
			case OP_TRUE: push(BOOL_VAL(1)); break;
			case OP_FALSE: push(BOOL_VAL(0)); break;
			case OP_NIL: push(NIL_VAL); break;
			case OP_NOT: push(BOOL_VAL(isFalsey(pop()))); break;
			case OP_EQUAL: {
				Value y = pop();
				Value x = pop();
				push(BOOL_VAL(valuesEqual(x, y)));
			}
			break;
			case OP_LESS: BINARY_OP(BOOL_VAL, <); break;
			case OP_GREATER: BINARY_OP(BOOL_VAL, >); break;
		}
	}
}

InterpretResult interpret(char *source) {
	Chunk chunk;
	initChunk(&chunk);
	if(!compile(source, &chunk)) {
		freeChunk(&chunk);
		return RESULT_COMPILE_ERROR;
	}
	vm.chunk = &chunk;
	vm.ip = chunk.bytecode;
	InterpretResult result = run();
	freeChunk(&chunk);
	return RESULT_OK;
}
