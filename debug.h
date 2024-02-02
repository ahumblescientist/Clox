#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>
#include <stdio.h>
#include "chunk.h"

void printChunk(Chunk *);
size_t printOpcode(Chunk *chunk, size_t);


#endif
