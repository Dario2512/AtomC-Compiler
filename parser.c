#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "parser.h"
#include "ad.h"
#include "lexer.h"
#include "at.h"

Token *iTk;		// the iterator in the tokens list
Token *consumedTk;		// the last consumed token
Symbol *owner;


void tkerr(const char *fmt,...){
    fprintf(stderr,"error in line %d: ",iTk->line);
    va_list va;
    va_start(va,fmt);
    vfprintf(stderr,fmt,va);
    va_end(va);
    fprintf(stderr,"\n");
    exit(EXIT_FAILURE);
}

char *tkCodeName(int code) {
    switch(code) {
        case ID:
            return "ID";
        case TYPE_INT:
            return "TYPE_INT";
        case TYPE_CHAR:
            return "TYPE_CHAR";
        case TYPE_DOUBLE:
            return "TYPE_DOUBLE";
        case ELSE:
            return "ELSE";
        case IF:
            return "IF";
        case RETURN:
            return "RETURN";
        case STRUCT:
            return "STRUCT";
        case VOID:
            return "VOID";
        case WHILE:
            return "WHILE";
        case COMMA:
            return "COMMA";
        case SEMICOLON:
            return "SEMICOLON";
        case LPAR:
            return "LPAR";
        case RPAR:
            return "RPAR";
        case LBRACKET:
            return "LBRACKET";
        case RBRACKET:
            return "RBRACKET";
        case LACC:
            return "LACC";
        case RACC:
            return "RACC";
        case END:
            return "END";
        case ADD:
            return "ADD";
        case MUL:
            return "MUL";
        case DIV:
            return "DIV";
        case DOT:
            return "DOT";
        case AND:
            return "AND";
        case OR:
            return "OR";
        case NOT:
            return "NOT";
        case NOTEQ:
            return "NOTEQ";
        case LESS:
            return "LESS";
        case LESSEQ:
            return "LESSEQ";
        case GREATER:
            return "GREATER";
        case GREATEREQ:
            return "GREATEREQ";
        case ASSIGN:
            return "ASSIGN";
        case EQUAL:
            return "EQUAL";
        case SUB:
            return "SUB";
        case INT:
            return "INT";
        case DOUBLE:
            return "DOUBLE";
        case CHAR:
            return "CHAR";
        case STRING:
            return "STRING";
        default:
            return "UNIDENTIFIED";
    }
}

bool consume(int code)
{
  //  printf("consume(%s)",tkCodeName(code));
    if(iTk->code==code)
    {
        consumedTk=iTk;
        iTk=iTk->next;
     //   printf(" => consumed\n");
        return true;
    }
   // printf("=> found %s\n", tkCodeName(iTk->code));
    return false;
}



bool typeBase(Type *t){
    t->n=-1;
    Token *start = iTk;
    if(consume(TYPE_INT)){
        t->tb=TB_INT;
        return true;
    }
    if(consume(TYPE_DOUBLE)){
        t->tb=TB_DOUBLE;
        return true;
    }
    if(consume(TYPE_CHAR)){
        t->tb=TB_CHAR;
        return true;
    }
    if(consume(STRUCT)){
        if(consume(ID)){
            Token *tkName = consumedTk;
            t->tb=TB_STRUCT;
            t->s=findSymbol(tkName->text);
            if(!t->s)tkerr("undefined struct!");
            return true;
        }else tkerr("Missing structure name!");
    }
    iTk=start;
    return false;
}


bool arrayDecl(Type *t){
    Token *start = iTk;
    if(consume(LBRACKET)){
        if(consume(INT)){
            Token *tkSize = consumedTk;
            t->n = tkSize->i; //array dimension is equal to the int in the array declaration
        } else {
            t->n=0; //no dimension array
        }
        if(consume(RBRACKET)){
            return true;
        }else tkerr("Missing ] or invalid expression inside [...]");
    }
    iTk = start;
    return false;
}


bool varDef(){
    Token *start = iTk;
    Type t;
    if(typeBase(&t)){
        if(consume(ID)){
            Token *tkName=consumedTk;
            if(arrayDecl(&t)){
                if(t.n==0)tkerr("array size missing");
            }
            if(consume(SEMICOLON)){
                Symbol *var = findSymbolInDomain(symTable, tkName->text);
                if(var)tkerr("name already exists!");
                var = newSymbol(tkName->text, SK_VAR);
                var->type = t;
                var->owner = owner;
                addSymbolToDomain(symTable, var);
                if(owner){
                    switch(owner->kind){
                        case SK_FN:
                        var->varIdx=symbolsLen(owner->fn.locals);
                        addSymbolToList(&owner->fn.locals,dupSymbol(var));
                        break;
                        case SK_STRUCT:
                        var->varIdx=typeSize(&owner->type);
                        addSymbolToList(&owner->structMembers,dupSymbol(var));
                        break;
                    }
                }else{
                    var->varMem=safeAlloc(typeSize(&t));
                }
                return true;
            }else tkerr("Missing ; !!");
        }else tkerr("Missing identifier from variable declaration");
    }
    iTk = start;
    return false;
}


bool structDef(){
    Token *start = iTk;
    if(consume(STRUCT)) {
        if(consume(ID)) {
            Token *tkName = consumedTk;
            if(consume(LACC)) {
                Symbol *s=findSymbolInDomain(symTable, tkName->text);
                if(s)tkerr("Structure name is not unique!");
                s=addSymbolToDomain(symTable,newSymbol(tkName->text,SK_STRUCT));
                s->type.tb = TB_STRUCT;
                s->type.s = s;
                s->type.n = -1;
                pushDomain();
                owner = s;
                while(1) {
                    if(varDef()) {
                    }
                    else break;
                }
                if(consume(RACC)) {
                    if(consume(SEMICOLON)) {
                        owner=NULL;
                        dropDomain();
                        return true;
                    }else tkerr("Missing ; after struct define!");
                }else tkerr("Missing } from structure define!");
            }

        }
    }

    iTk = start;
    return false;
}


bool fnParam() {
    Token *start = iTk;
    Type t;
    if(typeBase(&t)) {
        if(consume(ID)){
            Token *tkName = consumedTk;
            if(arrayDecl(&t)) {
                t.n=0;
            }
            Symbol *param=findSymbolInDomain(symTable,tkName->text);
            if(param)tkerr("symbol redefinition: %s",tkName->text);
            param=newSymbol(tkName->text,SK_PARAM);
            param->type=t;
            param->owner=owner;
            param->paramIdx=symbolsLen(owner->fn.params);
            addSymbolToDomain(symTable,param);
            addSymbolToList(&owner->fn.params,dupSymbol(param));
            return true;
        } else tkerr("Missing identifier in funcion parameter");
    }
    iTk = start;
    return false;
}


bool exprOrPrim(Ret *r) {
    Token *start = iTk;

    if (consume(OR)) {
        Ret right;
        if (exprAnd(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) {
                tkerr("Invalid operand type for ||");
            }
            *r = (Ret){{TB_INT, NULL, -1}, false, true};
            if (exprOrPrim(r)) {
                return true;
            }
        } else {
            tkerr("Missing expression after || operator");
        }
    }

    iTk = start;
    return true; // epsilon
}

bool exprOr(Ret *r) {
    Token *start = iTk;
    if(exprAnd(r)) {
        if(exprOrPrim(r)) {
            return true;
        }
    }
    iTk = start;
    return false;
}


bool exprAnd(Ret *r) {
    Token *start = iTk;
    if(exprEq(r)) {
        if(exprAndPrim(r)) {
            return true;
        }
    }
    iTk = start;
    return false;
}

bool exprAndPrim(Ret *r) {
    Token *start = iTk;

    if (consume(AND)) {
        Ret right;
        if (exprEq(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) {
                tkerr("invalid operand type for &&");
            }
            *r = (Ret){{TB_INT, NULL, -1}, false, true};
            if (exprAndPrim(r)) {
                return true;
            } else {
                tkerr("Invalid expression after == operator");
            }
        } else {
            tkerr("Missing expression after && operator");
        }
    }

    iTk = start;
    return true;
}

bool exprEq(Ret *r) {
    Token *start = iTk;
    if(exprRel(r)) {
        if(exprEqPrim(r)) {
            return true;
        }
    }
    iTk = start;
    return false;
}

bool exprEqPrim(Ret *r) {
    Token *start = iTk;
    if(consume(EQUAL)) {
        Ret right;
        if(exprRel(&right)) {
            Type tDst;
            if(!arithTypeTo(&r->type,&right.type,&tDst)) {
                tkerr("invalid operand type for == or!=");
            }
            if(exprEqPrim(r)) {
                return true;
            }
        } else tkerr("Missing expression after ==");
    }
    else if(consume(NOTEQ)) {
        Ret right;
        if(exprRel(&right)) {
            Type tDst;
            if(!arithTypeTo(&r->type,&right.type,&tDst)) {
                tkerr("invalid operand type for == or!=");
            }
            if(exprEqPrim(r)) {
                return true;
            }
        } else tkerr("Missing expression after ==");
    }
    iTk = start;
    return true;
}


bool exprAssign(Ret *r) {
    Token *start = iTk;
    Ret rDst;
    if(exprUnary(&rDst)) {
        if(consume(ASSIGN)) {
            if(exprAssign(r)){
                {
                    if(!rDst.lval)tkerr("the assign destination must be a left-value");
                    if(rDst.ct)tkerr("the assign destination cannot be constant");
                    if(!canBeScalar(&rDst))tkerr("the assign destination must be scalar");
                    if(!canBeScalar(r))tkerr("the assign source must be scalar");
                    if(!convTo(&r->type,&rDst.type))tkerr("the assign source cannot be converted todestination");
                    r->lval=false;
                    r->ct=true;
                    return true;
                }
            } else tkerr("Missing expression after =");
        }
    }
    iTk = start;
    if(exprOr(r)) {
        return true;
    }
    iTk = start;
    return false;
}


bool exprRel(Ret *r) {
    Token *start = iTk;
    if(exprAdd(r)) {
        if(exprRelPrim(r)) {
            return true;
        }
    }
    iTk = start;
    return false;
}

bool exprRelPrim(Ret *r) {
    Token *start = iTk;
    if(consume(LESS)) {
        Ret right;
        if(exprAdd(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) {
                tkerr("Invalid operand type for relational operator");
            }
            *r = (Ret){{TB_INT, NULL, -1}, false, true};
            if(exprRelPrim(r)) {
                return true;
            }
        }  else tkerr("Missing expression after <");
    }
    else if(consume(LESSEQ)) {
        Ret right;
        if(exprAdd(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) {
                tkerr("Invalid operand type for relational operator");
            }
            *r = (Ret){{TB_INT, NULL, -1}, false, true};
            if(exprRelPrim(r)) {
                return true;
            }
        } else tkerr("Missing expression after <=");
    }
    else if(consume(GREATER)) {
        Ret right;
        if(exprAdd(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) {
                tkerr("Invalid operand type for relational operator");
            }
            *r = (Ret){{TB_INT, NULL, -1}, false, true};
            if(exprRelPrim(r)) {
                return true;
            }
        }else tkerr("Missing expression after >");
    }
    else if(consume(GREATEREQ)) {
        Ret right;
        if(exprAdd(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) {
                tkerr("Invalid operand type for relational operator");
            }
            *r = (Ret){{TB_INT, NULL, -1}, false, true};
            if(exprRelPrim(r)) {
                return true;
            }
        } else tkerr("Missing expression after >=");
    }
    iTk = start;
    return true;
}


bool exprAdd(Ret *r) {
    Token *start = iTk;
    if(exprMul(r)) {
        if(exprAddPrim(r)) {
            return true;
        }
    }
    iTk = start;
    return false;
}

bool exprAddPrim(Ret *r) {
    Token *start = iTk;
    if(consume(ADD)) {
        Ret right;
        if(exprMul(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) {
                tkerr("Invalid operand type for division/substraction");
            }
            *r = (Ret){tDst, false, true};
            if(exprAddPrim(r)) {
                return true;
            }
        } else tkerr("Missing expression after +");
    }
    else if(consume(SUB)) {
        Ret right;
        if(exprMul(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) {
                tkerr("Invalid operand type for division/substraction");
            }
            *r = (Ret){tDst, false, true};
            if(exprAddPrim(r)) {
                return true;
            }
        } else tkerr("Missing expression after -");
    }
    iTk = start;
    return true;
}


bool exprMul(Ret *r) {
    Token *start = iTk;
    if(exprCast(r)) {
        if(exprMulPrim(r)) {
            return true;
        }
    }
    iTk = start;
    return false;
}

bool exprMulPrim(Ret *r) {
    Token *start = iTk;
    if(consume(MUL)) {
        Ret right;
        if(exprCast(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) {
                tkerr("Invalid operand type for division/multiplication");
            }
            *r = (Ret){tDst, false, true};
            if(exprMulPrim(r)) {
                return true;
            }
        } else tkerr("Missing expression after *");
    }
    else if(consume(DIV)) {
        Ret right;
        if(exprCast(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) {
                tkerr("Invalid operand type for division/multiplication");
            }
            *r = (Ret){tDst, false, true};
            if(exprMulPrim(r)) {
                return true;
            }
        } else tkerr("Missing expression after /");
    }
    iTk = start;
    return true;
}


bool exprCast(Ret *r) {
    Token *start = iTk;

    if (consume(LPAR)) {
        Type t;
        Ret op;
        if (typeBase(&t)) {
            if (arrayDecl(&t)) {
            }
            if (consume(RPAR)) {
                if (exprCast(&op)) {
                    if (t.tb == TB_STRUCT) {
                        tkerr("Cannot convert to a struct type");
                    }
                    if (op.type.tb == TB_STRUCT) {
                        tkerr("Cannot convert a struct");
                    }
                    if (op.type.n >= 0 && t.n < 0) {
                        tkerr("An array can be converted only to another array");
                    }
                    if (op.type.n < 0 && t.n >= 0) {
                        tkerr("A scalar can be converted only to another scalar");
                    }
                    *r = (Ret){t, false, true};
                    return true;
                } else {
                    tkerr("Invalid expression after cast");
                }
            } else {
                tkerr("Missing ) after cast");
            }
        }
    }

    if (exprUnary(r)) {
        return true;
    }

    iTk = start;
    return false;
}


bool exprUnary(Ret *r) {
    Token *start = iTk;

    if (consume(SUB)) {
        if (exprUnary(r)) {
            // Check if the operand is scalar for unary minus
            if (!canBeScalar(r)) {
                tkerr("unary - must have a scalar operand");
            }
            r->lval = false;
            r->ct = true;
            return true;
        } else {
            tkerr("Missing expression after - operator");
        }
    } else if (consume(NOT)) {
        if (exprUnary(r)) {
            // Check if the operand is scalar for logical NOT
            if (!canBeScalar(r)) {
                tkerr("unary ! must have a scalar operand");
            }
            r->type = (Type){TB_INT, NULL, -1}; // is always int
            r->lval = false;                    // is not an l-value
            r->ct = true;                       // is a constant expression
            return true;
        } else {
            tkerr("Missing expression after ! operator");
        }
    }

    iTk = start;
    if (exprPostfix(r)) {
        return true;
    }
    iTk = start;
    return false;
}


bool exprPostfix(Ret *r) {
    Token *start = iTk;
    if (exprPrimary(r)) {
        if (exprPostfixPrim(r)) {
            return true;
        }
    }
    iTk = start;
    return false;
}

bool exprPostfixPrim(Ret *r) {

    Token *start = iTk;

    if (consume(LBRACKET)) {
        Ret idx;
        if (expr(&idx)) {
            if (consume(RBRACKET)) {
                if (r->type.n < 0) {
                    tkerr("only an array can be indexed");
                }
                Type tInt = {TB_INT, NULL, -1};
                if (!convTo(&idx.type, &tInt)) {
                    tkerr("the index is not convertible to int");
                }
                r->type.n = -1;
                r->lval = true;
                r->ct = false;
                if (exprPostfixPrim(r)) {
                    return true;
                } else {
                    tkerr("Invalid expression after ]");
                }
            } else {
                tkerr("Missing ] after expression");
            }
        } else {
            tkerr("Missing [ after expression]");
        }
    } else if (consume(DOT)) {
        if (consume(ID)) {
            Token *tkName = consumedTk;
            if (r->type.tb != TB_STRUCT) {
                tkerr("a field can only be selected from a struct");
            }
            Symbol *s = findSymbolInList(r->type.s->structMembers, tkName->text);
            if (!s) {
                tkerr("the structure %s does not have a field %s", r->type.s->name,
                      tkName->text);
            }
            *r = (Ret){s->type, true, s->type.n >= 0};
            if (exprPostfixPrim(r)) {
                return true;
            } else {
                tkerr("Invalid expression after variable name");
            }
        } else {
            tkerr("Missing identifier after . ");
        }
    }
    iTk = start;
    return true;
}


bool exprPrimary(Ret *r) {
    Token *start = iTk;
    Symbol *s;
    Ret rArg;
    Symbol *param;

    if (consume(ID)) {
        char *tkName = consumedTk->text;
        s = findSymbol(tkName);
        if (!s) {
            tkerr("undefined identifier: %s", tkName);
        }

        if (consume(LPAR)) {
            if (s->kind != SK_FN) {
                tkerr("only a function can be called");
            }
            param = s->fn.params;

            if (expr(&rArg)) {
                if (!param) {
                    tkerr("too many arguments in function call");
                }
                if (!convTo(&rArg.type, &param->type)) {
                    tkerr("in call, cannot convert the argument type to the parameter "
                          "type");
                }
                param = param->next;

                for (;;) {
                    if (consume(COMMA)) {
                        if (expr(&rArg)) {
                            if (!param) {
                                tkerr("too many arguments in function call");
                            }
                            if (!convTo(&rArg.type, &param->type)) {
                                tkerr("in call, cannot convert the argument type to the "
                                      "parameter type");
                            }

                            param = param->next;
                        } else {
                            tkerr("Missing expression after , in function call");
                        }
                    } else {
                        break;
                    }
                }
            }
            if (consume(RPAR)) {
                if (param) {
                    tkerr("too few arguments in function call");
                }
                *r = (Ret){s->type, false, true};
                return true;
            } else {
                tkerr("missing ) in function call");
            }
        } else {
            if (s->kind == SK_FN) {
                tkerr("a function can only be called");
            }
            *r = (Ret){s->type, true, s->type.n >= 0};
            return true;
        }
    } else if (consume(INT)) {
        *r = (Ret){{TB_INT, NULL, -1}, false, true};
        return true;
    } else if (consume(DOUBLE)) {
        *r = (Ret){{TB_DOUBLE, NULL, -1}, false, true};
        return true;
    } else if (consume(CHAR)) {
        *r = (Ret){{TB_CHAR, NULL, -1}, false, true};
        return true;
    } else if (consume(STRING)) {
        *r = (Ret){{TB_CHAR, NULL, 0}, false, true};
        return true;
    } else if (consume(LPAR)) {
        if (expr(r)) {
            if (consume(RPAR)) {
                return true;
            } else {
                tkerr("Missing ) after expression");
            }
        } else {
            tkerr("Missing expression after (");
        }
    }
    iTk = start;
    return false;
}

bool expr(Ret *r) {
    Token *start = iTk;
    if(exprAssign(r)) {
        return true;
    }
    iTk = start;
    return false;
}


bool stm() {
    Token *start = iTk;
    Ret rCond , rExpr;
    if(stmCompound(true)){
        return true;
    }
    if(consume(IF)) {
        if(consume(LPAR)) {
            if(expr(&rCond)) {
                {if(!canBeScalar(&rCond))
                    tkerr("the if condition must be a scalar value");
                }
                if(consume(RPAR)) {
                    if(stm()){
                        if(consume(ELSE)) {
                            if(stm()){
                                return true;
                            } else tkerr("Missing statement after else");
                        }
                        return true;
                    } else tkerr("Missing statement after if");
                } else tkerr("Missing ) from if");
            } else tkerr("Missing expression from if");
        } else tkerr("Missing ( from if");
    }

    if(consume(WHILE)) {
        if (consume(LPAR)) {
            if(expr(&rCond)){
                {if(!canBeScalar(&rCond))
                    tkerr("the while condition must be a scalar value");}
                if(consume(RPAR)) {
                    if(stm()) {
                        return true;
                    } else tkerr("Missing statement from while");
                }else tkerr("Missing ) from while");
            } else tkerr("Missing expression from while");
        } else tkerr("Missing ( from while");
    }

    if(consume(RETURN)) {
        if(expr(&rExpr)){
            {
                if(owner->type.tb==TB_VOID)
                    tkerr("a void function cannot return a value");
                if(!canBeScalar(&rExpr))
                    tkerr("the return value must be a scalar value");
                if(!convTo(&rExpr.type,&owner->type))
                    tkerr("cannot convert the return expressiontype to the function return type");
            }
            if(consume(SEMICOLON)){
                return true;
            } else tkerr("Missing ; from return");
        }
        if (owner->type.tb != TB_VOID) {
            tkerr("a non-void function must return a value");
        }
        if(consume(SEMICOLON)) return true;
    }

    if(expr(&rExpr)) {
        if(consume(SEMICOLON)){
            return true;
        } else tkerr("Missing ; from expression");
    }
    else if(consume(SEMICOLON)) return true;

    iTk = start;
    return false;
}


bool stmCompound(bool dom) {
    Token *start = iTk;
    if(consume(LACC)) {
        if(dom)pushDomain();
        for(;;){
            if(varDef()){}
            else if(stm()){}
            else break;
        }
        if(consume(RACC)){
            if(dom)dropDomain();
            return true;
        } else tkerr("Missing } from compound statement!");
    }
    iTk = start;
    return false;
}


bool fnDef(){
    Token *start = iTk;
    Type t;
    if (consume(VOID)) {
        t.tb=TB_VOID;
        if (consume(ID)) {
            Token *tkName = consumedTk;
            if (consume(LPAR)) {
                Symbol *fn=findSymbolInDomain(symTable,tkName->text);
                if(fn)tkerr("symbol already exists");
                fn=newSymbol(tkName->text,SK_FN);
                fn->type=t;
                addSymbolToDomain(symTable,fn);
                owner=fn;
                pushDomain();
                if (fnParam()) {
                    for (;;) {
                        if (consume(COMMA)) {
                            if (fnParam()) {
                            } else {
                                tkerr("Missing parameter after , in function definition");
                                break;
                            }
                        } else break;
                    }
                }
                if (consume(RPAR)) {
                    if (stmCompound(false)) {
                        dropDomain();
                        owner=NULL;
                        return true;
                    }
                }
            } else tkerr("Missing ( from function define");
        } else tkerr("Missing function identifier");
    }
    else if (typeBase(&t)) {
        if (consume(ID)) {
            Token *tkName = consumedTk;
            if (consume(LPAR)) {
                Symbol *fn=findSymbolInDomain(symTable,tkName->text);
                if(fn)tkerr("symbol already exists");
                fn=newSymbol(tkName->text,SK_FN);
                fn->type=t;
                addSymbolToDomain(symTable,fn);
                owner=fn;
                pushDomain();
                if (fnParam()) {
                    for (;;) {
                        if (consume(COMMA)) {
                            if (fnParam()) {
                            } else {
                                tkerr("Missing parameter after , in function definition!");
                                break;
                            }
                        } else break;
                    }
                }
                if (consume(RPAR)) {
                    if (stmCompound(false)) {
                        dropDomain();
                        owner=NULL;
                        return true;
                    }
                }else tkerr("Missing ( from function define!");
            }
        } else tkerr("Missing function identifier!");
    }
    iTk = start;
    return false;
}


bool unit(){
    Token *start = iTk;
    for(;;){
        if(structDef()){}
        else if(fnDef()){}
        else if(varDef()){}
        else break;
    }
    if(consume(END)){
        return true;
    }
    iTk = start;
    return false;
}


void parse(Token *tokens){
    iTk=tokens;
    if(!unit())tkerr("Syntax error!");
}
