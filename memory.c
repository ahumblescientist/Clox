#include "memory.h"
#include "obj.h"
#include "vm.h"
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

static void freeObject(Obj *toFree)  {
	switch(toFree->type) {
		case OBJ_STRING: {
			ObjString *str = (ObjString *)toFree;
			FREE_ARRAY(char, str->chars, str->length);
			FREE(ObjString, toFree);
		}
		break;
		default: break;
	}
}

void freeObjects() {
	Obj *head = vm.head;
	while(head != NULL) {
		Obj *toFree = head;
		head = head->next;
		freeObject(toFree);
	}
}
