#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "utils.h"
#include "parser.h"

int main()
{
    char *inbuffer = loadFile("./tests/testlex.c");
    //puts(inbuffer);
    Token *tokens = tokenize(inbuffer);
    free(inbuffer);
    showTokens(tokens);
    //parse(tokens);


    return 0;
}