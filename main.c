#include <stdio.h>
#include "chunk.h"
#include "debug.h"

int main() {
	Chunk chunk;
	initChunk(&chunk);
	writeChunk(&chunk, OP_CONSTANT, 123);
	writeChunk(&chunk, addConstant(&chunk, 69), 22);
	writeChunk(&chunk, OP_RETURN, 11);
	printChunk(&chunk);
	freeChunk(&chunk);
	return 0;
}
