#include "table.h"
#include "obj.h"
#include <string.h>

#define TABLE_MAX_LOAD 0.75

void initTable(Table *table) {
	table->count = 0;
	table->size = 0;
	table->entries = NULL;
}


void freeTable(Table *table) {
	FREE_ARRAY(Entry, table->entries, table->size);
	initTable(table);
}

static Entry *findEntry(Entry *entries, size_t size, ObjString *key) {
	uint32_t index = key->hash % size;
	Entry *tombstone = NULL;
	while(1) {
		Entry *entry = entries+index;
		if(entry->key == NULL) {
			if(IS_NIL(entry->value)) {
				return tombstone != NULL ? tombstone : entry;
			} else {
				if(tombstone == NULL) tombstone = entry;
			}
		} else if(entry->key == key) {
			return entry;
		}
		index = (index + 1) % size;
	}
}

static void adjustSize(Table *table, size_t size) {
	Entry *entries = ALLOCATE(Entry, size);
	table->count = 0;
	for(size_t i=0;i<size;i++) {
		entries[i].key = NULL;
		entries[i].value = NIL_VAL;
	}
	for(size_t i=0;i<table->size;i++) {
		Entry *entry = table->entries+i;
		if(entry->key == NULL) continue;
		Entry *dest = findEntry(entries, size, entry->key);
		dest->key = entry->key;
		dest->value = entry->value;
		table->count++;
	}
	FREE_ARRAY(Entry, table->entries, table->size);
	table->entries = entries;
	table->size = size;
}

void tableAddAll(Table *from, Table *to) {
	for(size_t i=0;i<from->size;i++) {
		Entry *entry = from->entries+i;
		if(entry->key != NULL) {
			tableSet(to, entry->key, entry->value);
		}
	}
}


int tableSet(Table *table, ObjString *key, Value value) {
	if(table->count >= table->size * TABLE_MAX_LOAD) {
		size_t size = GROW_SIZE(table->size);
		adjustSize(table, size);
	}
	Entry *entry = findEntry(table->entries, table->size, key);
	int isNewKey = entry->key == NULL;
	if(isNewKey && IS_NIL(entry->value)) table->count++;
	entry->key = key;
	entry->value = value;
	return isNewKey;
}

int tableGet(Table *table, ObjString *key, Value *value) {
	if(table->count == 0) return 0;
	Entry *entry = findEntry(table->entries, table->size, key);
	if(entry->key == NULL) {
		return 0;
	}
	*value = entry->value;
	return 1;
}

int tableDel(Table *table, ObjString *key) {
	if(table->count == 0) return 0;
	Entry *entry = findEntry(table->entries, table->size, key);
	if(entry->key == NULL) return 0;
	entry->key = NULL;
	entry->value = BOOL_VAL(1);
	return 1;
}

ObjString *tableFindString(Table *table, char *string, size_t length, uint32_t hash) {
	if(table->count == 0) return NULL;
	size_t index = hash % table->size;
	while(1) {
		Entry *entry = table->entries+index;
		if(entry->key == NULL) {
			if(IS_NIL(entry->value)) return NULL;
		} else if(entry->key->length == length && hash == entry->key->hash && !memcmp(string, entry->key->chars, length)) {
			return entry->key;
		}
		index++;
		index %= table->size;
	}
}
