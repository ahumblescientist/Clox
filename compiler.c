#include "compiler.h"
#include "scanner.h"
#include "chunk.h"
#include "obj.h"
#include <stdlib.h>
#include <string.h>


typedef struct {
  Token name;
  int depth;
} Local;

typedef struct {
  Local locals[256];
  int localCount;
  int scopeDepth;
} Compiler;

Compiler *current = NULL;
static void initCompiler(Compiler *compiler) {
  compiler->localCount = 0;
  compiler->scopeDepth = 0;
  current = compiler;
}

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

typedef void (*ParseFn)(int);

typedef struct {
	ParseFn prefix;
	ParseFn infix;
	Precedence precedence;
} ParseRule;

static void expression(), parsePrecedence(Precedence), grouping(int), number(int), unary(int), binary(int), literal(int),
string(int), declaration(), statement(), variable(int);
static ParseRule *getRule(TokenType);
static uint8_t identifierConstant(Token*);
static int identifiersEqual(Token *, Token*);

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
  [TOKEN_IDENTIFIER]    = {variable,     NULL,   PREC_NONE},
  [TOKEN_STRING]        = {string,     NULL,   PREC_NONE},
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
		printf("at the end");
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

static int check (TokenType t) {
  return t == parser.current.type;
}

static int match(TokenType t) {
  if(!check(t)) return 0;
  advance();
  return 1;
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

static void number(int canAssign) {
	double value = strtod(parser.prev.start, NULL);
	emitConstant(NUMBER_VAL(value));
}

static void string(int canAssign) {
  emitConstant(OBJ_VAL(copyString(parser.prev.start + 1, parser.prev.length - 2)));
}

static void unary(int canAssign) {
	TokenType operatorType = parser.prev.type;
	parsePrecedence(PREC_UNARY);
	switch(operatorType) {
		case TOKEN_MINUS: emitByte(OP_NEGATE); break;
		case TOKEN_BANG: emitByte(OP_NOT); break;
		default: break;
	}
}

static void grouping(int canAssign) {
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void binary(int canAssign) {
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

static void literal(int canAssign) {
  switch(parser.prev.type) {
    case TOKEN_FALSE: emitByte(OP_FALSE); break;
    case TOKEN_TRUE: emitByte(OP_TRUE); break;
    case TOKEN_NIL: emitByte(OP_NIL); break;
    default: break;
  }
}

static void parsePrecedence(Precedence precedence) {
	advance();
  int canAssign = precedence <= PREC_ASSIGNMENT;
	ParseFn prefixRule = getRule(parser.prev.type)->prefix;
	if(prefixRule == NULL) {
		error("Expected Expression.");
		return;
	}
	prefixRule(canAssign);
	while(precedence <= getRule(parser.current.type)->precedence) {
		advance();
		ParseFn infixRule = getRule(parser.prev.type)->infix;
		infixRule(canAssign);
	}
  if(canAssign && match(TOKEN_EQUAL)) {
    error("Invalid Assignment target.");
  }
}

static int resloveLocal(Compiler *compiler, Token *name) {
  for(int i=compiler->localCount - 1;i>=0;i--) {
    Local *local = &compiler->locals[i];
    if(identifiersEqual(name, &local->name)) {
      if(local->depth == -1) {
        error("Can't read local variable in its own initializer.");
      }
      return i;
    }
  }
  return -1;
}

static void namedVariable(Token t, int canAssign) {
  uint8_t getOp, setOp;
  int arg = resloveLocal(current, &t);
  if(arg != -1) {
    getOp = OP_GET_LOCAL;
    setOp = OP_SET_LOCAL;
  } else {
    arg = identifierConstant(&t);
    getOp = OP_GET_GLOBAL;
    setOp = OP_SET_GLOBAL;
  }
  if(canAssign && match(TOKEN_EQUAL)) {
    expression();
    emitBytes(setOp, (uint8_t)arg);
  } else {
    emitBytes(getOp, (uint8_t)arg);
  }
}

static void variable(int canAssign) {
  namedVariable(parser.prev, canAssign);
}

static void expression() {
	parsePrecedence(PREC_ASSIGNMENT);
}

static void printStatement() {
  expression();
  consume(TOKEN_SEMICOLON, "Expect ';' after value.");
  emitByte(OP_PRINT);
}

static void expressionStatement() {
  expression();
  consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
  emitByte(OP_POP);
}

static void sync() {
  parser.panicMode = 0;
  while(parser.current.type != TOKEN_EOF) {
    if(parser.prev.type  == TOKEN_SEMICOLON) return;
    switch(parser.current.type) {
      case TOKEN_CLASS:
      case TOKEN_FUN:
      case TOKEN_VAR:
      case TOKEN_FOR:
      case TOKEN_IF:
      case TOKEN_WHILE:
      case TOKEN_PRINT:
      case TOKEN_RETURN:
        return;
      default: break;
    }
    advance();
  }
}

static void beginScope() {
  current->scopeDepth++;
}

static void endScope() {
  current->scopeDepth--;
  while(current->localCount > 0 && current->locals[current->localCount-1].depth > current->scopeDepth) {
    emitByte(OP_POP);
    current->localCount--;
  }
}

static void block() {
  while(!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
    declaration();
  }
  consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

static void statement() {
  if(match(TOKEN_PRINT)) {
    printStatement();
  } else if(match(TOKEN_LEFT_BRACE)) {
    beginScope();
    block();
    endScope();
  } else {
    expressionStatement();
  }
}

static uint8_t identifierConstant(Token *t) {
  return makeConstant(OBJ_VAL(copyString(t->start, t->length)));
}

static void addLocal(Token name) {
  if(current->localCount == 255) {
    error("Too many local variables in one function");
    return;
  }
  Local *local = &current->locals[current->localCount++];
  local->name = name;
  local->depth = -1;
}

static int identifiersEqual(Token *x, Token *y) {
  if(x->length != y->length) return 0;
  if(memcmp(x->start, y->start, x->length)) return 0;
  return 1;
}

static void declareVariable() {
  if(current->scopeDepth == 0) return;
  Token *name = &parser.prev;
  for(int i=current->localCount-1;i>=0;i--) {
    Local *local = &current->locals[i];
    if(local->depth != -1 && local->depth < current->scopeDepth) {
      break;
    }
    if(identifiersEqual(name, &local->name)) {
      error("Already a variable with the same name in this scope.");
    }
  }
  addLocal(*name);
}

static uint8_t parseVariable(char *msg) {
  consume(TOKEN_IDENTIFIER, msg);
  declareVariable();
  if(current->scopeDepth > 0) return 0;
  return identifierConstant(&parser.prev);
}

static void markInitialized() {
  current->locals[current->localCount - 1].depth = current->scopeDepth;
}

static void defineVariable(uint8_t index) {
  if(current->scopeDepth > 0) {
    markInitialized();
    return;
  }
  emitBytes(OP_DEFINE_GLOBAL, index);
}

static void varDeclaration() {
  uint8_t global = parseVariable("Expect Variable Name");
  if(match(TOKEN_EQUAL)) {
    expression();
  } else {
    emitByte(OP_NIL);
  }
  consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration");
  defineVariable(global);
}

static void declaration() {
  if(match(TOKEN_VAR)) {
    varDeclaration();
  } else {
    statement();
  }
}

int compile(char *source, Chunk *chunk) {
	parser.hadError = (parser.panicMode = 0);
	compilingChunk = chunk;	
	initScanner(source);
  Compiler compiler;
  initCompiler(&compiler);
  advance();
  while(!match(TOKEN_EOF)) {
    declaration();
  }
	endCompiler();
	return !parser.hadError;
}
