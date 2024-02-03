#include "vm.h"
#include "debug.h"

VM vm;

#define DEBUG_ENABLED
// #undef DEBUG_ENABLED

void push(Value v) {
	*(vm.top++) = v;
}

Value pop() {
	return *(--vm.top);
}

void resetStack() {
	vm.top = &vm.stack[0];
}

void initVM() {
	resetStack();
}

void freeVM() {
}

uint8_t read() {
	return *(vm.ip++);
}

Value readConstant() {
	return vm.chunk->constants.values[read()];
}

InterpretResult run() {
#define BINARY_OP(operator) do {\
	Value x = pop();\
	Value y = pop();\
	push(y operator x);\
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
			case OP_NEGATE: push(-pop()); break;
			case OP_ADD: BINARY_OP(+); break;
			case OP_SUBTRACT: BINARY_OP(-); break;
			case OP_MULTIPLY: BINARY_OP(*); break;
			case OP_DIVIDE: BINARY_OP(/); break;
		}
	}
}

InterpretResult interpret(Chunk *chunk) {
	vm.chunk = chunk;
	vm.ip = chunk->bytecode;
	return run();
}
