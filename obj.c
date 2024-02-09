#include "obj.h"
#include "vm.h"
#include <string.h>


#define ALLOCATE_OBJ(type, objType) (type*)allocateObject(sizeof(type), objType)

void printObject(Value v) {
	switch(AS_OBJ(v)->type) {
		case OBJ_STRING: printf("%s", AS_CSTRING(v));
	}
}

static struct Obj *allocateObject(size_t size, ObjType type) {
	Obj *object = (Obj*)reallocate(NULL, 0, size);
	object->type = type;
	object->next = vm.head;
	vm.head = object;
	return object;
}

static struct ObjString *allocateString(char *chars, size_t length) {
	ObjString *string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
	string->length = length;
	string->chars = chars;
	return string;
}

struct ObjString *copyString(char *start, size_t length) {
	char *heapChars = ALLOCATE(char, length+1);
	memcpy(heapChars, start, length);
	heapChars[length] = '\0';
	return allocateString(heapChars, length);
}

struct ObjString *takeString(char *start, size_t length) {
	return allocateString(start, length); 
}
