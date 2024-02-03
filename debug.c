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
		default:
			printf("unknown instruction\n");
			return index+1;
	}
}
