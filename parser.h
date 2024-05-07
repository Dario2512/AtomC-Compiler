#pragma once

#include <stdbool.h>
#include "lexer.h"
#include "ad.h"

bool unit();
bool structDef();
bool varDef();
bool typeBase();
bool arrayDecl(Type *t);
bool stm();
bool stmCompound(bool dom);
bool fnDef();
bool exprAdd();
bool exprRelPrim();
bool exprRel();
bool exprAssign();
bool exprEq();
bool exprEqPrim();
bool exprAndPrim();
bool exprAnd();
bool exprOr();
bool exprOrPrim();
bool fnParam();
bool expr();
bool exprCast();
bool exprUnary();
bool exprPostfix();
bool exprPostfixPrim();
bool exprPrimary();
bool exprAddPrim();
bool exprMul();
bool exprMulPrim();
