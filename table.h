#ifndef TABLE_H
#define TABLE_H

#include <stdio.h>
#include "value.h"

typedef struct {
	ObjString *key;
	Value value;
} Entry;

typedef struct {
	size_t count;
	size_t size;
	Entry *entries;
} Table;

void initTable(Table *);
void freeTable(Table *);
int tableSet(Table *, ObjString *, Value);
int tableGet(Table *, ObjString *, Value *);
int tableDel(Table *, ObjString *);
void tableAddAll(Table *, Table *);
ObjString *tableFindString(Table *, char*, size_t, uint32_t);

#endif
