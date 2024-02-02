#include "memory.h"
#include <stdlib.h>

void *reallocate(void *pointer, size_t oldSize, size_t size) {
	if(size == 0) {
		free(pointer);
		return NULL;
	}
	void *ret = realloc(pointer, size);
	if(!ret) exit(1);
	return ret;
}
