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
            else {
                fprintf(f, "%c", c);
            }
        }
    }
    fprintf(f, "\"");
}

static void escape_dot_string(FILE* f, const char* s) {
    if (!s) {
        return;
    }

    for (const char* p = s; *p; p++) {
        unsigned char c = (unsigned char)*p;

        if (c == '"') {
            fprintf(f, "\\\"");
        }
        else if (c == '\\') {
            fprintf(f, "\\\\");
        }
        else if (c == '\n') {
            fprintf(f, "\\n");
        }
        else if (c == '\r') {
            fprintf(f, "\\r");
        }
        else if (c == '\t') {
            fprintf(f, "\\t");
        }
        else if (c < 32) {
            fputc('?', f);
        }
        else {
            fputc(c, f);
        }
    }
}

static void nodeToJson(FILE* f, Node* node, int* counter) {
    int id;
    const char* label;

    if (!node) {
        return;
    }

    id = (*counter)++;
    label = node->value ? node->value : (node->type ? node->type : "<node>");

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

static int nodeToDot(FILE* f, Node* node, int* counter) {
    int id;
    const char* label;

    if (!node) {
        return -1;
    }

    id = (*counter)++;
    label = node->value ? node->value : (node->type ? node->type : "<node>");

    fprintf(f, "  node%d [label=\"", id);
    escape_dot_string(f, label);
    fprintf(f, "\"];\n");

    if (node->left) {
        int left_id = nodeToDot(f, node->left, counter);
        if (left_id >= 0) {
            fprintf(f, "  node%d -> node%d;\n", id, left_id);
        }
    }

    if (node->right) {
        int right_id = nodeToDot(f, node->right, counter);
        if (right_id >= 0) {
            fprintf(f, "  node%d -> node%d;\n", id, right_id);
        }
    }

    return id;
}

void exportAstToJson(Node* root, const char* filename) {
    FILE* f;
    int counter = 0;

    if (!root || !filename) {
        return;
    }

    f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Cannot open file %s\n", filename);
        return;
    }

    nodeToJson(f, root, &counter);

    fclose(f);
}

void exportAstToDot(Node* root, const char* filename) {
    FILE* f;
    int counter = 0;

    if (!root || !filename) {
        return;
    }

    f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Cannot open file %s\n", filename);
        return;
    }

    fprintf(f, "digraph AST {\n");
    fprintf(f, "  rankdir=TB;\n");
    fprintf(f, "  node [shape=box, fontname=\"Courier\", fontsize=11];\n");
    nodeToDot(f, root, &counter);
    fprintf(f, "}\n");

    fclose(f);
}

int exportAstDotToPng(const char* dot_filename, const char* png_filename) {
    char command[4096];
    int rc;

    if (!dot_filename || !png_filename) {
        return 0;
    }

    snprintf(command, sizeof(command),
        "dot -Tpng \"%s\" -o \"%s\"",
        dot_filename, png_filename);

    rc = system(command);
    return rc == 0 ? 1 : 0;
}
