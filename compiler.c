#include "compiler.h"
#include "scanner.h"
#include "chunk.h"

void compile(char *source) {
	initScanner(source);
	size_t line = 0;
	while(1) {
		Token token = scanToken();
//		if(token.line != line) {
//			line = token.line;
//			printf("%4d", (int)line);
//		} else {
//			printf("	| ");
//		}
		printf("%2d '%.*s'\n", token.type, (int)token.length, token.start);
		if(token.type == TOKEN_EOF) break;
	}
}
