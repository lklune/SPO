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

typedef struct UserTypeField UserTypeField;
struct UserTypeField {
    char* name;
    char* type_name;
    int offset;
    char* owner_type_name;
    UserTypeField* next;
};

typedef struct UserTypeMethod UserTypeMethod;
struct UserTypeMethod {
    char* name;
    char* full_name;
    char* return_type;
    Node* ast_node;
    UserTypeMethod* next;
};

typedef struct UserType UserType;
struct UserType {
    char* name;
    char* base_type_name;
    UserTypeField* fields;
    UserTypeMethod* methods;
    int size_bytes;
    int resolved;
    int resolving;
    UserType* next;
};

typedef struct TypeCollection TypeCollection;
struct TypeCollection {
    UserType* types;
    int type_count;
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
    char* owner_type_name;
    int is_method;
    TypeCollection* types;
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
    TypeCollection* types;
};

Operation* createOperation(char* op_type, Operation* left, Operation* right, const char* value, int line_number);
void freeOperation(Operation* op);

BasicBlock* createBasicBlock(int id);
void addOperationToBlock(BasicBlock* block, Operation* op);
void freeBasicBlock(BasicBlock* block);

CFG* createCFG(void);
void addBlockToCFG(CFG* cfg, BasicBlock* block);
void freeCFG(CFG* cfg);

TypeCollection* createTypeCollection(void);
void addTypeToCollection(TypeCollection* collection, UserType* type_info);
UserType* findUserType(TypeCollection* collection, const char* type_name);
UserTypeField* findUserTypeField(TypeCollection* collection, const char* type_name, const char* field_name);
UserTypeMethod* findUserTypeMethod(TypeCollection* collection, const char* type_name, const char* method_name);
int isBuiltinTypeName(const char* type_name);
int getTypeStorageSize(TypeCollection* collection, const char* type_name);
void freeTypeCollection(TypeCollection* collection);

Function* createFunction(FunctionSignature* signature, CFG* cfg, const char* source_file,
    const char* owner_type_name, int is_method, TypeCollection* types);
void freeFunction(Function* func);

FileCollection* createFileCollection(void);
void addFileToCollection(FileCollection* collection, const char* filename, Node* ast);
FunctionCollection* createFunctionCollection(void);
void addFunctionToCollection(FunctionCollection* collection, Function* func);
ErrorCollection* createErrorCollection(void);
void addErrorToCollection(ErrorCollection* collection, const char* message, const char* filename, int line_number);

AnalysisResult* createAnalysisResult(FunctionCollection* functions, ErrorCollection* errors,
    TypeCollection* types);
void freeAnalysisResult(AnalysisResult* result);

AnalysisResult* buildCFGFromAST(FileCollection* file_collection);

#endif // CFG_H
