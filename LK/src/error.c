#include <stdio.h>
#include "error.h"

void yyerror(const char* s) {
    fprintf(stderr, "Parse error: %s\n", s ? s : "unknown");
}
