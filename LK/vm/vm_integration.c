//
// vm_integration.c - Интеграция виртуальной машины с основным компилятором
// Задание 3: Динамическая типизация, двухадресный формат, 4 банка памяти
//

#include "vm.h"
#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>

// ============================================================================
// Функции интеграции ВМ и компилятора
// ============================================================================

/**
 * Инициализировать ВМ для выполнения скомпилированной функции
 */
VMContext* vmInitializeForCompiledFunction(CompiledFunction* compiled) {
    if (!compiled || !compiled->code) return NULL;

    // Создать память ВМ
    VMMemory* mem = vmMemoryCreate(256, 256, 512);
    if (!mem) return NULL;

    // Инициализировать переменные из allocator
    if (compiled->alloc) {
        for (int i = 0; i < compiled->alloc->binding_count; i++) {
            VariableBinding* binding = &compiled->alloc->bindings[i];

            // Добавить переменную в банк данных
            VMValue init_val = vmMakeInt(0);
            if (binding->is_constant) {
                init_val = vmMakeInt(binding->constant_value);
            }
            vmAddVariable(mem, binding->var_name, init_val);
        }
    }

    // Создать контекст выполнения
    VMContext* ctx = vmContextCreate(compiled->code, mem);
    return ctx;
}

/**
 * Выполнить скомпилированную функцию и получить результат
 */
VMValue vmExecuteCompiledFunction(CompiledFunction* compiled) {
    if (!compiled) return vmMakeNone();

    // Инициализировать ВМ
    VMContext* ctx = vmInitializeForCompiledFunction(compiled);
    if (!ctx) return vmMakeNone();

    // Выполнить программу
    vmRun(ctx);

    // Получить результат
    VMValue result = vmGetReturnValue(ctx);

    // Очистка
    VMMemory* mem = ctx->memory;
    vmContextFree(ctx);
    vmMemoryFree(mem);

    return result;
}

/**
 * Выполнить все скомпилированные функции из коллекции
 */
void vmExecuteAllCompiledFunctions(CompiledFunctionCollection* collection) {
    if (!collection) return;

    printf("\n======================================================\n");
    printf("         EXECUTING COMPILED FUNCTIONS\n");
    printf("======================================================\n\n");

    for (int i = 0; i < collection->function_count; i++) {
        CompiledFunction* func = &collection->functions[i];
        const char* func_name = func->signature && func->signature->name ?
            func->signature->name : "unknown";

        printf("Executing function: %s\n", func_name);
        printf("Code size: %d instructions\n", func->code->instruction_count);

        // Выполнить функцию
        VMValue result = vmExecuteCompiledFunction(func);

        // Вывести результат
        printf("Result type: %d\n", result.type);
        printf("Result value: %s\n", vmToString(result));
        printf("---\n\n");
    }

    printf("======================================================\n");
    printf("         EXECUTION COMPLETED\n");
    printf("======================================================\n\n");
}

/**
 * Вывести статистику выполнения
 */
void vmPrintExecutionStatistics(VMContext* ctx) {
    if (!ctx) return;

    printf("\nExecution Statistics:\n");
    printf("  Program Counter: %d\n", ctx->pc);
    printf("  Instructions executed: %d\n", ctx->pc);
    if (ctx->code) {
        printf("  Total instructions: %d\n", ctx->code->instruction_count);
    }
    printf("  Stack pointer: %d\n", ctx->memory->stack.pointer);
}

/**
 * Вывести содержимое памяти ВМ
 */
void vmPrintMemoryState(VMMemory* mem) {
    if (!mem) return;

    printf("\n=== MEMORY STATE ===\n");

    // Банк констант
    printf("\nConstants Bank (%d items):\n", mem->constants_bank.count);
    for (int i = 0; i < mem->constants_bank.count; i++) {
        VMValue* val = &mem->constants_bank.values[i];
        printf("  [%d]: %s (type: %d)\n", i, vmToString(*val), val->type);
    }

    // Банк данных (переменные)
    printf("\nData Bank (%d variables):\n", mem->data_bank.count);
    for (int i = 0; i < mem->data_bank.count; i++) {
        printf("  %s = %s (type: %d)\n",
            mem->data_bank.var_names[i],
            vmToString(mem->data_bank.values[i]),
            mem->data_bank.values[i].type);
    }

    // Стек
    printf("\nStack (pointer: %d/%d):\n", mem->stack.pointer, mem->stack.size);
    for (int i = 0; i < mem->stack.pointer; i++) {
        printf("  [%d]: %s (type: %d)\n",
            i,
            vmToString(mem->stack.values[i]),
            mem->stack.values[i].type);
    }
}

/**
 * Вывести код программы в удобном формате
 */
void vmPrintProgramCode(LinearCode* code) {
    if (!code) return;

    printf("\n=== PROGRAM CODE (Linear Format) ===\n");
    printf("Total instructions: %d\n\n", code->instruction_count);

    for (int i = 0; i < code->instruction_count; i++) {
        Instruction* instr = &code->instructions[i];
        printf("[%04d] %-16s", i, instructionToMnemonic(instr->type));

        // Операнд 1
        char op1_str[128] = "", op2_str[128] = "";
        operandToString(&instr->operand1, op1_str, sizeof(op1_str));
        operandToString(&instr->operand2, op2_str, sizeof(op2_str));

        if (op1_str[0] != '\0') {
            printf("%s", op1_str);
            if (op2_str[0] != '\0') {
                printf(", %s", op2_str);
            }
        }

        printf("\n");
    }
}

// ============================================================================
// Отладочные функции
// ============================================================================

/**
 * Выполнить программу пошагово с выводом отладки
 */
int vmDebugRun(VMContext* ctx) {
    if (!ctx) return 0;

    printf("\n=== DEBUG EXECUTION ===\n\n");

    int step = 0;
    while (ctx->running && ctx->pc < ctx->code->instruction_count) {
        Instruction* instr = &ctx->code->instructions[ctx->pc];

        printf("Step %d [PC=%d]: ", step++, ctx->pc);
        printf("%-16s", instructionToMnemonic(instr->type));

        // Вывести операнды
        char op1_str[128] = "", op2_str[128] = "";
        operandToString(&instr->operand1, op1_str, sizeof(op1_str));
        operandToString(&instr->operand2, op2_str, sizeof(op2_str));

        if (op1_str[0] != '\0') {
            printf("%s", op1_str);
            if (op2_str[0] != '\0') {
                printf(", %s", op2_str);
            }
        }
        printf("\n");

        // Выполнить инструкцию
        vmExecuteInstruction(ctx);

        // Вывести состояние памяти после инструкции
        if (ctx->memory->data_bank.count > 0) {
            printf("  Registers: ");
            for (int i = 0; i < ctx->memory->data_bank.count && i < 3; i++) {
                printf("%s=%s ",
                    ctx->memory->data_bank.var_names[i],
                    vmToString(ctx->memory->data_bank.values[i]));
            }
            printf("\n");
        }
    }

    printf("\n=== EXECUTION COMPLETED ===\n\n");
    return 1;
}

/**
 * Проверить и вывести тип данных значения
 */
const char* vmTypeToString(VMDataType type) {
    switch (type) {
    case VM_TYPE_INT:
        return "INT";
    case VM_TYPE_FLOAT:
        return "FLOAT";
    case VM_TYPE_STRING:
        return "STRING";
    case VM_TYPE_BOOL:
        return "BOOL";
    case VM_TYPE_NONE:
        return "NONE";
    default:
        return "UNKNOWN";
    }
}