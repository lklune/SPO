//
//
// error.h

#ifndef ERROR_H
#define ERROR_H

void yyerror(const char* s);
// прототип лексического анализатора для возвращения след токена (вроде, ых)
int yylex(void);

#endif // ERROR_H