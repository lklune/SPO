%code requires {
#include "node.h"
}
%{
#include "parser.tab.h"
#include "error.h"

int yylex(void);              // прототип лексера
void yyerror(const char *s);  // прототип функции обработки ошибок
%}

%debug
%define parse.error verbose

%union {
    Node* node;
}

%token <node> PLUS MINUS STAR SLASH PERCENT
%token <node> ASSIGN
%token <node> EQUAL NOTEQUAL
%token <node> LESSTHAN GREATERTHAN LESSTHANEQ GREATERTHANEQ
%token <node> AND OR NOT
%token <node> BIT_AND BIT_OR BIT_XOR
%token <node> SHIFT_LEFT SHIFT_RIGHT
%token <node> BIT_NOT
%token <node> OF
%token <node> ARRAY
%token <node> DEF END BEGIN_BLOCK
%token <node> IDENTIFIER
%token <node> STR
%token <node> COMMA
%token <node> DOUBLE_DOT
%token <node> CHAR
%token <node> BIN HEX DEC
%token <node> TRUE FALSE
%token <node> IF ELSE THAN WHILE UNTIL DO BREAK
%token <node> SEMICOLON
%token <node> LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET
%token <node> TYPEDEF
%token <node> ARRAY_COMMAS

%right BIT_NOT NOT
%left STAR SLASH PERCENT
%left SHIFT_LEFT SHIFT_RIGHT
%left PLUS MINUS
%left BIT_AND
%left BIT_XOR
%left BIT_OR
%left LESSTHAN GREATERTHAN LESSTHANEQ GREATERTHANEQ
%left EQUAL NOTEQUAL
%left AND
%left OR
%right ASSIGN

%type <node> typeRef
%type <node> funcSignature
%type <node> argDef
%type <node> sourceItem
%type <node> listSourceItem
%type <node> statement
%type <node> var
%type <node> if
%type <node> block
%type <node> loop
%type <node> repeat
%type <node> break
%type <node> expression
%type <node> builtin
%type <node> custom
%type <node> array
%type <node> source
%type <node> listArgDef
%type <node> optionalTypeRef
%type <node> literal
%type <node> place
%type <node> slice
%type <node> range
%type <node> listRange
%type <node> expr
%type <node> listExpr
%type <node> optionalListExpr
%type <node> call
%type <node> braces
%type <node> unary
%type <node> binary
%type <node> assignment
%type <node> listVarDeclared
%type <node> listVarDeclaredItem
%type <node> optionalElseStatement
%type <node> listStatement

%%

source: 
      { $$ = NULL; }
    | source sourceItem { 
          $$ = createNode("source", $1, $2, NULL); 
      }
    ;

sourceItem: 
      DEF funcSignature listStatement END { 
          $$ = createNode("sourceItem", $2, $3, NULL); 
      }
    ;

listSourceItem: 
      sourceItem listSourceItem { $$ = createNode("listSourceItem", $1, $2, NULL); }
    | { $$ = NULL; }
    ;

/* FuncSignature */

funcSignature: 
      IDENTIFIER LPAREN listArgDef RPAREN optionalTypeRef { 
          $$ = createNode("funcSignature", $3, $5, $1 ? $1->value : NULL); 
      }
    ;

listArgDef: 
      { $$ = NULL; }
    | argDef { $$ = $1; }
    | argDef COMMA listArgDef { $$ = createNode("listArgDef", $1, $3, NULL); }
    ;

argDef: 
      IDENTIFIER optionalTypeRef { $$ = createNode("argDef", $1, $2, NULL); }
    ;

optionalTypeRef: 
      { $$ = NULL; }
    | LPAREN OF typeRef RPAREN { $$ = $3; }
    ;

/* TypeRef */

typeRef: 
      builtin { $$ = $1; }
    | custom { $$ = $1; }
    | array { $$ = $1; }
    ;

builtin: 
      TYPEDEF { $$ = $1; }
    ;

/* Statement */

statement: 
      var { $$ = $1; }
    | if { $$ = $1; }
    | loop { $$ = $1; }
    | repeat { $$ = $1; }
    | block { $$ = $1; }
    | break { $$ = $1; }
    | expression { $$ = $1; }
    ;

listStatement: 
      statement listStatement { $$ = createNode("listStatement", $1, $2, NULL); }
    | { $$ = NULL; }
    ;

custom: IDENTIFIER { $$ = $1; };

array: typeRef ARRAY LBRACKET DEC RBRACKET { $$ = createNode("array", $1, NULL, $4 ? $4->value : NULL); };

/* IF ELSE */

if: IF expr THAN statement optionalElseStatement { 
        $$ = createNode("if", $2, $3, NULL);
        if ($4) {
            $$->right = $4;
        }
    };

optionalElseStatement: 
      ELSE statement optionalElseStatement { $$ = createNode("else", $2, $3, NULL); }
    | ELSE statement { $$ = createNode("else", $2, NULL, NULL); }
    | { $$ = NULL; };

/* Block */

block: 
      LBRACE listStatement RBRACE { $$ = createNode("block", $2, NULL, NULL); }
    | BEGIN_BLOCK listStatement END { $$ = createNode("block", $2, NULL, NULL); }
    | LBRACE listSourceItem RBRACE { $$ = createNode("block", $2, NULL, NULL); }
    | BEGIN_BLOCK listSourceItem END { $$ = createNode("block", $2, NULL, NULL); }
    | LBRACE RBRACE { $$ = createNode("block", NULL, NULL, NULL); }
    | BEGIN_BLOCK END { $$ = createNode("block", NULL, NULL, NULL); }
    ;

loop: 
      WHILE expr listStatement END { $$ = createNode("loop", $2, $3, "while"); }
    | UNTIL expr listStatement END { $$ = createNode("loop", $2, $3, "until"); }
    ;

repeat: 
      statement WHILE expr SEMICOLON { $$ = createNode("repeat", $1, $3, "while"); }
    | statement UNTIL expr SEMICOLON { $$ = createNode("repeat", $1, $3, "until"); }
    ;

break: BREAK SEMICOLON { $$ = createNode("break", NULL, NULL, NULL); };

expression: expr SEMICOLON { $$ = $1; };

assignment: expr ASSIGN expr { $$ = createNode("assignment", $1, $3, NULL); };

expr: unary { $$ = $1; }
    | binary { $$ = $1; }
    | assignment { $$ = $1; }
    | braces { $$ = $1; }
    | call { $$ = $1; }
    | slice { $$ = $1; }
    | place { $$ = $1; }
    | literal { $$ = $1; }
    ;

binary: expr PLUS expr { $$ = createNode("SUM", $1, $3, NULL); }
    | expr MINUS expr { $$ = createNode("MINUS", $1, $3, NULL); }
    | expr STAR expr { $$ = createNode("MUL", $1, $3, NULL); }
    | expr SLASH expr { $$ = createNode("SLASH", $1, $3, NULL); }
    | expr PERCENT expr { $$ = createNode("PERCENT", $1, $3, NULL); }
    | expr EQUAL expr { $$ = createNode("EQUALITY", $1, $3, NULL); }
    | expr NOTEQUAL expr { $$ = createNode("NOTEQUAL", $1, $3, NULL); }
    | expr LESSTHAN expr { $$ = createNode("LESSTHAN", $1, $3, NULL); }
    | expr GREATERTHAN expr { $$ = createNode("GREATERTHAN", $1, $3, NULL); }
    | expr LESSTHANEQ expr { $$ = createNode("LESSTHANEQ", $1, $3, NULL); }
    | expr GREATERTHANEQ expr { $$ = createNode("GREATERTHANEQ", $1, $3, NULL); }
    | expr AND expr { $$ = createNode("AND", $1, $3, NULL); }
    | expr OR expr { $$ = createNode("OR", $1, $3, NULL); }
    | expr BIT_AND expr { $$ = createNode("BIT_AND", $1, $3, NULL); }
    | expr BIT_OR expr { $$ = createNode("BIT_OR", $1, $3, NULL); }
    | expr BIT_XOR expr { $$ = createNode("BIT_XOR", $1, $3, NULL); }
    | expr SHIFT_LEFT expr { $$ = createNode("SHIFT_LEFT", $1, $3, NULL); }
    | expr SHIFT_RIGHT expr { $$ = createNode("SHIFT_RIGHT", $1, $3, NULL); }
    ;

unary: PLUS expr { $$ = createNode("PLUS", $2, NULL, NULL); }
    | MINUS expr { $$ = createNode("MINUS", $2, NULL, NULL); }
    | NOT expr { $$ = createNode("NOT", $2, NULL, NULL); }
    | BIT_NOT expr { $$ = createNode("BIT_NOT", $2, NULL, NULL); }
    ;

braces: LPAREN expr RPAREN { $$ = createNode("braces", $2, NULL, NULL); };

call: IDENTIFIER LPAREN optionalListExpr RPAREN { $$ = createNode("CALL", $1, $3, NULL); };

optionalListExpr: listExpr { $$ = createNode("optionalListExpr", $1, NULL, NULL); }
    | { $$ = createNode("optionalListExpr", NULL, NULL, NULL); };

listExpr: expr COMMA listExpr { $$ = createNode("listExpr", $1, $3, NULL); }
    | expr { $$ = createNode("listExpr", $1, NULL, NULL); };

slice: expr LBRACKET listRange RBRACKET { $$ = createNode("slice", $1, $3, NULL); };

range: expr DOUBLE_DOT expr { $$ = createNode("range", $1, $3, NULL); }
    | expr { $$ = createNode("range", $1, NULL, NULL); };

listRange: range COMMA listRange { $$ = createNode("listRange", $1, $3, NULL); }
    | range { $$ = createNode("listRange", $1, NULL, NULL); };

place: IDENTIFIER { $$ = $1; };

literal: TRUE { $$ = $1; }
    | FALSE { $$ = $1; }
    | STR { $$ = $1; }
    | CHAR { $$ = $1; }
    | HEX { $$ = $1; }
    | BIN { $$ = $1; }
    | DEC { $$ = $1; };

/* VAR */

listVarDeclared: listVarDeclaredItem COMMA listVarDeclared { $$ = createNode("listVarDeclared", $1, $3, NULL); }
    | listVarDeclaredItem { $$ = createNode("listVarDeclared", $1, NULL, NULL); };

listVarDeclaredItem: IDENTIFIER { $$ = $1; }
    | assignment { $$ = $1; };

var: typeRef listVarDeclared SEMICOLON { $$ = createNode("var", $1, $2, NULL); };

%%