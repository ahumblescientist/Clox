#include "value.h"

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
	printf("%lf", v);
}
