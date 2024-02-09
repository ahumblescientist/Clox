#include "value.h"
#include "obj.h"
#include <string.h>

void initValueArray(ValueArray *v) {
	v->values = NULL;
	v->size = 0;
	v->index = 0;
}

void freeValueArray(ValueArray *v) {
	FREE_ARRAY(Value, v->values, v->size);
	initValueArray(v);
}

void writeValueArray(ValueArray *v, Value val) {
	if(v->size <= v->index) {
		size_t oldSize = v->size;
		v->size = GROW_SIZE(oldSize);
		v->values = GROW_ARRAY(Value, v->values, oldSize, v->size);
	}
	v->values[v->index++] = val;
}

void printValue(Value v) {
	switch(v.type) {
		case VAL_NUMBER: printf("%g", AS_NUMBER(v)); break;
		case VAL_NIL: printf("nil"); break;
		case VAL_BOOL: printf(AS_BOOL(v) ? "true": "false"); break;
		case VAL_OBJ: printObject(v); break;
	}
}

uint8_t valuesEqual(Value a, Value b) {
	if(a.type != b.type) return 0;
	switch(a.type) {
		case VAL_NUMBER: return (AS_NUMBER(a) == AS_NUMBER(b)); break;
		case VAL_BOOL: return (AS_BOOL(a) == AS_BOOL(b)); break;
		case VAL_NIL: return 1;
		case VAL_OBJ: {
			char *x = AS_CSTRING(a);
			char *y = AS_CSTRING(b);
			return ((!strcmp(x, y)) ? 1 : 0);
		}
		break;
		default: return 0;
	}
}
