//
// export_ast.c
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "export_ast.h"

static void escape_json_string(FILE* f, const char* s) {
    if (!s) {
        fprintf(f, "\"\"");
        return;
    }

    fprintf(f, "\"");
    for (const char* p = s; *p; p++) {
        unsigned char c = (unsigned char)*p;

        switch (c) {
        case '"':  fprintf(f, "\\\""); break;      // Двойная кавычка
        case '\\': fprintf(f, "\\\\"); break;      // Обратный слэш
        case '\b': fprintf(f, "\\b");  break;      // Backspace
        case '\f': fprintf(f, "\\f");  break;      // Form feed
        case '\n': fprintf(f, "\\n");  break;      // Перевод строки
        case '\r': fprintf(f, "\\r");  break;      // Возврат каретки
        case '\t': fprintf(f, "\\t");  break;      // Табуляция
        default:
            if (c < 32) {
                fprintf(f, "\\u%04x", c);
            }
            else if (c < 127) {
                fprintf(f, "%c", c);
            }
            else {

            }
        }
    }
    fprintf(f, "\"");
}

static void nodeToJson(FILE* f, Node* node, int* counter) {
    if (!node) return;

    int id = (*counter)++;
    const char* label = node->value ? node->value : (node->type ? node->type : "<node>");

    fprintf(f, "{ \"id\": \"node%d\", \"name\": ", id);
    escape_json_string(f, label);

    if (node->left || node->right) {
        fprintf(f, ", \"children\": [\n");
        if (node->left) {
            nodeToJson(f, node->left, counter);
        }
        if (node->left && node->right) {
            fprintf(f, ",\n");
        }
        if (node->right) {
            nodeToJson(f, node->right, counter);
        }
        fprintf(f, "\n]\n");
    }

    fprintf(f, "}");
}

void exportAstToJson(Node* root, const char* filename) {
    if (!root) return;

    FILE* f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Cannot open file %s\n", filename);
        return;
    }

    int counter = 0;
    nodeToJson(f, root, &counter);

    fclose(f);
}