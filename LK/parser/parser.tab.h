/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_PARSER_TAB_H_INCLUDED
# define YY_YY_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 1 ".\\parser.y"

#include "node.h"

#line 53 "parser.tab.h"

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    PLUS = 258,                    /* PLUS  */
    MINUS = 259,                   /* MINUS  */
    STAR = 260,                    /* STAR  */
    SLASH = 261,                   /* SLASH  */
    PERCENT = 262,                 /* PERCENT  */
    EQUAL = 263,                   /* EQUAL  */
    NOTEQUAL = 264,                /* NOTEQUAL  */
    LESSTHAN = 265,                /* LESSTHAN  */
    GREATERTHAN = 266,             /* GREATERTHAN  */
    LESSTHANEQ = 267,              /* LESSTHANEQ  */
    GREATERTHANEQ = 268,           /* GREATERTHANEQ  */
    AND = 269,                     /* AND  */
    OR = 270,                      /* OR  */
    NOT = 271,                     /* NOT  */
    OF = 272,                      /* OF  */
    ARRAY = 273,                   /* ARRAY  */
    DEF = 274,                     /* DEF  */
    END = 275,                     /* END  */
    BEGIN_BLOCK = 276,             /* BEGIN_BLOCK  */
    IDENTIFIER = 277,              /* IDENTIFIER  */
    STR = 278,                     /* STR  */
    COMMA = 279,                   /* COMMA  */
    DOUBLE_DOT = 280,              /* DOUBLE_DOT  */
    CHAR = 281,                    /* CHAR  */
    BIN = 282,                     /* BIN  */
    HEX = 283,                     /* HEX  */
    DEC = 284,                     /* DEC  */
    TRUE = 285,                    /* TRUE  */
    FALSE = 286,                   /* FALSE  */
    IF = 287,                      /* IF  */
    ELSE = 288,                    /* ELSE  */
    THAN = 289,                    /* THAN  */
    WHILE = 290,                   /* WHILE  */
    UNTIL = 291,                   /* UNTIL  */
    DO = 292,                      /* DO  */
    BREAK = 293,                   /* BREAK  */
    SEMICOLON = 294,               /* SEMICOLON  */
    LPAREN = 295,                  /* LPAREN  */
    RPAREN = 296,                  /* RPAREN  */
    LBRACE = 297,                  /* LBRACE  */
    RBRACE = 298,                  /* RBRACE  */
    LBRACKET = 299,                /* LBRACKET  */
    RBRACKET = 300,                /* RBRACKET  */
    TYPEDEF = 301,                 /* TYPEDEF  */
    ARRAY_COMMAS = 302,            /* ARRAY_COMMAS  */
    ASSIGN = 303,                  /* ASSIGN  */
    BIT_OR = 304,                  /* BIT_OR  */
    BIT_XOR = 305,                 /* BIT_XOR  */
    BIT_AND = 306,                 /* BIT_AND  */
    SHIFT_LEFT = 307,              /* SHIFT_LEFT  */
    SHIFT_RIGHT = 308,             /* SHIFT_RIGHT  */
    BIT_NOT = 309                  /* BIT_NOT  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 15 ".\\parser.y"

    Node* node;

#line 128 "parser.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_PARSER_TAB_H_INCLUDED  */
