//
// Структуры данных
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

// Создать новую операцию
Operation* createOperation(char* op_type, Operation* left, Operation* right, const char* value, int line_number);

// Освободить память, занятую операцией и всеми её подоперациями
void freeOperation(Operation* op);

// Создать новый базовый блок
BasicBlock* createBasicBlock(int id);

// Добавить операцию в базовый блок
void addOperationToBlock(BasicBlock* block, Operation* op);

// Освободить память, занятую базовым блоком
void freeBasicBlock(BasicBlock* block);

// Создать новый граф потока управления
CFG* createCFG(void);

// Добавить базовый блок в CFG
void addBlockToCFG(CFG* cfg, BasicBlock* block);

// Освободить память, занятую CFG
void freeCFG(CFG* cfg);

// Создать новую функцию
Function* createFunction(FunctionSignature* signature, CFG* cfg, const char* source_file);

// Освободить память, занятую функцией
void freeFunction(Function* func);

// Создать коллекцию файлов
FileCollection* createFileCollection(void);

// Добавить файл в коллекцию
void addFileToCollection(FileCollection* collection, const char* filename, Node* ast);

// Создать коллекцию функций
FunctionCollection* createFunctionCollection(void);

// Добавить функцию в коллекцию
void addFunctionToCollection(FunctionCollection* collection, Function* func);

// Создать коллекцию ошибок
ErrorCollection* createErrorCollection(void);

// Добавить ошибку в коллекцию
void addErrorToCollection(ErrorCollection* collection, const char* message, const char* filename, int line_number);

// Создать результат анализа
AnalysisResult* createAnalysisResult(FunctionCollection* functions, ErrorCollection* errors);

// Освободить память, занятую результатом анализа
void freeAnalysisResult(AnalysisResult* result);

// Построить граф потока управления на основе AST дерева
AnalysisResult* buildCFGFromAST(FileCollection* file_collection);

#endif // CFG_H