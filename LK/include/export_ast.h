#ifndef EXPORT_AST_H
#define EXPORT_AST_H

#include "node.h"

void exportAstToJson(Node* root, const char* filename);
void exportAstToDot(Node* root, const char* filename);
int exportAstDotToPng(const char* dot_filename, const char* png_filename);

#endif // EXPORT_AST_H
