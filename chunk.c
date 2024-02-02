#include "chunk.h"

void initChunk(Chunk *chunk) {
	chunk->size = 0;
	chunk->index = 0;
	chunk->bytecode = NULL;
	chunk->lines = NULL;
	initValueArray(&chunk->constants);
}


void writeChunk(Chunk *chunk, uint8_t value, size_t line) {
	if(chunk->index >= chunk->size) {
		size_t oldSize = chunk->size;
		chunk->size = GROW_SIZE(chunk->size);
		chunk->bytecode = GROW_ARRAY(uint8_t, chunk->bytecode, oldSize, chunk->size);
		chunk->lines = GROW_ARRAY(size_t, chunk->lines, oldSize, chunk->size);
	}
	chunk->bytecode[chunk->index] = value;
	chunk->lines[chunk->index++] = line;
}

void freeChunk(Chunk *chunk) {
	FREE_ARRAY(uint8_t, chunk->bytecode, chunk->size);
	FREE_ARRAY(Value, chunk->constants.values, chunk->constants.size);
	FREE_ARRAY(size_t, chunk->lines, chunk->size);
	initChunk(chunk); // reset values
}

uint8_t addConstant(Chunk *chunk, Value value) {
	writeValueArray(&chunk->constants, value);
	return chunk->constants.index - 1;
}
