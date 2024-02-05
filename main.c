#include <stdio.h>
#include <stdlib.h>
#include "chunk.h"
#include "debug.h"
#include "vm.h"
#include "compiler.h"

char *readFile(char *filename) {
	FILE *file = fopen(filename, "r");
	if(!file) {
		printf("unable to open file %s\n", filename);
		exit(1);
	}
	fseek(file, 0L, SEEK_END);
	size_t filesize = ftell(file);
	fseek(file, 0L, SEEK_SET);
	char *ret = (char *)malloc(filesize + 1);
	if(!ret) {
		printf("unable to allocate memory\n");
		exit(1);
	}

	size_t current = fread(ret, sizeof(char), filesize, file);
	if(current < filesize) {
		printf("failed to read file %s\n", filename);
		exit(1);
	}
	ret[filesize] = '\0';
	fclose(file);
	return ret;
}

void runFile(char *filename) {
	char *source = readFile(filename);
	InterpretResult result = interpret(source);
	free(source);
	if(result == RESULT_COMPILE_ERROR || result == RESULT_RUNTIME_ERROR) exit(1);
}

void repl() {
	char line[1024];
	// TODO make dynamic array of characters for the input
	while(1) {
		printf("> ");
		if(fgets(line, sizeof(line), stdin) == 0) {
			printf("\n");
			break;
		}
		interpret(line);
	}
}

int main(int argc, char **argv) {
	initVM();
	if(argc == 1) {
		repl();
	} else if(argc == 2) {
		runFile(argv[1]);
	} else {
		printf("usage: clox [file.lox]\n");
		return 1;
	}
	freeVM();
	return 0;
}
