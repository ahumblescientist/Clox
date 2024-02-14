#include "obj.h"
#include "vm.h"
#include <string.h>


#define ALLOCATE_OBJ(type, objType) (type*)allocateObject(sizeof(type), objType)

void printObject(Value v) {
	switch(AS_OBJ(v)->type) {
		case OBJ_STRING: printf("%s", AS_CSTRING(v));
	}
}

static uint32_t hashString(char *chars, size_t length) {
	uint32_t hash = 2166136261u;
	for(size_t i=0;i<length;i++) {
		hash ^= (uint8_t)chars[i];
		hash *= 16777619;
	}
	return hash;
}

static struct Obj *allocateObject(size_t size, ObjType type) {
	Obj *object = (Obj*)reallocate(NULL, 0, size);
	object->type = type;
	object->next = vm.head;
	vm.head = object;
	return object;
}

static struct ObjString *allocateString(char *chars, size_t length, uint32_t hash) {
	ObjString *string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
	string->length = length;
	string->chars = chars;
	string->hash = hash;
	tableSet(&vm.strings, string, NIL_VAL);
	return string;
}

struct ObjString *copyString(char *start, size_t length) {
	uint32_t hash = hashString(start, length);
	ObjString *interned = tableFindString(&vm.strings, start, length, hash);
	if(interned != NULL) return interned;
	char *heapChars = ALLOCATE(char, length+1);
	memcpy(heapChars, start, length);
	heapChars[length] = '\0';
	return allocateString(heapChars, length, hash);
}

struct ObjString *takeString(char *start, size_t length) {
	uint32_t hash = hashString(start, length); 
	ObjString *interned = tableFindString(&vm.strings, start, length, hash);
	if(interned != NULL) {
		FREE_ARRAY(char, start, length+1);
		return interned;
	}
	return allocateString(start, length, hash); 
}
