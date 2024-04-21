#pragma once

#include "lexer.h"
#include <stdbool.h>

bool typeBase();
bool unit();
void parse(Token *tokens);
bool structDef();
bool varDef();
bool arrayDecl();
bool fnParam();
bool fnDef();
bool structDef();
bool stm();
bool stmCompound();

bool expr();
bool exprAssign();
bool exprAdd();
bool exprAddPrim();
bool exprMul();
bool exprMulPrim();
bool exprOr();
bool exprOrPrim();
bool exprAnd();
bool exprAndPrim();
bool exprEq();
bool exprEqPrim();
bool exprCast();
bool exprRel();
bool exprRelPrim();
bool exprUnary();
bool exprPostfix();
bool exprPostfixPrim();
bool exprPrimary();
/*
*/
