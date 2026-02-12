//
//
// node.h

#ifndef NODE_H
#define NODE_H

typedef struct Node Node;

// Структура узла дерева
struct Node {
    char* type;
    Node* left;
    Node* right;
    char* value;
    int id;
};

// Вывести дерево в консоль
void printTree(const Node* root, int level);

// Создать новый узел
Node* createNode(char* type, Node* left, Node* right, const char* value);

#endif // NODE_H