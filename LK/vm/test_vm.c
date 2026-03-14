//
// test_vm.c - Тестовая программа для проверки виртуальной машины
// Динамическая типизация, двухадресный формат, 4 банка памяти
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm.h"

// Создать простую программу с двухадресным кодом
LinearCode* createTestProgram1(void) {
    LinearCode* code = createLinearCode();

    // Программа: z = x + y; print(z);
    // где x = 5, y = 3

    // 1. LOAD_CONST R0, 5     (загрузить 5 в R0)
    addInstruction(code, INSTR_LOAD_CONST,
        createRegisterOperand(0),
        createConstantOperand(5));

    // 2. LOAD_CONST R1, 3     (загрузить 3 в R1)
    addInstruction(code, INSTR_LOAD_CONST,
        createRegisterOperand(1),
        createConstantOperand(3));

    // 3. ADD R0, R1           (R0 = R0 + R1)
    addInstruction(code, INSTR_ADD,
        createRegisterOperand(0),
        createRegisterOperand(1));

    // 4. PRINT R0             (вывести R0)
    addInstruction(code, INSTR_PRINT,
        createRegisterOperand(0),
        createConstantOperand(0));

    // 5. RET R0               (вернуть R0)
    addInstruction(code, INSTR_RET,
        createRegisterOperand(0),
        createConstantOperand(0));

    return code;
}

// Программа с умножением и вычитанием
LinearCode* createTestProgram2(void) {
    LinearCode* code = createLinearCode();

    // Программа: y = 10 * 2; z = y - 5; print(z);

    // 1. LOAD_CONST R0, 10
    addInstruction(code, INSTR_LOAD_CONST,
        createRegisterOperand(0),
        createConstantOperand(10));

    // 2. LOAD_CONST R1, 2
    addInstruction(code, INSTR_LOAD_CONST,
        createRegisterOperand(1),
        createConstantOperand(2));

    // 3. MUL R0, R1           (R0 = 10 * 2 = 20)
    addInstruction(code, INSTR_MUL,
        createRegisterOperand(0),
        createRegisterOperand(1));

    // 4. LOAD_CONST R2, 5
    addInstruction(code, INSTR_LOAD_CONST,
        createRegisterOperand(2),
        createConstantOperand(5));

    // 5. SUB R0, R2           (R0 = 20 - 5 = 15)
    addInstruction(code, INSTR_SUB,
        createRegisterOperand(0),
        createRegisterOperand(2));

    // 6. PRINT R0
    addInstruction(code, INSTR_PRINT,
        createRegisterOperand(0),
        createConstantOperand(0));

    // 7. RET R0
    addInstruction(code, INSTR_RET,
        createRegisterOperand(0),
        createConstantOperand(0));

    return code;
}

// Программа с делением
LinearCode* createTestProgram3(void) {
    LinearCode* code = createLinearCode();

    // Программа: a = 100; b = 4; c = a / b; print(c);

    // 1. LOAD_CONST R0, 100
    addInstruction(code, INSTR_LOAD_CONST,
        createRegisterOperand(0),
        createConstantOperand(100));

    // 2. LOAD_CONST R1, 4
    addInstruction(code, INSTR_LOAD_CONST,
        createRegisterOperand(1),
        createConstantOperand(4));

    // 3. DIV R0, R1           (R0 = 100 / 4 = 25)
    addInstruction(code, INSTR_DIV,
        createRegisterOperand(0),
        createRegisterOperand(1));

    // 4. PRINT R0
    addInstruction(code, INSTR_PRINT,
        createRegisterOperand(0),
        createConstantOperand(0));

    // 5. RET R0
    addInstruction(code, INSTR_RET,
        createRegisterOperand(0),
        createConstantOperand(0));

    return code;
}

void testVMMemory(void) {
    printf("\n=== TEST: VM Memory Management ===\n");

    VMMemory* mem = vmMemoryCreate(256, 256, 512);

    // Тест добавления констант
    int const_idx = vmAddConstant(mem, vmMakeInt(42));
    printf("Added constant 42 at index: %d\n", const_idx);

    VMValue* const_val = vmGetConstant(mem, const_idx);
    printf("Retrieved constant: %ld\n", const_val->value.int_val);

    // Тест переменных
    vmAddVariable(mem, "x", vmMakeInt(10));
    vmAddVariable(mem, "y", vmMakeString("hello"));

    VMValue* x_val = vmGetVariable(mem, "x");
    VMValue* y_val = vmGetVariable(mem, "y");

    printf("Variable x: %ld\n", x_val->value.int_val);
    printf("Variable y: %s\n", y_val->value.string_val);

    // Тест стека
    vmPush(mem, vmMakeInt(100));
    vmPush(mem, vmMakeInt(200));

    VMValue pop1 = vmPop(mem);
    VMValue pop2 = vmPop(mem);

    printf("Popped from stack: %ld, %ld\n", pop2.value.int_val, pop1.value.int_val);

    vmMemoryFree(mem);
    printf("Memory test passed!\n");
}

void testVMTyping(void) {
    printf("\n=== TEST: Dynamic Typing ===\n");

    // Тест типизации
    VMValue int_val = vmMakeInt(42);
    VMValue float_val = vmMakeFloat(3.14);
    VMValue bool_val = vmMakeBool(1);
    VMValue str_val = vmMakeString("test");

    printf("Int -> Float: %.2f\n", vmToFloat(int_val));
    printf("Float -> Int: %ld\n", vmToInt(float_val));
    printf("Bool -> Int: %ld\n", vmToInt(bool_val));
    printf("String -> Int: %ld\n", vmToInt(str_val));

    printf("Typing test passed!\n");
}

void testVMComparison(void) {
    printf("\n=== TEST: Value Comparison ===\n");

    VMValue a = vmMakeInt(10);
    VMValue b = vmMakeInt(10);
    VMValue c = vmMakeInt(5);

    printf("10 == 10: %d (expected 1)\n", vmEqual(a, b));
    printf("10 < 5: %d (expected 0)\n", vmLess(a, c));
    printf("10 > 5: %d (expected 1)\n", vmGreater(a, c));

    printf("Comparison test passed!\n");
}

void runTestProgram(int program_num) {
    printf("\n=== TEST: Program %d ===\n", program_num);

    LinearCode* code = NULL;
    const char* description = "";

    switch (program_num) {
    case 1:
        code = createTestProgram1();
        description = "Addition: 5 + 3";
        break;
    case 2:
        code = createTestProgram2();
        description = "Multiplication and subtraction: 10 * 2 - 5";
        break;
    case 3:
        code = createTestProgram3();
        description = "Division: 100 / 4";
        break;
    default:
        printf("Unknown program\n");
        return;
    }

    printf("Description: %s\n", description);
    printf("Executing...\n");

    // Создать память и контекст
    VMMemory* mem = vmMemoryCreate(256, 256, 512);
    VMContext* ctx = vmContextCreate(code, mem);

    // Выполнить программу
    vmRun(ctx);

    // Вывести результат
    VMValue result = vmGetReturnValue(ctx);
    printf("Result type: %d, Value: %ld\n", result.type, result.value.int_val);

    // Очистка
    vmContextFree(ctx);
    vmMemoryFree(mem);
    freeLinearCode(code);

    printf("Program %d test passed!\n", program_num);
}

int main(void) {
    printf("========================================\n");
    printf("    VIRTUAL MACHINE TESTING SUITE\n");
    printf("    Assignment 3: Dynamic Typing\n");
    printf("    Two-Address Format Code\n");
    printf("    Four Memory Banks\n");
    printf("========================================\n");

    // Тесты управления памятью
    testVMMemory();

    // Тесты типизации
    testVMTyping();

    // Тесты сравнения
    testVMComparison();

    // Запуск тестовых программ
    runTestProgram(1);
    runTestProgram(2);
    runTestProgram(3);

    printf("\n========================================\n");
    printf("    ALL TESTS COMPLETED\n");
    printf("========================================\n");

    return 0;
}