#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include "parser.h"


Token *iTk;		   // the iterator in the tokens list
Token *consumedTk; // the last consumed token

void tkerr(const char *fmt, ...)
{
	fprintf(stderr, "error in line %d: ", iTk->line);
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

bool consume(int code)
{
	if (iTk->code == code)
	{
		consumedTk = iTk;
		iTk = iTk->next;
		return true;
	}
	return false;
}

// typeBase: TYPE_INT | TYPE_DOUBLE | TYPE_CHAR | STRUCT ID
bool typeBase()
{
	Token *start = iTk;
	if (consume(TYPE_INT))
	{
		return true;
	}
	if (consume(TYPE_DOUBLE))
	{
		return true;
	}
	if (consume(TYPE_CHAR))
	{
		return true;
	}
	if (consume(STRUCT))
	{
		if (consume(ID))
		{
			return true;
		}
	}
	iTk = start;
	return false;
}

// unit: ( structDef | fnDef | varDef )* END
// return:
// true- daca regula a fost indeplinita
// false- daca regula nu a fost indeplinita

bool unit(){
    printf("#unit: %d\n", iTk->code);
	for(;;){
		if(structDef()){}
		else if(fnDef()){}
		else if(varDef()){}
		else break;
		}
		if(consume(END)){
			return true;
		}
	return false;
}

void parse(Token *tokens)
{
	iTk = tokens;
	if (!unit())
		tkerr("syntax error");
}

// Implementarea func pentru regula structDef
bool structDef(){
    printf("#structDef: %d\n", iTk->code);
    if(consume(STRUCT)) {
        if(consume(ID)) {
            if(consume(LACC)) {
                for(;;) {
                    if(varDef()) {
                        if(consume(RACC)) {
                            if(consume(SEMICOLON)) {
                                return true;
                            }
                        }
                    }
                    else {
                        break;
                    }
                }
            }
        }
    }

    return false;
}

bool varDef(){
    printf("#varDef: %d\n", iTk->code);
    if(typeBase()){
        if(consume(ID)){
            if(arrayDecl()){
                if(consume(SEMICOLON)){
                    return true;
                }
            }
            if(consume(SEMICOLON)){
                return true;
            }
        }
    }
    return false;
}

bool arrayDecl(){
    printf("#arrayDecl: %d\n", iTk->code);
    if(consume(LBRACKET)){
        if(consume(INT)){
            if(consume(RBRACKET)){
                return true;
            }else(tkerr("lipseste ']' din array"));
        }
        if(consume(RBRACKET)){
            return true;
        }else(tkerr("lipseste ']' din  array"));
    }else(tkerr("lipseste '[' din array"));
    return false;
}


bool fnParam() {
    printf("#fnParam: %d\n", iTk->code);
    if(typeBase()) {
        if(consume(ID)){
            if(arrayDecl()) {
                return true;
            }
            return true;
        }
    }
    return false;
}

bool fnDef(){
    printf("#fnDef: %d\n", iTk->code);
    if(typeBase()) {

    }
    else if(consume(VOID)) {

    }
    if(consume(ID)) {
        if(consume(LPAR)) {
            if(fnParam()){
                for(;;) {
                    if(consume(COMMA)) {
                        if(fnParam()) {

                        }
                        else break;
                    }
                }
            }
            if(consume(RPAR)) {
                if(stmCompound()) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool structDef(){
    printf("#structDef: %d\n", iTk->code);
    if(consume(STRUCT)) {
        if(consume(ID)) {
            if(consume(LACC)) {
                for(;;) {
                    if(varDef()) {
                        if(consume(RACC)) {
                            if(consume(SEMICOLON)) {
                                return true;
                            }
                        }
                    }
                    else {
                        break;
                    }
                }
            }
        }
    }

    return false;
}

/*
stm: stmCompound
| IF LPAR expr RPAR stm ( ELSE stm )?
| WHILE LPAR expr RPAR stm
| RETURN expr? SEMICOLON
| expr? SEMICOLON
*/
bool stm() {
    printf("#stm: %d\n", iTk->code);
    if(stmCompound()){
        return true;
    }
    // | IF LPAR expr RPAR stm ( ELSE stm )?
    if(consume(IF)) {
        if(consume(LPAR)) {
            if(expr()) {
                if(consume(RPAR)) {
                    if(stm()){
                        if(consume(ELSE)) {
                            if(stm()){
                                return true;
                            }
                        }
                        return true;
                    }
                }
            }
        }
    }
    // | WHILE LPAR expr RPAR stm
    else if(consume(WHILE)) {
        if (consume(LPAR)) {
            if(expr()){
                if(consume(RPAR)) {
                    if(stm()) {
                        return true;
                    }
                }
            }
        }
    }
    // | RETURN expr? SEMICOLON
    else if(consume(RETURN)) {
        if(expr()){
            if(consume(SEMICOLON)){
                return true;
            }
        }
        if(consume(SEMICOLON)) return true;
    }
    // | expr? SEMICOLON
    else if(expr()) {
        if(consume(SEMICOLON)){
            return true;
        }
    }

    return false;
}

// stmCompound: LACC ( varDef | stm )* RACC
bool stmCompound() {
    printf("#stmCompound: %d\n", iTk->code);
    if(consume(LACC)) {
        for(;;){
            if(varDef()){}
            else if(stm()){}
            else break;
        }
        if(consume(RACC)){
            return true;
        }
    }
    return false;
}