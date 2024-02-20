#include "debug.h"


void printChunk(Chunk *chunk) {
	printf("=============== debug ===============\n");
	size_t index = 0;
	while((index = printOpcode(chunk, index)));
}

size_t simpleOpcode(char *text) {
	printf("%s\n", text);
	return 1;
}

size_t constantOpcode(char *text, uint8_t constantIndex, Value constant) {
	printf("%s i:%02X v:", text, constantIndex);
	printValue(constant);
	printf("\n");
	return 2;
}

size_t printOpcode(Chunk *chunk, size_t index) {
	if(chunk->index <= index) {
		return 0;
	}
	printf("%04zX %zu ", index, chunk->lines[index]);
	uint8_t current = chunk->bytecode[index];
	switch(current) {
		case OP_RETURN:
			return index+simpleOpcode("OP_RETURN");
		case OP_CONSTANT: {
			uint8_t constantIndex = chunk->bytecode[index+1];
			return index+constantOpcode("OP_CONSTANT", constantIndex, chunk->constants.values[constantIndex]);
		}
		case OP_NEGATE:
			return index + simpleOpcode("OP_NEGATE");
		case OP_ADD:
			return index + simpleOpcode("OP_ADD");
		case OP_SUBTRACT:
			return index + simpleOpcode("OP_SUBTRACT");
		case OP_MULTIPLY:
			return index + simpleOpcode("OP_MULTIPLY");
		case OP_DIVIDE:
			return index + simpleOpcode("OP_DIVIDE");
		case OP_TRUE:
			return index + simpleOpcode("OP_TRUE");
		case OP_FALSE:
			return index + simpleOpcode("OP_FALSE");
		case OP_NIL:
			return index + simpleOpcode("OP_NIL");
		case OP_NOT:
			return index + simpleOpcode("OP_NOT");
		case OP_EQUAL:
			return index + simpleOpcode("OP_EQUAL");
		case OP_LESS:
			return index + simpleOpcode("OP_LESS");
		case OP_GREATER:
			return index + simpleOpcode("OP_GREATER");
		case OP_PRINT:
			return index + simpleOpcode("OP_PRINT");
		case OP_POP:
			return index + simpleOpcode("OP_POP");
		case OP_SET_GLOBAL:
			return index + simpleOpcode("OP_SET_GLOBAL");
		case OP_GET_GLOBAL:
			return index + simpleOpcode("OP_GET_GLOBAL");
		case OP_SET_LOCAL:
			return index + simpleOpcode("OP_SET_LOCAL");
		case OP_GET_LOCAL:
			return index + simpleOpcode("OP_GET_LOCAL");
		case OP_DEFINE_GLOBAL: {
			uint8_t constantIndex = chunk->bytecode[index+1];
			return index+constantOpcode("OP_DEFINE_GLOBAL", constantIndex, chunk->constants.values[constantIndex]);
		}
		default:
			printf("unknown instruction\n");
			return index+1;
	}
}
