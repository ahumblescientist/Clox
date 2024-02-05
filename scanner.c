#include "scanner.h"
#include <string.h>

Scanner scanner;

void initScanner(char *source) {
	scanner.start = source;
	scanner.current = source;
	scanner.line = 1;
}
static Token makeToken(TokenType type) {
	Token token;
	token.type = type;
	token.start = scanner.start;
	token.length = scanner.current - scanner.start;
	token.line = scanner.line;
	return token;
}

static Token errorToken(char *msg) {
	Token token;
	token.type = TOKEN_ERROR;
	token.start = msg;
	token.length = strlen(msg);
	token.line = scanner.line;
	return token;
}

static int isAtEnd() {
	return (*scanner.current) == '\0';
}

static int isDigit(char c) {
	return (c >= '0' && c <= '9');
}

static int isAlpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static char advance() {
	return *(scanner.current++);
}

static int match(char c) {
	if(isAtEnd()) return 0;
	if(*scanner.current == c) {
		advance();
		return 1;
	}
	return 0;
}

static char peek() {
	return *scanner.current;
}

static char peekNext() {
	if(isAtEnd()) return '\0';
	return *(scanner.current+1);
}

static void skip() {
	char c;
	while(1) {
		c = peek();
		switch(c) {
			case '\n': {scanner.line++; advance();} break;
			case ' ': advance(); break;
			case '\t': advance(); break;
			case '\r': advance(); break;
			case '/': {
				if(peekNext() == '/') {
					while(peek() != '\n' && !isAtEnd()) advance();
				} else {
					return;
				}
			}
			break;
			default:
				return;
		}
	}
}

static Token string() {
	while(peek() != '"') {
		if(isAtEnd()) {
			return errorToken("unterminated string.");
		}
		if(peek() == '\n') scanner.line++;
		advance();
	}
	advance();
	return makeToken(TOKEN_STRING);
}

static Token number() {
	while(isDigit(peek())) advance();
	if(peek() == '.' && isDigit(peekNext())) {
		advance();
		while(isDigit(peek())) advance();
	}
	return makeToken(TOKEN_NUMBER);
}

static TokenType checkKeyword(int start, int length, char *check, TokenType ret) {
	if(length != scanner.current - scanner.start - start) {
		return TOKEN_IDENTIFIER;
	}
	if(memcmp(scanner.start + start, check, length) == 0) return ret;
	return TOKEN_IDENTIFIER;
}

static TokenType identifierType() {
  switch (*scanner.start) {
    case 'a': return checkKeyword(1, 2, "nd", TOKEN_AND);
    case 'c': return checkKeyword(1, 4, "lass", TOKEN_CLASS);
    case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
    case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
    case 'n': return checkKeyword(1, 2, "il", TOKEN_NIL);
    case 'o': return checkKeyword(1, 1, "r", TOKEN_OR);
    case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT);
    case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
    case 's': return checkKeyword(1, 4, "uper", TOKEN_SUPER);
    case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
    case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);	
		case 'f': {
			if(scanner.current != scanner.start+1) {
				switch(*(scanner.start + 1)) {
					case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE);
					case 'u': return checkKeyword(2, 6, "nction", TOKEN_FUN);
					case 'o': return checkKeyword(2, 1, "r", TOKEN_FOR);
				}
			}
		}
		break;
		case 't': {
			if(scanner.current != scanner.start + 1) {
				switch(*(scanner.start + 1)) {
					case 'h': return checkKeyword(2, 2, "is", TOKEN_THIS);
					case 'r': return checkKeyword(2, 2, "ue", TOKEN_TRUE);
				}
			}
		}
		break;
		default:
			break;
	}
	return TOKEN_IDENTIFIER;
}

static Token identifier() {
	while(isAlpha(peek()) || isDigit(peek())) advance();
	return makeToken(identifierType());
}



Token scanToken() {
	skip();
	scanner.start = scanner.current;
	if(isAtEnd()) return makeToken(TOKEN_EOF);
	char c = advance();
	if(isDigit(c)) return number();
	if(isAlpha(c)) return identifier();
	switch(c) {
		case '(': return makeToken(TOKEN_LEFT_PAREN);
    case ')': return makeToken(TOKEN_RIGHT_PAREN);
    case '{': return makeToken(TOKEN_LEFT_BRACE);
    case '}': return makeToken(TOKEN_RIGHT_BRACE);
    case ';': return makeToken(TOKEN_SEMICOLON);
    case ',': return makeToken(TOKEN_COMMA);
    case '.': return makeToken(TOKEN_DOT);
    case '-': return makeToken(TOKEN_MINUS);
    case '+': return makeToken(TOKEN_PLUS);
    case '/': return makeToken(TOKEN_SLASH);
    case '*': return makeToken(TOKEN_STAR);
		case '!':
			return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
		case '=':
			return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
		case '<':
			return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
		case '>':
			return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
		case '"': return string();

	}
	return errorToken("Unexpected character");
}
