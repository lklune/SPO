//
// codegen.h - Генерация линейного кода из CFG
//

#ifndef CODEGEN_H
#define CODEGEN_H

#include "cfg.h"
#include <stdint.h>

typedef enum {
    // Перемещение данных
    INSTR_MOV,              // MOV dst, src
    INSTR_LOAD_CONST,       // LOAD_CONST reg, const

    // Арифметические операции
    INSTR_ADD,              // ADD reg1, reg2
    INSTR_SUB,              // SUB reg1, reg2
    INSTR_MUL,              // MUL reg1, reg2
    INSTR_DIV,              // DIV reg1, reg2
    INSTR_MOD,              // MOD reg1, reg2

    // Логические операции
    INSTR_AND,              // AND reg1, reg2
    INSTR_OR,               // OR reg1, reg2
    INSTR_XOR,              // XOR reg1, reg2
    INSTR_NOT,              // NOT reg

    // Побитовые операции
    INSTR_BIT_AND,          // BIT_AND reg1, reg2
    INSTR_BIT_OR,           // BIT_OR reg1, reg2
    INSTR_BIT_XOR,          // BIT_XOR reg1, reg2
    INSTR_BIT_NOT,          // BIT_NOT reg
    INSTR_SHIFT_LEFT,       // SHIFT_LEFT reg, count
    INSTR_SHIFT_RIGHT,      // SHIFT_RIGHT reg, count

    // Сравнение
    INSTR_CMP,              // CMP reg1, reg2

    // Условные переходы
    INSTR_JEQ,              // JEQ label (если равно)
    INSTR_JNE,              // JNE label (если не равно)
    INSTR_JLT,              // JLT label (если <)
    INSTR_JGT,              // JGT label (если >)
    INSTR_JLE,              // JLE label (если <=)
    INSTR_JGE,              // JGE label (если >=)

    // Безусловный переход
    INSTR_JMP,              // JMP label

    // Вызовы функций
    INSTR_CALL,             // CALL function_name
    INSTR_RET,              // RET value

    // Вывод
    INSTR_PRINT,            // PRINT reg
    INSTR_PRINT_STR,        // PRINT_STR string

    // Метка (не инструкция, но используется для организации кода)
    INSTR_LABEL,            // label:

    // Завершение
    INSTR_END,              // конец кода
} InstructionType;

typedef struct {
    enum {
        OPERAND_REGISTER,   // R0, R1, ...
        OPERAND_VARIABLE,   // имя переменной
        OPERAND_CONSTANT,   // число
        OPERAND_LABEL,      // метка
        OPERAND_STRING,     // строка
    } type;

    union {
        int register_id;    // для OPERAND_REGISTER
        char* name;         // для OPERAND_VARIABLE или OPERAND_LABEL
        long value;         // для OPERAND_CONSTANT
    } value;
} Operand;

// Инструкция ВМ
typedef struct {
    InstructionType type;
    Operand operand1;       // Первый операнд (опционально)
    Operand operand2;       // Второй операнд (опционально)
    int line_number;        // Номер строки в исходном коде
} Instruction;

// Линейный код
typedef struct {
    Instruction* instructions;
    int instruction_count;
    int max_instructions;
} LinearCode;

// Привязка переменной к регистру или памяти
typedef struct {
    char* var_name;         // Имя переменной
    int register_id;        // ID регистра (-1 если в памяти)
    int memory_address;     // Адрес в памяти (-1 если в регистре)
    int is_constant;        // 1 если константа
    long constant_value;    // Значение константы
} VariableBinding;

// Распределение регистров
typedef struct {
    VariableBinding* bindings;
    int binding_count;
    int max_bindings;

    int next_register;      // Следующий свободный регистр
    int next_memory_addr;   // Следующий свободный адрес памяти
    int max_registers;      // Максимум регистров
    int max_memory;         // Максимум памяти
} RegisterAllocator;

typedef struct {
    FunctionSignature* signature;   // Сигнатура из CFG
    LinearCode* code;               // Сгенерированный линейный код
    RegisterAllocator* alloc;       // Информация о регистрах и памяти
    int code_start_line;            // Строка начала кода
} CompiledFunction;

typedef struct {
    CompiledFunction* functions;
    int function_count;
    int max_functions;
} CompiledFunctionCollection;

// Создание операндов
Operand createRegisterOperand(int reg_id);
Operand createVariableOperand(const char* var_name);
Operand createConstantOperand(long value);
Operand createLabelOperand(const char* label);
Operand createStringOperand(const char* string);

// Линейный код
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