#include <stdio.h>
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main() {
	initVM();
	Chunk chunk;
	initChunk(&chunk);
	int constant = addConstant(&chunk, 1);
  writeChunk(&chunk, OP_CONSTANT, 123);
  writeChunk(&chunk, constant, 123);

  constant = addConstant(&chunk, 3);
  writeChunk(&chunk, OP_CONSTANT, 123);
  writeChunk(&chunk, constant, 123);
  writeChunk(&chunk, OP_SUBTRACT, 123);

  writeChunk(&chunk, OP_RETURN, 123);
	interpret(&chunk);
	freeVM();
	freeChunk(&chunk);
	return 0;
}
