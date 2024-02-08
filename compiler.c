#include "compiler.h"
#include "scanner.h"
#include "chunk.h"
#include <stdlib.h>


typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT,  // =
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >=
  PREC_TERM,        // + -
  PREC_FACTOR,      // * /
  PREC_UNARY,       // ! -
  PREC_CALL,        // . ()
  PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)();


typedef struct {
	ParseFn prefix;
	ParseFn infix;
	Precedence precedence;
} ParseRule;

static void expression(), parsePrecedence(Precedence), grouping(), number(), unary(), binary(), literal();
static ParseRule *getRule(TokenType);

typedef struct {
	Token current;
	Token prev;
	int hadError, panicMode;
} Parser;

Parser parser;

Chunk *compilingChunk;

ParseRule rules[] = {
  [TOKEN_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
  [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, 
  [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
  [TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
  [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
  [TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
  [TOKEN_BANG]          = {unary,     NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,     binary,   PREC_EQUALITY},
  [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,     binary,   PREC_EQUALITY},
  [TOKEN_GREATER]       = {NULL,     binary,   PREC_COMPARISON},
  [TOKEN_GREATER_EQUAL] = {NULL,     binary,   PREC_COMPARISON},
  [TOKEN_LESS]          = {NULL,     binary,   PREC_COMPARISON},
  [TOKEN_LESS_EQUAL]    = {NULL,     binary,   PREC_COMPARISON},
  [TOKEN_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_STRING]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {literal,     NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NIL]           = {literal,     NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_TRUE]          = {literal,     NULL,   PREC_NONE},
  [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};

static ParseRule *getRule(TokenType type) {
	return &rules[type];
}

static Chunk *currentChunk() {
	return compilingChunk;
}

static void errorAt(Token *token, char *msg) {
	if(parser.panicMode) return;
	parser.panicMode = 1;
	printf("[line %zu] Error ", token->line);
	if(token->type == TOKEN_EOF) {
		printf("at the end.");
	} else if(token->type != TOKEN_ERROR) {
		printf("at '%.*s'", (int)token->length, token->start);
	}
	printf(": %s\n", msg);
	parser.hadError = 1;
}

static void error(char *msg) {
	errorAt(&parser.prev, msg);
}

static void errorAtCurrent(char *msg) {
	errorAt(&parser.current, msg);
}

static void advance() {
	parser.prev = parser.current;
	while(1) {
		parser.current = scanToken();
		if(parser.current.type != TOKEN_ERROR) break;
		errorAtCurrent(parser.current.start);
	}
}

static void consume(TokenType type, char *msg) {
	if(parser.current.type == type) {
		advance();
		return;
	}
	errorAtCurrent(msg);
}

static void emitByte(uint8_t byte) {
	writeChunk(currentChunk(), byte, parser.prev.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2) {
	emitByte(byte1);
	emitByte(byte2);
}

static uint8_t makeConstant(Value value) {
	int index = addConstant(currentChunk(), value);
	if(index > 255) {
		error("too many constants in one chunk.");
		return 0;
	}
	return (uint8_t)index;
}

static void emitConstant(Value value) {
	emitBytes(OP_CONSTANT, makeConstant(value));
}

static void emitReturn() {
	emitByte(OP_RETURN);
}

static void endCompiler() {
	emitReturn();
}

static void number() {
	double value = strtod(parser.prev.start, NULL);
	emitConstant(NUMBER_VAL(value));
}

static void unary() {
	TokenType operatorType = parser.prev.type;
	parsePrecedence(PREC_UNARY);
	switch(operatorType) {
		case TOKEN_MINUS: emitByte(OP_NEGATE); break;
		case TOKEN_BANG: emitByte(OP_NOT); break;
		default: break;
	}
}

static void grouping() {
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void binary() {
	TokenType operatorType = parser.prev.type;
	ParseRule *rule = getRule(operatorType);
	parsePrecedence((Precedence)(rule->precedence + 1));
	switch(operatorType) {
		case TOKEN_PLUS: emitByte(OP_ADD); break;
		case TOKEN_MINUS: emitByte(OP_SUBTRACT); break;
		case TOKEN_STAR: emitByte(OP_MULTIPLY); break;
		case TOKEN_SLASH: emitByte(OP_DIVIDE); break;
		case TOKEN_BANG_EQUAL: emitBytes(OP_EQUAL, OP_NOT); break;
		case TOKEN_EQUAL_EQUAL: emitByte(OP_EQUAL); break;
		case TOKEN_LESS: emitByte(OP_LESS); break;
		case TOKEN_LESS_EQUAL: emitBytes(OP_GREATER, OP_NOT); break;
		case TOKEN_GREATER: emitByte(OP_GREATER); break;
		case TOKEN_GREATER_EQUAL: emitBytes(OP_LESS, OP_NOT); break;
		default: break;
	}
}

static void literal() {
  switch(parser.prev.type) {
    case TOKEN_FALSE: emitByte(OP_FALSE); break;
    case TOKEN_TRUE: emitByte(OP_TRUE); break;
    case TOKEN_NIL: emitByte(OP_NIL); break;
    default: break;
  }
}

static void parsePrecedence(Precedence precedence) {
	advance();
	ParseFn prefixRule = getRule(parser.prev.type)->prefix;
	if(prefixRule == NULL) {
		error("Expected Expression.");
		return;
	}
	prefixRule();
	while(precedence <= getRule(parser.current.type)->precedence) {
		advance();
		ParseFn infixRule = getRule(parser.prev.type)->infix;
		infixRule();
	}
}

static void expression() {
	parsePrecedence(PREC_ASSIGNMENT);
}

int compile(char *source, Chunk *chunk) {
	parser.hadError = (parser.panicMode = 0);
	compilingChunk = chunk;	
	initScanner(source);
	advance();
	expression();
	consume(TOKEN_EOF, "expected end of expression.");
	endCompiler();
	return !parser.hadError;
}
