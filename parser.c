#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "parser.h"
#include "ad.h"
#include "lexer.h"

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


bool exprOrPrim() {
    Token *start = iTk;
    if(consume(OR)) {
        if(exprAnd()) {
            if(exprOrPrim()) {
                return true;
            }
        } else tkerr("Missing expression after ||");
    }
    iTk = start;
    return true;
}

bool exprOr() {
    Token *start = iTk;
    if(exprAnd()) {
        if(exprOrPrim()) {
            return true;
        }
    }
    iTk = start;
    return false;
}


bool exprAnd() {
    Token *start = iTk;
    if(exprEq()) {
        if(exprAndPrim()) {
            return true;
        }
    }
    iTk = start;
    return false;
}

bool exprAndPrim() {
    Token *start = iTk;
    if(consume(AND)) {
        if(exprEq()) {
            if(exprAndPrim()) {
                return true;
            }
        } else tkerr("Missing expression after &&");
    }
    iTk = start;
    return true;
}


bool exprEq() {
    Token *start = iTk;
    if(exprRel()) {
        if(exprEqPrim()) {
            return true;
        }
    }
    iTk = start;
    return false;
}

bool exprEqPrim() {
    Token *start = iTk;
    if(consume(EQUAL)) {
        if(exprRel()) {
            if(exprEqPrim()) {
                return true;
            }
        } else tkerr("Missing expression after ==");
    }
    else if(consume(NOTEQ)) {
        if(exprRel()) {
            if(exprEqPrim()) {
                return true;
            }
        } else tkerr("Missing expression after !=");
    }
    iTk = start;
    return true;
}


bool exprAssign() {
    Token *start = iTk;
    if(exprUnary()) {
        if(consume(ASSIGN)) {
            if(exprAssign()){
                return true;
            } else tkerr("Missing expression after =");
        }
    }
    iTk = start;
    if(exprOr()) {
        return true;
    }
    iTk = start;
    return false;
}


bool exprRel() {
    Token *start = iTk;
    if(exprAdd()) {
        if(exprRelPrim()) {
            return true;
        }
    }
    iTk = start;
    return false;
}

bool exprRelPrim() {
    Token *start = iTk;
    if(consume(LESS)) {
        if(exprAdd()) {
            if(exprRelPrim()) {
                return true;
            }
        }  else tkerr("Missing expression after <");
    }
    else if(consume(LESSEQ)) {
        if(exprAdd()) {
            if(exprRelPrim()) {
                return true;
            }
        } else tkerr("Missing expression after <=");
    }
    else if(consume(GREATER)) {
        if(exprAdd()) {
            if(exprRelPrim()) {
                return true;
            }
        } else tkerr("Missing expression after >");
    }
    else if(consume(GREATEREQ)) {
        if(exprAdd()) {
            if(exprRelPrim()) {
                return true;
            }
        } else tkerr("Missing expression after >=");
    }
    iTk = start;
    return true;
}


bool exprAdd() {
    Token *start = iTk;
    if(exprMul()) {
        if(exprAddPrim()) {
            return true;
        }
    }
    iTk = start;
    return false;
}

bool exprAddPrim() {
    Token *start = iTk;
    if(consume(ADD)) {
        if(exprMul()) {
            if(exprAddPrim()) {
                return true;
            }
        } else tkerr("Missing expression after +");
    }
    else if(consume(SUB)) {
        if(exprMul()) {
            if(exprAddPrim()) {
                return true;
            }
        } else tkerr("Missing expression after -");
    }
    iTk = start;
    return true;
}


bool exprMul() {
    Token *start = iTk;
    if(exprCast()) {
        if(exprMulPrim()) {
            return true;
        }
    }
    iTk = start;
    return false;
}

bool exprMulPrim() {
    Token *start = iTk;
    if(consume(MUL)) {
        if(exprCast()) {
            if(exprMulPrim()) {
                return true;
            }
        } else tkerr("Missing expression after *");
    }
    else if(consume(DIV)) {
        if(exprCast()) {
            if(exprMulPrim()) {
                return true;
            }
        } else tkerr("Missing expression after /");
    }
    iTk = start;
    return true;
}


bool exprCast() {
    Token *start = iTk;
    if(consume(LPAR)) {
        Type t;
        if(typeBase(&t)) {
            if(arrayDecl(&t)) {
                if(consume(RPAR)) {
                    if(exprCast()) {
                        return true;
                    }
                } else tkerr("Missing ) from cast expression!");
            }

            if(consume(RPAR)) {
                if(exprCast()) {
                    return true;
                }
            }
        } else tkerr("Cast expression type is missing or it is not the correct one!");
    }
    iTk = start;
    if(exprUnary()) {
        return true;
    }
    iTk = start;
    return false;
}


bool exprUnary() {
    Token *start = iTk;
    if(consume(SUB)) {
        if(exprUnary()) {
            return true;
        } else tkerr("Missing expressoin after -");
    }
    else if(consume(NOT)) {
        if(exprUnary()) {
            return true;
        } else tkerr("Missing expression after ! opperator");
    }
    iTk = start;
    if(exprPostfix()) {
        return true;
    }
    iTk = start;
    return false;
}


bool exprPostfix() {
    Token *start = iTk;
    if(exprPrimary()) {
        if(exprPostfixPrim()) {
            return true;
        }
    }
    iTk = start;
    return false;
}

bool exprPostfixPrim() {
    Token *start = iTk;
    if(consume(LBRACKET)) {
        if(expr()) {
            if(consume(RBRACKET)) {
                if(exprPostfixPrim()) {
                    return true;
                }
            } else tkerr("Missing ] from array access");
        } else tkerr("Missing expression from array access");
    }
    if(consume(DOT)) {
        if(consume(ID)) {
            if(exprPostfixPrim()) {
                return true;
            }
        } else tkerr("Missing identifier after . opperator");
    }
    iTk = start;
    return true;
}


bool exprPrimary() {
    Token *start = iTk;
    if(consume(ID)) {
        if(consume(LPAR)) {
            if(expr()) {
                for(;;) {
                    if(consume(COMMA)) {
                        if(expr()) {

                        } else {
                            tkerr("Missing expression after , in function!");
                            break;
                        }
                    }
                    else break;
                }
            }
            if(consume(RPAR)) {
                return true;
            } else tkerr("Missing ) in function");
        }
        return true;
    }
    if (consume(INT)) {
        return true;
    }
    else if (consume(DOUBLE)) {
        return true;
    }
    else if (consume(CHAR)) {
        return true;
    }
    else if (consume(STRING)) {
        return true;
    }
    if(consume(LPAR)) {
        if(expr()) {
            if(consume(RPAR)) {
                return true;
            } else tkerr("Missing ) in function!");
        }
    }
    iTk = start;
    return false;
}

bool expr() {
    if(exprAssign()) {
        return true;
    }
    return false;
}


bool stm() {
    Token *start = iTk;
    if(stmCompound(true)){
        return true;
    }
    if(consume(IF)) {
        if(consume(LPAR)) {
            if(expr()) {
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
            if(expr()){
                if(consume(RPAR)) {
                    if(stm()) {
                        return true;
                    } else tkerr("Missing statement from while");
                }else tkerr("Missing ) from while");
            } else tkerr("Missing expression from while");
        } else tkerr("Missing ( from while");
    }

    if(consume(RETURN)) {
        if(expr()){
            if(consume(SEMICOLON)){
                return true;
            } else tkerr("Missing ; from return");
        }
        if(consume(SEMICOLON)) return true;
    }

    if(expr()) {
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
