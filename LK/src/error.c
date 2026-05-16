#include <stdio.h>
#include "error.h"

extern int yylineno;

void yyerror(const char* s) {
    fprintf(stderr, "Parse error at line %d: %s\n",
        yylineno, s ? s : "unknown");
}
