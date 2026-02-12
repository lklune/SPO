//
//
// error.c

#include <stdio.h>
#include "error.h"

void yyerror(const char* s) {
    fprintf(stderr, "Ошибка: %s\n", s);
}