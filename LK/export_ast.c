//
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "export_ast.h"

static void nodeToJson(FILE* f, Node* node, int* counter) {
    if (!node) return;

    int id = (*counter)++;
    const char* label = node->value ? node->value : (node->type ? node->type : "<node>");

    char escaped[1024];
    int j = 0;
    for (int i = 0; label[i] && j < 1023; i++) {
        if (label[i] == '\"') escaped[j++] = '\\';
        escaped[j++] = label[i];
    }
    escaped[j] = '\0';

    fprintf(f, "{ \"id\": \"node%d\", \"name\": \"%s\"", id, escaped);

    if (node->left || node->right) {
        fprintf(f, ", \"children\": [\n");
        if (node->left) nodeToJson(f, node->left, counter);
        if (node->left && node->right) fprintf(f, ",\n");
        if (node->right) nodeToJson(f, node->right, counter);
        fprintf(f, "]\n");
    }
    fprintf(f, "}");
}


void exportAstToJson(Node* root, const char* filename) {
    if (!root) return;

    FILE* f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Не удалось создать файл %s\n", filename);
        return;
    }

    int counter = 0;
    nodeToJson(f, root, &counter);

    fclose(f);
}
