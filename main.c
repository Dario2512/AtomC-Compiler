#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "utils.h"
#include "parser.h"
#include "ad.h"
#include "vm.h"

int main()
{
    char *inbuf = loadFile("tests/testad.c");
    Token *tokens = tokenize(inbuf);

    free(inbuf);

    pushDomain();
    parse(tokens);


    showDomain(symTable,"global");
    dropDomain();


    return 0;
}
