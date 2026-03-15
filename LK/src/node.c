#include "node.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

void printTree(const Node* root, int level) {
    if (!root) return;

    for (int i = 0; i < level; i++) {
        printf("  ");
    }

    const char* label = root->value ? root->value : (root->type ? root->type : "<node>");
    printf("%s\n", label);
    printTree(root->left, level + 1);
    printTree(root->right, level + 1);
}