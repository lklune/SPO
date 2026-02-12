#include <stdio.h>
#include <stdlib.h>
#include "node.h"
#include <error.h>
#include "export_ast.h"

// Прототип функции Bison/Yacc
extern FILE* yyin;
extern int yyparse(void);

/* Глобальные массивы узлов из node.c */
extern Node** allNodes;
extern size_t allNodesCount;

void exportAstToJson(Node* root, const char* filename);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Used: %s <input_file>\n", argv[0]);
        return 1;
    }
    if (argc > 1) {
        FILE* input_file = fopen(argv[1], "r");
        if (input_file) {
            // наш указатель -_-
            yyin = input_file;
            // функция синтаксического анализа 
            yyparse();
            fclose(input_file);
            if (allNodesCount > 0) {
                printTree(allNodes[allNodesCount - 1], 0);
                exportAstToJson(allNodesCount > 0 ? allNodes[allNodesCount - 1] : NULL, "ast.json");
                printf("AST saved in ast.json\n");
            }
        }
        else {
            printf("Error opening file: %s\n", argv[1]);
        }
    }
    else {
        printf("Usage: %s <input_file>\n", argv[0]);
    }

    return 0;
}