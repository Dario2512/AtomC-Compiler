#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "lexer.h"

int main()
{
    char *inbuf=loadFile("tests/1.c");
    puts(inbuf);
    Token *tokens=tokenize(inbuf);
    showTokens(tokens);
    free(inbuf);

    return 0;
}