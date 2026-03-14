//
// export_code.c - Реализация экспорта линейного кода в мнемонический формат
//

#include "export_code.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* instructionToMnemonic(InstructionType type) {
    switch (type) {
    // Перемещение данных
    case INSTR_MOV:           return "MOV";
    case INSTR_LOAD_CONST:    return "LOAD_CONST";

    // Арифметика
    case INSTR_ADD:           return "ADD";
    case INSTR_SUB:           return "SUB";
    case INSTR_MUL:           return "MUL";
    case INSTR_DIV:           return "DIV";
    case INSTR_MOD:           return "MOD";

    // Логика
    case INSTR_AND:           return "AND";
    case INSTR_OR:            return "OR";
    case INSTR_XOR:           return "XOR";
    case INSTR_NOT:           return "NOT";

    // Побитовые операции
    case INSTR_BIT_AND:       return "BIT_AND";
    case INSTR_BIT_OR:        return "BIT_OR";
    case INSTR_BIT_XOR:       return "BIT_XOR";
    case INSTR_BIT_NOT:       return "BIT_NOT";
    case INSTR_SHIFT_LEFT:    return "SHIFT_LEFT";
    case INSTR_SHIFT_RIGHT:   return "SHIFT_RIGHT";

    // Сравнение
    case INSTR_CMP:           return "CMP";

    // Условные переходы
    case INSTR_JEQ:           return "JEQ";
    case INSTR_JNE:           return "JNE";
    case INSTR_JLT:           return "JLT";
    case INSTR_JGT:           return "JGT";
    case INSTR_JLE:           return "JLE";
    case INSTR_JGE:           return "JGE";

    // Безусловные переходы
    case INSTR_JMP:           return "JMP";

    // Вызовы функций
    case INSTR_CALL:          return "CALL";
    case INSTR_RET:           return "RET";

    // Вывод
    case INSTR_PRINT:         return "PRINT";
    case INSTR_PRINT_STR:     return "PRINT_STR";

    // Метка
    case INSTR_LABEL:         return "LABEL";

    // Завершение
    case INSTR_END:           return "END";

    default:                  return "UNKNOWN";
    }
}

void operandToString(Operand* operand, char* buffer, int buffer_size) {
    if (!operand || !buffer || buffer_size <= 0) return;

    switch (operand->type) {
    case OPERAND_REGISTER:
        snprintf(buffer, buffer_size, "R%d", operand->value.register_id);
        break;

    case OPERAND_VARIABLE:
        snprintf(buffer, buffer_size, "%s",
            operand->value.name ? operand->value.name : "?");
        break;

    case OPERAND_CONSTANT:
        snprintf(buffer, buffer_size, "%ld", operand->value.value);
        break;

    case OPERAND_LABEL:
        snprintf(buffer, buffer_size, "%s",
            operand->value.name ? operand->value.name : "?");
        break;

    case OPERAND_STRING:
        snprintf(buffer, buffer_size, "\"%s\"",
            operand->value.name ? operand->value.name : "");
        break;

    default:
        snprintf(buffer, buffer_size, "?");
    }
}

void exportCompiledFunction(CompiledFunction* func, const char* filepath) {
    if (!func || !filepath) return;

    FILE* f = fopen(filepath, "w");
    if (!f) {
        fprintf(stderr, "Cannot open file: %s\n", filepath);
        return;
    }

    fprintf(f, "==================================================\n");
    fprintf(f, "      COMPILED CODE FOR: %s\n",
        func->signature && func->signature->name ? func->signature->name : "unknown");
    fprintf(f, "==================================================\n\n");

    if (func->signature) {
        fprintf(f, "; Function signature: %s",
            func->signature->name ? func->signature->name : "unknown");

        if (func->signature->return_type) {
            fprintf(f, " -> %s", func->signature->return_type);
        }
        fprintf(f, "\n");

        if (func->signature->args) {
            fprintf(f, "; Parameters: ");
            FunctionArg* arg = func->signature->args;
            int first = 1;
            while (arg) {
                if (!first) fprintf(f, ", ");
                fprintf(f, "%s: %s",
                    arg->name ? arg->name : "?",
                    arg->type ? arg->type : "?");
                first = 0;
                arg = arg->next;
            }
            fprintf(f, "\n");
        }
        fprintf(f, "\n");
    }

    fprintf(f, "; Register allocation:\n");
    if (func->alloc) {
        for (int i = 0; i < func->alloc->binding_count; i++) {
            VariableBinding* binding = &func->alloc->bindings[i];
            fprintf(f, ";   ");
            if (binding->register_id >= 0) {
                fprintf(f, "R%d: %s", binding->register_id, binding->var_name);
            }
            else if (binding->memory_address >= 0) {
                fprintf(f, "MEM[%d]: %s", binding->memory_address, binding->var_name);
            }
            fprintf(f, "\n");
        }
    }
    fprintf(f, "\n");

    // Разделитель
    fprintf(f, "tttttttttttttttttttttttttttttttttttttttttttttttt\n");
    fprintf(f, "             ASSEMBLY LISTING\n");
    fprintf(f, "tttttttttttttttttttttttttttttttttttttttttttttttn\n");

    // Код
    if (func->code) {
        for (int i = 0; i < func->code->instruction_count; i++) {
            Instruction* instr = &func->code->instructions[i];

            // Мнемоника
            fprintf(f, "  %-16s", instructionToMnemonic(instr->type));

            // Операнды
            char op1_str[256] = "", op2_str[256] = "";

            if (instr->operand1.type != OPERAND_CONSTANT || instr->operand1.value.value != 0) {
                operandToString(&instr->operand1, op1_str, sizeof(op1_str));
            }

            if (instr->operand2.type != OPERAND_CONSTANT || instr->operand2.value.value != 0) {
                operandToString(&instr->operand2, op2_str, sizeof(op2_str));
            }

            if (op1_str[0] != '\0') {
                fprintf(f, "%s", op1_str);
                if (op2_str[0] != '\0') {
                    fprintf(f, ", %s", op2_str);
                }
            }

            fprintf(f, "\n");
        }
    }

    // Завершение
    fprintf(f, "\ntttttttttttttttttttttttttttttttttttttttttttttttt\n");
    fprintf(f, "              END OF CODE\n");
    fprintf(f, "tttttttttttttttttttttttttttttttttttttttttttttttttt\n");

    fclose(f);
}

void exportAllCompiledFunctions(CompiledFunctionCollection* collection,
    const char* output_directory) {
    if (!collection || !output_directory) return;

    for (int i = 0; i < collection->function_count; i++) {
        CompiledFunction* func = &collection->functions[i];

        char filepath[1024];
        const char* func_name = func->signature && func->signature->name ?
            func->signature->name : "unknown";

        snprintf(filepath, sizeof(filepath), "%s/%s.linear_code.txt",
            output_directory, func_name);

        printf("Exporting compiled code: %s\n", filepath);
        exportCompiledFunction(func, filepath);
    }
}

void exportCompiledFunctionsToSingleFile(CompiledFunctionCollection* collection,
    const char* filepath) {
    if (!collection || !filepath) return;

    FILE* f = fopen(filepath, "w");
    if (!f) {
        fprintf(stderr, "Cannot open file: %s\n", filepath);
        return;
    }

    for (int i = 0; i < collection->function_count; i++) {
        CompiledFunction* func = &collection->functions[i];

        fprintf(f, "Function: %s\n",
            func->signature && func->signature->name ? func->signature->name : "unknown");

        // Регистры
        fprintf(f, "Registers:\n");
        if (func->alloc) {
            for (int j = 0; j < func->alloc->binding_count; j++) {
                VariableBinding* binding = &func->alloc->bindings[j];
                fprintf(f, "  ");
                if (binding->register_id >= 0) {
                    fprintf(f, "R%d: %s\n", binding->register_id, binding->var_name);
                }
                else if (binding->memory_address >= 0) {
                    fprintf(f, "MEM[%d]: %s\n", binding->memory_address, binding->var_name);
                }
            }
        }
        fprintf(f, "\n");

        // Код
        fprintf(f, "Code:\n");
        if (func->code) {
            for (int j = 0; j < func->code->instruction_count; j++) {
                Instruction* instr = &func->code->instructions[j];

                fprintf(f, "  %04d:  %-16s",
                    j, instructionToMnemonic(instr->type));

                char op1_str[256] = "", op2_str[256] = "";
                operandToString(&instr->operand1, op1_str, sizeof(op1_str));
                operandToString(&instr->operand2, op2_str, sizeof(op2_str));

                if (op1_str[0] != '\0') {
                    fprintf(f, "%s", op1_str);
                    if (op2_str[0] != '\0') {
                        fprintf(f, ", %s", op2_str);
                    }
                }

                fprintf(f, "\n");
            }
        }

        fprintf(f, "\n\n");
    }

    fclose(f);
}