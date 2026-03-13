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
#line 1 "parser.y"

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
    ASSIGN = 263,                  /* ASSIGN  */
    EQUAL = 264,                   /* EQUAL  */
    NOTEQUAL = 265,                /* NOTEQUAL  */
    LESSTHAN = 266,                /* LESSTHAN  */
    GREATERTHAN = 267,             /* GREATERTHAN  */
    LESSTHANEQ = 268,              /* LESSTHANEQ  */
    GREATERTHANEQ = 269,           /* GREATERTHANEQ  */
    AND = 270,                     /* AND  */
    OR = 271,                      /* OR  */
    NOT = 272,                     /* NOT  */
    BIT_AND = 273,                 /* BIT_AND  */
    BIT_OR = 274,                  /* BIT_OR  */
    BIT_XOR = 275,                 /* BIT_XOR  */
    SHIFT_LEFT = 276,              /* SHIFT_LEFT  */
    SHIFT_RIGHT = 277,             /* SHIFT_RIGHT  */
    BIT_NOT = 278,                 /* BIT_NOT  */
    OF = 279,                      /* OF  */
    ARRAY = 280,                   /* ARRAY  */
    DEF = 281,                     /* DEF  */
    END = 282,                     /* END  */
    BEGIN_BLOCK = 283,             /* BEGIN_BLOCK  */
    IDENTIFIER = 284,              /* IDENTIFIER  */
    STR = 285,                     /* STR  */
    COMMA = 286,                   /* COMMA  */
    DOUBLE_DOT = 287,              /* DOUBLE_DOT  */
    CHAR = 288,                    /* CHAR  */
    BIN = 289,                     /* BIN  */
    HEX = 290,                     /* HEX  */
    DEC = 291,                     /* DEC  */
    TRUE = 292,                    /* TRUE  */
    FALSE = 293,                   /* FALSE  */
    IF = 294,                      /* IF  */
    ELSE = 295,                    /* ELSE  */
    THAN = 296,                    /* THAN  */
    WHILE = 297,                   /* WHILE  */
    UNTIL = 298,                   /* UNTIL  */
    DO = 299,                      /* DO  */
    BREAK = 300,                   /* BREAK  */
    SEMICOLON = 301,               /* SEMICOLON  */
    LPAREN = 302,                  /* LPAREN  */
    RPAREN = 303,                  /* RPAREN  */
    LBRACE = 304,                  /* LBRACE  */
    RBRACE = 305,                  /* RBRACE  */
    LBRACKET = 306,                /* LBRACKET  */
    RBRACKET = 307,                /* RBRACKET  */
    TYPEDEF = 308,                 /* TYPEDEF  */
    ARRAY_COMMAS = 309             /* ARRAY_COMMAS  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 15 "parser.y"

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
