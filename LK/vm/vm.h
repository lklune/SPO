//
// vm.h - Виртуальная машина для выполнения линейного кода
// Динамическая типизация, двухадресный формат, 4 банка памяти
//

#ifndef VM_H
#define VM_H

#include "codegen.h"
#include <stdint.h>

typedef enum {
    VM_TYPE_NONE,
    VM_TYPE_INT,
    VM_TYPE_FLOAT,
    VM_TYPE_STRING,
    VM_TYPE_BOOL
} VMDataType;

// Значение в памяти ВМ (динамический тип)
typedef struct {
    VMDataType type;
    union {
        long int_val;
        double float_val;
        char* string_val;
        int bool_val;
    } value;
} VMValue;

// Банк памяти: code, constants, data, stack
typedef struct {
    // Банк кода: адреса инструкций
    struct {
        int* addresses;         // Адреса инструкций
        int count;
    } code_bank;

    // Банк констант: постоянные значения
    struct {
        VMValue* values;        // Значения констант
        int count;
        int max_count;
    } constants_bank;

    // Банк данных: переменные программы
    struct {
        VMValue* values;        // Значения переменных
        char** var_names;       // Имена переменных
        int count;
        int max_count;
    } data_bank;

    // Стек: для временных значений и вызовов
    struct {
        VMValue* values;        // Значения в стеке
        int pointer;            // Указатель стека
        int size;               // Размер стека
    } stack;
} VMMemory;

typedef struct {
    VMMemory* memory;                   // Память ВМ
    LinearCode* code;                   // Текущий код для выполнения
    int pc;                             // Program Counter (счетчик инструкций)
    int* label_addresses;               // Адреса меток (для переходов)
    char** label_names;                 // имена меток
    int label_count;

    // Состояние для условных переходов
    int compare_result;                 // Результат последней CMP операции
    int running;                        // Флаг выполнения
} VMContext;

VMMemory* vmMemoryCreate(int constants_size, int data_size, int stack_size);
void vmMemoryFree(VMMemory* mem);

// Работа с банком констант
int vmAddConstant(VMMemory* mem, VMValue value);
VMValue* vmGetConstant(VMMemory* mem, int index);

// Работа с банком данных (переменные)
int vmAddVariable(VMMemory* mem, const char* var_name, VMValue value);
int vmFindVariable(VMMemory* mem, const char* var_name);
VMValue* vmGetVariable(VMMemory* mem, const char* var_name);
void vmSetVariable(VMMemory* mem, const char* var_name, VMValue value);

// Работа со стеком
void vmPush(VMMemory* mem, VMValue value);
VMValue vmPop(VMMemory* mem);
VMValue* vmPeek(VMMemory* mem);

// Создание значений
VMValue vmMakeInt(long int_val);
VMValue vmMakeFloat(double float_val);
VMValue vmMakeBool(int bool_val);
VMValue vmMakeString(const char* string_val);
VMValue vmMakeNone(void);

// Конвертация типов
long vmToInt(VMValue val);
double vmToFloat(VMValue val);
int vmToBool(VMValue val);
const char* vmToString(VMValue val);

// Сравнение значений
int vmEqual(VMValue a, VMValue b);
int vmLess(VMValue a, VMValue b);
int vmGreater(VMValue a, VMValue b);

VMContext* vmContextCreate(LinearCode* code, VMMemory* memory);
void vmContextFree(VMContext* ctx);

// Выполнить одну инструкцию
int vmExecuteInstruction(VMContext* ctx);

// Выполнить весь код
int vmRun(VMContext* ctx);

// Получить результат выполнения
VMValue vmGetReturnValue(VMContext* ctx);

#endif // VM_H