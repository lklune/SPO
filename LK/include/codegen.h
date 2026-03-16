#ifndef CODEGEN_H
#define CODEGEN_H

#include "cfg.h"
#include <stdint.h>

typedef enum {
    INSTR_MOV,              // MOV dst, src
    INSTR_LOAD_CONST,       // LOAD_CONST reg, const

    INSTR_ADD,              // ADD reg1, reg2
    INSTR_SUB,              // SUB reg1, reg2
    INSTR_MUL,              // MUL reg1, reg2
    INSTR_DIV,              // DIV reg1, reg2
    INSTR_MOD,              // MOD reg1, reg2

    INSTR_AND,              // AND reg1, reg2
    INSTR_OR,               // OR reg1, reg2
    INSTR_XOR,              // XOR reg1, reg2
    INSTR_NOT,              // NOT reg

    INSTR_BIT_AND,          // BIT_AND reg1, reg2
    INSTR_BIT_OR,           // BIT_OR reg1, reg2
    INSTR_BIT_XOR,          // BIT_XOR reg1, reg2
    INSTR_BIT_NOT,          // BIT_NOT reg
    INSTR_SHIFT_LEFT,       // SHIFT_LEFT reg, count
    INSTR_SHIFT_RIGHT,      // SHIFT_RIGHT reg, count

    INSTR_CMP,              // CMP reg1, reg2

    INSTR_JEQ,              // JEQ label
    INSTR_JNE,              // JNE label
    INSTR_JLT,              // JLT label
    INSTR_JGT,              // JGT label
    INSTR_JLE,              // JLE label
    INSTR_JGE,              // JGE label

    INSTR_JMP,              // JMP label

    INSTR_CALL,             // CALL function_name
    INSTR_RET,              // RET value
    INSTR_PUSH,             // PUSH reg
    INSTR_POP,              // POP reg

    INSTR_PRINT,            // PRINT reg
    INSTR_PRINT_STR,        // PRINT_STR string

    INSTR_LABEL,            // label:

    INSTR_END, 
} InstructionType;

typedef struct {
    enum {
        OPERAND_REGISTER,
        OPERAND_VARIABLE,
        OPERAND_CONSTANT,
        OPERAND_LABEL,
        OPERAND_STRING,
    } type;

    union {
        int register_id;
        char* name;
        long value;
    } value;
} Operand;

typedef struct {
    InstructionType type;
    Operand operand1;
    Operand operand2;
    int line_number;
} Instruction;

typedef struct {
    Instruction* instructions;
    int instruction_count;
    int max_instructions;
} LinearCode;

typedef struct {
    char* var_name;
    int register_id;
    int memory_address;
    int is_constant;
    long constant_value;
} VariableBinding;

typedef struct {
    VariableBinding* bindings;
    int binding_count;
    int max_bindings;

    int next_register;
    int next_memory_addr;
    int max_registers;
    int max_memory;
} RegisterAllocator;

typedef struct {
    FunctionSignature* signature;
    LinearCode* code;
    RegisterAllocator* alloc;
    int code_start_line;
} CompiledFunction;

typedef struct {
    CompiledFunction* functions;
    int function_count;
    int max_functions;
} CompiledFunctionCollection;

Operand createRegisterOperand(int reg_id);
Operand createVariableOperand(const char* var_name);
Operand createConstantOperand(long value);
Operand createLabelOperand(const char* label);
Operand createStringOperand(const char* string);

LinearCode* createLinearCode(void);
void addInstruction(LinearCode* code, InstructionType type,
    Operand op1, Operand op2);
void freeLinearCode(LinearCode* code);


RegisterAllocator* createRegisterAllocator(int max_registers, int max_memory);
int allocateRegister(RegisterAllocator* alloc);
void freeRegister(RegisterAllocator* alloc, int reg);
int allocateMemory(RegisterAllocator* alloc);
void bindVariable(RegisterAllocator* alloc, const char* var_name,
    int register_id, int memory_address);
VariableBinding* findVariableBinding(RegisterAllocator* alloc,
    const char* var_name);
void freeRegisterAllocator(RegisterAllocator* alloc);


CompiledFunction* generateCodeFromFunction(Function* cfg_func);
CompiledFunctionCollection* generateCodeFromAST(FunctionCollection* functions);

void freeCompiledFunction(CompiledFunction* func);
void freeCompiledFunctionCollection(CompiledFunctionCollection* collection);

#endif // CODEGEN_H
