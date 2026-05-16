#ifndef NODE_H
#define NODE_H

typedef struct Node Node;

struct Node {
    char* type;
    Node* left;
    Node* right;
    char* value;
    int line_number;
};

Node* createNode(char* type, Node* left, Node* right, const char* value);
void setNodeLine(Node* node, int line_number);

#endif // NODE_H
