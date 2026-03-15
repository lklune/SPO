#ifndef NODE_H
#define NODE_H

typedef struct Node Node;

struct Node {
    char* type;
    Node* left;
    Node* right;
    char* value;
    int id;
};

void printTree(const Node* root, int level);

Node* createNode(char* type, Node* left, Node* right, const char* value);

#endif // NODE_H