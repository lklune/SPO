#include "node.h"
#include <stdlib.h>
#include <string.h>

extern int yylineno;

Node** allNodes = NULL;
size_t allNodesCount = 0;
size_t allNodesCapacity = 0;

Node* createNode(char* type, Node* left, Node* right, const char* value) {
    Node* node = malloc(sizeof(Node));
    if (!node) { return NULL; }

    node->type = type;
    node->left = left;
    node->right = right;
    node->value = value ? strdup(value) : NULL;
    if (left && left->line_number > 0) {
        node->line_number = left->line_number;
    }
    else if (right && right->line_number > 0) {
        node->line_number = right->line_number;
    }
    else {
        node->line_number = yylineno;
    }

    if (allNodesCapacity == 0) {
        allNodesCapacity = 1024;
        allNodes = malloc(allNodesCapacity * sizeof(Node*));
    }
    if (allNodesCount >= allNodesCapacity) {
        allNodesCapacity *= 2;
        allNodes = realloc(allNodes, allNodesCapacity * sizeof(Node*));
    }
    allNodes[allNodesCount++] = node;

    return node;
}

void setNodeLine(Node* node, int line_number) {
    if (!node) {
        return;
    }

    node->line_number = line_number;
}
