#pragma once

#include "lexer.h"

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