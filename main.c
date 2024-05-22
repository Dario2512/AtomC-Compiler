#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "utils.h"
#include "parser.h"
#include "ad.h"
#include "vm.h"
#include "at.h"

int main()
{
    char *inbuf = loadFile("C:/Users/dario/Desktop/AtomClion2/AtomClion/tests/testat.c");
    Token *tokens = tokenize(inbuf);

    free(inbuf);

    pushDomain();
    parse(tokens);


    showDomain(symTable,"global");
    dropDomain();


    return 0;
}
