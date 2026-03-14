//
// ��������� ������
// 

#ifndef CFG_H
#define CFG_H

#include "node.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct Operation Operation;
struct Operation {
    char* op_type;
    Operation* left;
    Operation* right;
    char* value;
    int line_number;
    Operation* next;
};

typedef struct BasicBlock BasicBlock;
struct BasicBlock {
    int id;
    Operation* operations;
    BasicBlock* true_target;
    BasicBlock* false_target;
    BasicBlock* next;
    int is_entry;
    int is_exit;
    Node* ast_node;
};

typedef struct CFG CFG;
struct CFG {
    BasicBlock* entry_block;
    BasicBlock* exit_block;
    BasicBlock* blocks;
    int block_count;
};

typedef struct FunctionArg FunctionArg;
struct FunctionArg {
    char* name;
    char* type;
    FunctionArg* next;
};

typedef struct FunctionSignature FunctionSignature;
struct FunctionSignature {
    char* name;
    FunctionArg* args;
    char* return_type;
};

typedef struct Function Function;
struct Function {
    FunctionSignature* signature;
    CFG* cfg;
    char* source_file;
    Function* next;
};

typedef struct FileInfo FileInfo;
struct FileInfo {
    char* filename;
    Node* ast;
    FileInfo* next;
};

typedef struct FileCollection FileCollection;
struct FileCollection {
    FileInfo* files;
    int file_count;
};

typedef struct FunctionCollection FunctionCollection;
struct FunctionCollection {
    Function* functions;
    int function_count;
};

typedef struct Error Error;
struct Error {
    char* message;
    char* filename;
    int line_number;
    Error* next;
};

typedef struct ErrorCollection ErrorCollection;
struct ErrorCollection {
    Error* errors;
    int error_count;
};

typedef struct AnalysisResult AnalysisResult;
struct AnalysisResult {
    FunctionCollection* functions;
    ErrorCollection* errors;
};

// ������� ����� ��������
Operation* createOperation(char* op_type, Operation* left, Operation* right, const char* value, int line_number);

// ���������� ������, ������� ��������� � ����� � �������������
void freeOperation(Operation* op);

// ������� ����� ������� ����
BasicBlock* createBasicBlock(int id);

// �������� �������� � ������� ����
void addOperationToBlock(BasicBlock* block, Operation* op);

// ���������� ������, ������� ������� ������
void freeBasicBlock(BasicBlock* block);

// ������� ����� ���� ������ ����������
CFG* createCFG(void);

// �������� ������� ���� � CFG
void addBlockToCFG(CFG* cfg, BasicBlock* block);

// ���������� ������, ������� CFG
void freeCFG(CFG* cfg);

// ������� ����� �������
Function* createFunction(FunctionSignature* signature, CFG* cfg, const char* source_file);

// ���������� ������, ������� ��������
void freeFunction(Function* func);

// ������� ��������� ������
FileCollection* createFileCollection(void);

// �������� ���� � ���������
void addFileToCollection(FileCollection* collection, const char* filename, Node* ast);

// ������� ��������� �������
FunctionCollection* createFunctionCollection(void);

// �������� ������� � ���������
void addFunctionToCollection(FunctionCollection* collection, Function* func);

// ������� ��������� ������
ErrorCollection* createErrorCollection(void);

// �������� ������ � ���������
void addErrorToCollection(ErrorCollection* collection, const char* message, const char* filename, int line_number);

// ������� ��������� �������
AnalysisResult* createAnalysisResult(FunctionCollection* functions, ErrorCollection* errors);

// ���������� ������, ������� ����������� �������
void freeAnalysisResult(AnalysisResult* result);

// ��������� ���� ������ ���������� �� ������ AST ������
AnalysisResult* buildCFGFromAST(FileCollection* file_collection);

#endif // CFG_H