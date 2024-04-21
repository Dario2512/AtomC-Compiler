#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "parser.h"

Token *iTk;		// the iterator in the tokens list
Token *consumedTk;		// the last consumed token


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
            return "N\\A";
    }
}

bool consume(int code){
    //printf("consume(%s)",tkCodeName(code));
    if(iTk->code==code){
        consumedTk=iTk;
        iTk=iTk->next;
        //printf(" => consumed\n");
        return true;
    }
    return false;
}


bool typeBase(){
    Token *start = iTk;
    if(consume(TYPE_INT)){
        return true;
    }
    if(consume(TYPE_DOUBLE)){
        return true;
    }
    if(consume(TYPE_CHAR)){
        return true;
    }
    if(consume(STRUCT)){
        if(consume(ID)){
            return true;
        }else tkerr("Missing structure name!");
    }
    iTk=start;
    return false;
}


bool arrayDecl(){
    Token *start = iTk;
    if(consume(LBRACKET)){
        if(consume(INT)){}
        if(consume(RBRACKET)){
            return true;
        }else tkerr("Missing ] from array declaration!");
    }
    iTk = start;
    return false;
}


bool varDef(){
    Token *start = iTk;
    if(typeBase()){
        if(consume(ID)){
            if(arrayDecl()){
                if(consume(SEMICOLON)){
                    return true;
                }else tkerr("Missing ; from variable declaration!");
            }
            if(consume(SEMICOLON)){
                return true;
            }else tkerr("Missing ; from variable variabilei!!");
        }else tkerr("Missing identifier from variable declaration");
    }
    iTk = start;
    return false;
}


bool structDef(){
    Token *start = iTk;
    if(consume(STRUCT)) {
        if(consume(ID)) {
            if(consume(LACC)) {
                while(1) {
                    if(varDef()) {
                    }
                    else break;
                }
                if(consume(RACC)) {
                    if(consume(SEMICOLON)) {
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
    if(typeBase()) {
        if(consume(ID)){
            if(arrayDecl()) {
                return true;
            }
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
    return true; // epsilon
}


bool exprCast() {
    Token *start = iTk;
    if(consume(LPAR)) {
        if(typeBase()) {
            if(arrayDecl()) {
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
    if(stmCompound()){
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


bool stmCompound() {
    Token *start = iTk;
    if(consume(LACC)) {
        for(;;){
            if(varDef()){}
            else if(stm()){}
            else break;
        }
        if(consume(RACC)){
            return true;
        } else tkerr("Missing } from compound statement!");
    }
    iTk = start;
    return false;
}


bool fnDef(){
    Token *start = iTk;
    if (consume(VOID)) {
        if (consume(ID)) {
            if (consume(LPAR)) {
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
                    if (stmCompound()) {
                        return true;
                    }
                }
            } else tkerr("Missing ( from function define");
        } else tkerr("Missing function identifier");
    }
    else if (typeBase()) {
        if (consume(ID)) {
            if (consume(LPAR)) {
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
                    if (stmCompound()) {
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
