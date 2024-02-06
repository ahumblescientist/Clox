#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>
#include <stdio.h>
#include "value.h"
#include "memory.h"

typedef enum {
	OP_CONSTANT,
	OP_NEGATE,
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_RETURN,
} Opcode;

typedef struct {
	size_t index;
	size_t size;
	uint8_t *bytecode;
	ValueArray constants;
	size_t *lines;
	size_t lineIndex;
	size_t lineSize;
} Chunk;

void initChunk(Chunk *), freeChunk(Chunk *);
void writeChunk(Chunk *, uint8_t, size_t);
int addConstant(Chunk *, Value);

#endif
