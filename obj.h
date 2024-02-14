#ifndef OBJ_H
#define OBJ_H

#include "memory.h"
#include "value.h"

typedef enum {
	OBJ_STRING,
} ObjType;

struct Obj {
	ObjType type;
	struct Obj *next;
};

struct ObjString {
	struct Obj obj;
	size_t length;
	char *chars;
	uint32_t hash;
};

struct ObjString *copyString(char*, size_t);
struct ObjString *takeString(char*, size_t);
void printObject(Value);

static inline uint8_t isObjType(Value v, ObjType t) {
	return (IS_OBJ(v) && AS_OBJ(v)->type == t);
}

#define OBJ_TYPE(value) (AS_OBJ(value)->type)
#define IS_STRING(value) isObjType(value, OBJ_STRING)
#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)

#endif
