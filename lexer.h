#pragma once

enum
{
	ID,
	INT,
	DOUBLE,
	STRING,
	CHAR
	// keywords
	,
	TYPE_CHAR,
	TYPE_INT,
	TYPE_DOUBLE,
	// delimiters
	COMMA,
	DOT,
	END,
	SEMICOLON,
	RETURN
	// operators
	,
	ASSIGN,
	EQUAL,
	LESS,
	LESSEQ,
	GREATEREQ,
	GREATER,
	DIV,
	ADD,
	SUB,
	AND,
	OR,
	NOT,
	NOTEQ,
	MUL,
	SPACE,
	LPAR,
	RPAR,
	LACC,
	RACC,
	LBRACKET,
	RBRACKET,
	WHILE,
	IF,
	ELSE,
	STRUCT,
	VOID,
	LINECOMMENT,
};

typedef struct Token
{
	int code; // ID, TYPE_CHAR, ...
	int line; // the line from the input file
	union
	{
		char *text; // the chars for ID, STRING (dynamically allocated)
		int i;		// the value for INT
		char c;		// the value for CHAR
		double d;	// the value for DOUBLE
	};
	struct Token *next; // next token in a simple linked list
} Token;

Token *tokenize(const char *pch);
void showTokens(const Token *tokens);
