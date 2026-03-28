#include "export_code.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static VariableBinding* findBindingByName(RegisterAllocator* alloc, const char* name) {
    if (!alloc || !name) {
        return NULL;
    }

    for (int i = 0; i < alloc->binding_count; i++) {
        VariableBinding* binding = &alloc->bindings[i];
        if (binding->var_name && strcmp(binding->var_name, name) == 0) {
            return binding;
        }
    }

    return NULL;
}

static void formatAsmOperand(Operand* operand,
    RegisterAllocator* alloc,
    char* buffer,
    int buffer_size) {
    if (!operand || !buffer || buffer_size <= 0) {
        return;
    }

    buffer[0] = '\0';

    switch (operand->type) {
    case OPERAND_REGISTER:
        snprintf(buffer, (size_t)buffer_size, "r%d", operand->value.register_id);
        break;
    case OPERAND_CONSTANT:
        snprintf(buffer, (size_t)buffer_size, "%ld", operand->value.value);
        break;
    case OPERAND_LABEL:
        snprintf(buffer, (size_t)buffer_size, "%s", operand->value.name ? operand->value.name : "?");
        break;
    case OPERAND_VARIABLE:
    {
        VariableBinding* binding = findBindingByName(alloc, operand->value.name);
        if (binding && binding->memory_address >= 0) {
            snprintf(buffer, (size_t)buffer_size, "%d", binding->memory_address);
        }
        else {
            snprintf(buffer, (size_t)buffer_size, "0");
        }
        break;
    }
    case OPERAND_STRING:
        snprintf(buffer, (size_t)buffer_size, "\"%s\"", operand->value.name ? operand->value.name : "");
        break;
    default:
        snprintf(buffer, (size_t)buffer_size, "?");
        break;
    }
}

static void printAsmInstruction(FILE* f, Instruction* instr, RegisterAllocator* alloc) {
    char op1[256] = "";
    char op2[256] = "";

    if (!f || !instr) {
        return;
    }

    if (instr->type == INSTR_LABEL) {
        formatAsmOperand(&instr->operand1, alloc, op1, (int)sizeof(op1));
        fprintf(f, "%s:\n", op1);
        return;
    }

    formatAsmOperand(&instr->operand1, alloc, op1, (int)sizeof(op1));
    formatAsmOperand(&instr->operand2, alloc, op2, (int)sizeof(op2));

    switch (instr->type) {
    case INSTR_MOV:
        if (instr->operand1.type == OPERAND_VARIABLE && instr->operand2.type == OPERAND_REGISTER) {
            fprintf(f, "ST %s, %s\n", op2, op1);
        }
        else if (instr->operand1.type == OPERAND_REGISTER && instr->operand2.type == OPERAND_VARIABLE) {
            fprintf(f, "LD %s, %s\n", op2, op1);
        }
        else {
            fprintf(f, "MOV %s, %s\n", op2, op1);
        }
        break;

    case INSTR_LOAD_CONST:
        fprintf(f, "MOV %s, %s\n", op2, op1);
        break;

    case INSTR_ADD:
        fprintf(f, "ADD %s, %s\n", op1, op2);
        break;
    case INSTR_SUB:
        fprintf(f, "SUB %s, %s\n", op1, op2);
        break;
    case INSTR_MUL:
        fprintf(f, "MUL %s, %s\n", op1, op2);
        break;
    case INSTR_DIV:
        fprintf(f, "DIV %s, %s\n", op1, op2);
        break;
    case INSTR_MOD:
        fprintf(f, "MOD %s, %s\n", op1, op2);
        break;
    case INSTR_AND:
        fprintf(f, "AND %s, %s\n", op1, op2);
        break;
    case INSTR_OR:
        fprintf(f, "OR %s, %s\n", op1, op2);
        break;
    case INSTR_XOR:
        fprintf(f, "XOR %s, %s\n", op1, op2);
        break;
    case INSTR_BIT_AND:
        fprintf(f, "BIT_AND %s, %s\n", op1, op2);
        break;
    case INSTR_BIT_OR:
        fprintf(f, "BIT_OR %s, %s\n", op1, op2);
        break;
    case INSTR_BIT_XOR:
        fprintf(f, "BIT_XOR %s, %s\n", op1, op2);
        break;
    case INSTR_SHIFT_LEFT:
        fprintf(f, "SHIFT_LEFT %s, %s\n", op1, op2);
        break;
    case INSTR_SHIFT_RIGHT:
        fprintf(f, "SHIFT_RIGHT %s, %s\n", op1, op2);
        break;
    case INSTR_NOT:
        fprintf(f, "NOT %s\n", op1);
        break;
    case INSTR_BIT_NOT:
        fprintf(f, "BIT_NOT %s\n", op1);
        break;
    case INSTR_CMP:
        fprintf(f, "CMP %s, %s\n", op1, op2);
        break;
    case INSTR_JEQ:
        fprintf(f, "JEQ %s\n", op1);
        break;
    case INSTR_JNE:
        fprintf(f, "JNE %s\n", op1);
        break;
    case INSTR_JLT:
        fprintf(f, "JLT %s\n", op1);
        break;
    case INSTR_JGT:
        fprintf(f, "JGT %s\n", op1);
        break;
    case INSTR_JLE:
        fprintf(f, "JLE %s\n", op1);
        break;
    case INSTR_JGE:
        fprintf(f, "JGE %s\n", op1);
        break;
    case INSTR_JMP:
        fprintf(f, "JMP %s\n", op1);
        break;
    case INSTR_CALL:
        fprintf(f, "CALL %s\n", op1);
        break;
    case INSTR_PUSH:
        fprintf(f, "PUSH %s\n", op1);
        break;
    case INSTR_POP:
        fprintf(f, "POP %s\n", op1);
        break;
    case INSTR_PRINT:
        fprintf(f, "OUT %s\n", op1);
        break;
    case INSTR_PRINT_STR:
        fprintf(f, "; unsupported PRINT_STR %s\n", op1);
        break;
    case INSTR_RET:
        if (instr->operand1.type == OPERAND_REGISTER && instr->operand1.value.register_id != 0) {
            fprintf(f, "MOV %s, r0\n", op1);
        }
        else if (instr->operand1.type == OPERAND_VARIABLE) {
            fprintf(f, "LD %s, r0\n", op1);
        }
        else if (instr->operand1.type == OPERAND_CONSTANT) {
            fprintf(f, "MOV %s, r0\n", op1);
        }
        fprintf(f, "RET\n");
        break;
    case INSTR_END:
        fprintf(f, "END\n");
        break;
    default:
        fprintf(f, "; unsupported instruction %s\n", instructionToMnemonic(instr->type));
        break;
    }
}

static int instructionOperandCount(InstructionType type) {
    switch (type) {
    case INSTR_END:
        return 0;
    case INSTR_LABEL:
    case INSTR_JEQ:
    case INSTR_JNE:
    case INSTR_JLT:
    case INSTR_JGT:
    case INSTR_JLE:
    case INSTR_JGE:
    case INSTR_JMP:
    case INSTR_CALL:
    case INSTR_PUSH:
    case INSTR_POP:
    case INSTR_RET:
    case INSTR_PRINT:
    case INSTR_PRINT_STR:
    case INSTR_NOT:
    case INSTR_BIT_NOT:
        return 1;
    default:
        return 2;
    }
}

const char* instructionToMnemonic(InstructionType type) {
    switch (type) {
    case INSTR_MOV:         return "MOV";
    case INSTR_LOAD_CONST:  return "LOAD_CONST";
    case INSTR_ADD:         return "ADD";
    case INSTR_SUB:         return "SUB";
    case INSTR_MUL:         return "MUL";
    case INSTR_DIV:         return "DIV";
    case INSTR_MOD:         return "MOD";
    case INSTR_AND:         return "AND";
    case INSTR_OR:          return "OR";
    case INSTR_XOR:         return "XOR";
    case INSTR_NOT:         return "NOT";
    case INSTR_BIT_AND:     return "BIT_AND";
    case INSTR_BIT_OR:      return "BIT_OR";
    case INSTR_BIT_XOR:     return "BIT_XOR";
    case INSTR_BIT_NOT:     return "BIT_NOT";
    case INSTR_SHIFT_LEFT:  return "SHIFT_LEFT";
    case INSTR_SHIFT_RIGHT: return "SHIFT_RIGHT";
    case INSTR_CMP:         return "CMP";
    case INSTR_JEQ:         return "JEQ";
    case INSTR_JNE:         return "JNE";
    case INSTR_JLT:         return "JLT";
    case INSTR_JGT:         return "JGT";
    case INSTR_JLE:         return "JLE";
    case INSTR_JGE:         return "JGE";
    case INSTR_JMP:         return "JMP";
    case INSTR_CALL:        return "CALL";
    case INSTR_RET:         return "RET";
    case INSTR_PUSH:        return "PUSH";
    case INSTR_POP:         return "POP";
    case INSTR_PRINT:       return "PRINT";
    case INSTR_PRINT_STR:   return "PRINT_STR";
    case INSTR_LABEL:       return "LABEL";
    case INSTR_END:         return "END";
    default:                return "UNKNOWN";
    }
}

void operandToString(Operand* operand, char* buffer, int buffer_size) {
    if (!operand || !buffer || buffer_size <= 0) {
        return;
    }

    switch (operand->type) {
    case OPERAND_REGISTER:
        snprintf(buffer, (size_t)buffer_size, "R%d", operand->value.register_id);
        break;
    case OPERAND_VARIABLE:
    case OPERAND_LABEL:
        snprintf(buffer, (size_t)buffer_size, "%s", operand->value.name ? operand->value.name : "?");
        break;
    case OPERAND_CONSTANT:
        snprintf(buffer, (size_t)buffer_size, "%ld", operand->value.value);
        break;
    case OPERAND_STRING:
        snprintf(buffer, (size_t)buffer_size, "\"%s\"", operand->value.name ? operand->value.name : "");
        break;
    default:
        snprintf(buffer, (size_t)buffer_size, "?");
        break;
    }
}

static void printInstruction(FILE* f, Instruction* instr) {
    char op1[256] = "";
    char op2[256] = "";
    int op_count = instructionOperandCount(instr->type);

    fprintf(f, "  %-16s", instructionToMnemonic(instr->type));

    if (op_count >= 1) {
        operandToString(&instr->operand1, op1, (int)sizeof(op1));
        fprintf(f, "%s", op1);
    }
    if (op_count >= 2) {
        operandToString(&instr->operand2, op2, (int)sizeof(op2));
        fprintf(f, ", %s", op2);
    }
    fprintf(f, "\n");
}

static void appendRuntime(FILE* f) {
    if (!f) {
        return;
    }

    fprintf(f, "\nreadByte:\n");
    fprintf(f, "\tIN r0\n");
    fprintf(f, "\tRET\n");

    fprintf(f, "\nwriteByte:\n");
    fprintf(f, "\tOUT r0\n");
    fprintf(f, "\tRET\n");

    fprintf(f, "\nwriteInt:\n");
    fprintf(f, "\tMOV 268435455, r3\n");
    fprintf(f, "\tBIT_AND r0, r3\n");
    fprintf(f, "\tPUSH r0\n");
    fprintf(f, "\tMOV r0, r1\n");
    fprintf(f, "\tMOV 10, r2\n");
    fprintf(f, "\tDIV r1, r2\n");
    fprintf(f, "\tCMP r1, 0\n");
    fprintf(f, "\tJEQ writeInt_ones\n");
    fprintf(f, "\tMOV r1, r0\n");
    fprintf(f, "\tCALL writeInt\n");
    fprintf(f, "writeInt_ones:\n");
    fprintf(f, "\tPOP r0\n");
    fprintf(f, "\tMOV r0, r1\n");
    fprintf(f, "\tMOV 10, r2\n");
    fprintf(f, "\tMOD r1, r2\n");
    fprintf(f, "\tMOV r1, r0\n");
    fprintf(f, "\tCALL writeDigit\n");
    fprintf(f, "\tRET\n");

    fprintf(f, "\nwriteDigit:\n");
    fprintf(f, "\tMOV 48, r1\n");
    fprintf(f, "\tADD r0, r1\n");
    fprintf(f, "\tCALL writeByte\n");
    fprintf(f, "\tRET\n");

    fprintf(f, "\nreadOp:\n");
    fprintf(f, "\tCALL readByte\n");
    fprintf(f, "\tMOV r0, r1\n");
    fprintf(f, "readOp_skip:\n");
    fprintf(f, "\tCMP r1, 32\n");
    fprintf(f, "\tJEQ readOp_next\n");
    fprintf(f, "\tCMP r1, 10\n");
    fprintf(f, "\tJEQ readOp_next\n");
    fprintf(f, "\tCMP r1, 13\n");
    fprintf(f, "\tJEQ readOp_next\n");
    fprintf(f, "\tMOV r1, r0\n");
    fprintf(f, "\tRET\n");
    fprintf(f, "readOp_next:\n");
    fprintf(f, "\tCALL readByte\n");
    fprintf(f, "\tMOV r0, r1\n");
    fprintf(f, "\tJMP readOp_skip\n");

    fprintf(f, "\nreadDigit:\n");
    fprintf(f, "\tCALL readByte\n");
    fprintf(f, "\tMOV r0, r1\n");
    fprintf(f, "readDigit_skip:\n");
    fprintf(f, "\tCMP r1, 32\n");
    fprintf(f, "\tJEQ readDigit_next\n");
    fprintf(f, "\tCMP r1, 10\n");
    fprintf(f, "\tJEQ readDigit_next\n");
    fprintf(f, "\tCMP r1, 13\n");
    fprintf(f, "\tJEQ readDigit_next\n");
    fprintf(f, "\tMOV r1, r0\n");
    fprintf(f, "\tRET\n");
    fprintf(f, "readDigit_next:\n");
    fprintf(f, "\tCALL readByte\n");
    fprintf(f, "\tMOV r0, r1\n");
    fprintf(f, "\tJMP readDigit_skip\n");

    fprintf(f, "\nreadInt:\n");
    fprintf(f, "readInt_skip:\n");
    fprintf(f, "\tCALL readByte\n");
    fprintf(f, "\tMOV r0, r1\n");
    fprintf(f, "\tCMP r1, 32\n");
    fprintf(f, "\tJEQ readInt_skip\n");
    fprintf(f, "\tCMP r1, 10\n");
    fprintf(f, "\tJEQ readInt_skip\n");
    fprintf(f, "\tCMP r1, 13\n");
    fprintf(f, "\tJEQ readInt_skip\n");
    fprintf(f, "\tMOV 0, r2\n");
    fprintf(f, "readInt_loop:\n");
    fprintf(f, "\tCMP r1, 48\n");
    fprintf(f, "\tJEQ readInt_digit_0\n");
    fprintf(f, "\tCMP r1, 49\n");
    fprintf(f, "\tJEQ readInt_digit_1\n");
    fprintf(f, "\tCMP r1, 50\n");
    fprintf(f, "\tJEQ readInt_digit_2\n");
    fprintf(f, "\tCMP r1, 51\n");
    fprintf(f, "\tJEQ readInt_digit_3\n");
    fprintf(f, "\tCMP r1, 52\n");
    fprintf(f, "\tJEQ readInt_digit_4\n");
    fprintf(f, "\tCMP r1, 53\n");
    fprintf(f, "\tJEQ readInt_digit_5\n");
    fprintf(f, "\tCMP r1, 54\n");
    fprintf(f, "\tJEQ readInt_digit_6\n");
    fprintf(f, "\tCMP r1, 55\n");
    fprintf(f, "\tJEQ readInt_digit_7\n");
    fprintf(f, "\tCMP r1, 56\n");
    fprintf(f, "\tJEQ readInt_digit_8\n");
    fprintf(f, "\tCMP r1, 57\n");
    fprintf(f, "\tJEQ readInt_digit_9\n");
    fprintf(f, "\tJMP readInt_done\n");
    fprintf(f, "readInt_digit_0:\n");
    fprintf(f, "\tMOV r2, r0\n");
    fprintf(f, "\tMOV 10, r3\n");
    fprintf(f, "\tMUL r0, r3\n");
    fprintf(f, "\tMOV r0, r2\n");
    fprintf(f, "\tJMP readInt_next\n");
    fprintf(f, "readInt_digit_1:\n");
    fprintf(f, "\tMOV r2, r0\n");
    fprintf(f, "\tMOV 10, r3\n");
    fprintf(f, "\tMUL r0, r3\n");
    fprintf(f, "\tMOV r0, r2\n");
    fprintf(f, "\tMOV 1, r0\n");
    fprintf(f, "\tADD r2, r0\n");
    fprintf(f, "\tJMP readInt_next\n");
    fprintf(f, "readInt_digit_2:\n");
    fprintf(f, "\tMOV r2, r0\n");
    fprintf(f, "\tMOV 10, r3\n");
    fprintf(f, "\tMUL r0, r3\n");
    fprintf(f, "\tMOV r0, r2\n");
    fprintf(f, "\tMOV 2, r0\n");
    fprintf(f, "\tADD r2, r0\n");
    fprintf(f, "\tJMP readInt_next\n");
    fprintf(f, "readInt_digit_3:\n");
    fprintf(f, "\tMOV r2, r0\n");
    fprintf(f, "\tMOV 10, r3\n");
    fprintf(f, "\tMUL r0, r3\n");
    fprintf(f, "\tMOV r0, r2\n");
    fprintf(f, "\tMOV 3, r0\n");
    fprintf(f, "\tADD r2, r0\n");
    fprintf(f, "\tJMP readInt_next\n");
    fprintf(f, "readInt_digit_4:\n");
    fprintf(f, "\tMOV r2, r0\n");
    fprintf(f, "\tMOV 10, r3\n");
    fprintf(f, "\tMUL r0, r3\n");
    fprintf(f, "\tMOV r0, r2\n");
    fprintf(f, "\tMOV 4, r0\n");
    fprintf(f, "\tADD r2, r0\n");
    fprintf(f, "\tJMP readInt_next\n");
    fprintf(f, "readInt_digit_5:\n");
    fprintf(f, "\tMOV r2, r0\n");
    fprintf(f, "\tMOV 10, r3\n");
    fprintf(f, "\tMUL r0, r3\n");
    fprintf(f, "\tMOV r0, r2\n");
    fprintf(f, "\tMOV 5, r0\n");
    fprintf(f, "\tADD r2, r0\n");
    fprintf(f, "\tJMP readInt_next\n");
    fprintf(f, "readInt_digit_6:\n");
    fprintf(f, "\tMOV r2, r0\n");
    fprintf(f, "\tMOV 10, r3\n");
    fprintf(f, "\tMUL r0, r3\n");
    fprintf(f, "\tMOV r0, r2\n");
    fprintf(f, "\tMOV 6, r0\n");
    fprintf(f, "\tADD r2, r0\n");
    fprintf(f, "\tJMP readInt_next\n");
    fprintf(f, "readInt_digit_7:\n");
    fprintf(f, "\tMOV r2, r0\n");
    fprintf(f, "\tMOV 10, r3\n");
    fprintf(f, "\tMUL r0, r3\n");
    fprintf(f, "\tMOV r0, r2\n");
    fprintf(f, "\tMOV 7, r0\n");
    fprintf(f, "\tADD r2, r0\n");
    fprintf(f, "\tJMP readInt_next\n");
    fprintf(f, "readInt_digit_8:\n");
    fprintf(f, "\tMOV r2, r0\n");
    fprintf(f, "\tMOV 10, r3\n");
    fprintf(f, "\tMUL r0, r3\n");
    fprintf(f, "\tMOV r0, r2\n");
    fprintf(f, "\tMOV 8, r0\n");
    fprintf(f, "\tADD r2, r0\n");
    fprintf(f, "\tJMP readInt_next\n");
    fprintf(f, "readInt_digit_9:\n");
    fprintf(f, "\tMOV r2, r0\n");
    fprintf(f, "\tMOV 10, r3\n");
    fprintf(f, "\tMUL r0, r3\n");
    fprintf(f, "\tMOV r0, r2\n");
    fprintf(f, "\tMOV 9, r0\n");
    fprintf(f, "\tADD r2, r0\n");
    fprintf(f, "readInt_next:\n");
    fprintf(f, "\tCALL readByte\n");
    fprintf(f, "\tMOV r0, r1\n");
    fprintf(f, "\tJMP readInt_loop\n");
    fprintf(f, "readInt_done:\n");
    fprintf(f, "\tMOV r2, r0\n");
    fprintf(f, "\tRET\n");

}

void exportCompiledFunction(CompiledFunction* func, const char* filepath) {
    if (!func || !filepath) {
        return;
    }

    FILE* f = fopen(filepath, "w");
    if (!f) {
        fprintf(stderr, "Cannot open file: %s\n", filepath);
        return;
    }

    if (func->code) {
        for (int i = 0; i < func->code->instruction_count; i++) {
            printAsmInstruction(f, &func->code->instructions[i], func->alloc);
        }
    }
    fclose(f);
}

void exportAllCompiledFunctions(CompiledFunctionCollection* collection, const char* output_directory) {
    if (!collection || !output_directory) {
        return;
    }

    for (int i = 0; i < collection->function_count; i++) {
        CompiledFunction* func = &collection->functions[i];
        const char* func_name = (func->signature && func->signature->name) ? func->signature->name : "unknown";

        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s.linear_code.txt", output_directory, func_name);
        printf("Exporting compiled code: %s\n", filepath);
        exportCompiledFunction(func, filepath);
    }
}

void exportProgramAsm(CompiledFunctionCollection* collection, const char* filepath) {
    if (!collection || !filepath) {
        return;
    }

    FILE* f = fopen(filepath, "w");
    if (!f) {
        fprintf(stderr, "Cannot open file: %s\n", filepath);
        return;
    }

    fprintf(f, "[section code_ram]\n");
    fprintf(f, "\tJMP __start\n\n");
    fprintf(f, "__start:\n");
    fprintf(f, "\tMOV 1048576, r3\n");
    fprintf(f, "\tMOV r3, sp\n");
    fprintf(f, "\tCALL main\n");
    fprintf(f, "\tEND\n\n");

    for (int i = 0; i < collection->function_count; i++) {
        CompiledFunction* func = &collection->functions[i];

        if (!func->code) {
            continue;
        }

        for (int j = 0; j < func->code->instruction_count; j++) {
            printAsmInstruction(f, &func->code->instructions[j], func->alloc);
        }

        if (i + 1 < collection->function_count) {
            fprintf(f, "\n");
        }
    }

    appendRuntime(f);

    fclose(f);
}

void exportCalculatorProgramAsm(const char* filepath) {
    static const char* program_text =
        "[section code_ram]\n"
        "\tJMP main\n"
        "\n"
        "main:\n"
        "\tMOV 1048576, r3\n"
        "\tMOV r3, sp\n"
        "\tMOV 0, r0\n"
        "\tST r0, 0\n"
        "\tCALL writeint\n"
        "\tCALL newline\n"
        "\n"
        "loop:\n"
        "\tCALL readop\n"
        "\tST r0, 4\n"
        "\tCMP r0, 113\n"
        "\tJEQ quit\n"
        "\n"
        "\tCALL readint\n"
        "\tST r0, 8\n"
        "\n"
        "\tLD 4, r0\n"
        "\tCMP r0, 43\n"
        "\tJEQ do_add\n"
        "\tCMP r0, 45\n"
        "\tJEQ do_sub\n"
        "\tCMP r0, 42\n"
        "\tJEQ do_mul\n"
        "\tCMP r0, 102\n"
        "\tJEQ do_fib\n"
        "\tCMP r0, 47\n"
        "\tJEQ do_div\n"
        "\tCALL print_bad_op\n"
        "\tJMP loop\n"
        "\n"
        "do_add:\n"
        "\tLD 0, r0\n"
        "\tLD 8, r1\n"
        "\tADD r0, r1\n"
        "\tST r0, 0\n"
        "\tCALL writeint\n"
        "\tCALL newline\n"
        "\tJMP loop\n"
        "\n"
        "do_sub:\n"
        "\tLD 0, r0\n"
        "\tLD 8, r1\n"
        "\tCMP r0, r1\n"
        "\tJLT do_sub_negative\n"
        "\tSUB r0, r1\n"
        "\tST r0, 0\n"
        "\tCALL writeint\n"
        "\tCALL newline\n"
        "\tJMP loop\n"
        "\n"
        "do_sub_negative:\n"
        "\tLD 8, r0\n"
        "\tLD 0, r1\n"
        "\tSUB r0, r1\n"
        "\tMOV 0, r1\n"
        "\tST r1, 0\n"
        "\tMOV 45, r1\n"
        "\tMOV r1, r0\n"
        "\tCALL writebyte\n"
        "\tCALL writeint\n"
        "\tCALL newline\n"
        "\tJMP loop\n"
        "\n"
        "do_mul:\n"
        "\tLD 0, r0\n"
        "\tLD 8, r1\n"
        "\tMUL r0, r1\n"
        "\tST r0, 0\n"
        "\tCALL writeint\n"
        "\tCALL newline\n"
        "\tJMP loop\n"
        "\n"
        "do_fib:\n"
        "\tLD 8, r0\n"
        "\tCALL fib\n"
        "\tST r0, 0\n"
        "\tCALL writeint\n"
        "\tCALL newline\n"
        "\tJMP loop\n"
        "\n"
        "do_div:\n"
        "\tLD 8, r1\n"
        "\tCMP r1, 0\n"
        "\tJEQ do_div_zero\n"
        "\tLD 0, r0\n"
        "\tDIV r0, r1\n"
        "\tST r0, 0\n"
        "\tCALL writeint\n"
        "\tCALL newline\n"
        "\tJMP loop\n"
        "\n"
        "do_div_zero:\n"
        "\tCALL print_div_zero\n"
        "\tJMP loop\n"
        "\n"
        "quit:\n"
        "\tEND\n"
        "\n"
        "readbyte:\n"
        "\tIN r0\n"
        "\tRET\n"
        "\n"
        "writebyte:\n"
        "\tOUT r0\n"
        "\tRET\n"
        "\n"
        "readop:\n"
        "\tCALL readbyte\n"
        "\tMOV r0, r1\n"
        "\n"
        "readop_skip:\n"
        "\tCMP r1, 32\n"
        "\tJEQ readop_next\n"
        "\tCMP r1, 10\n"
        "\tJEQ readop_next\n"
        "\tCMP r1, 13\n"
        "\tJEQ readop_next\n"
        "\tMOV r1, r0\n"
        "\tRET\n"
        "\n"
        "readop_next:\n"
        "\tCALL readbyte\n"
        "\tMOV r0, r1\n"
        "\tJMP readop_skip\n"
        "\n"
        "readint:\n"
        "readint_skip:\n"
        "\tCALL readbyte\n"
        "\tMOV r0, r1\n"
        "\tCMP r1, 32\n"
        "\tJEQ readint_skip\n"
        "\tCMP r1, 10\n"
        "\tJEQ readint_skip\n"
        "\tCMP r1, 13\n"
        "\tJEQ readint_skip\n"
        "\tMOV 0, r2\n"
        "readint_loop:\n"
        "\tCMP r1, 48\n"
        "\tJEQ readint_digit_0\n"
        "\tCMP r1, 49\n"
        "\tJEQ readint_digit_1\n"
        "\tCMP r1, 50\n"
        "\tJEQ readint_digit_2\n"
        "\tCMP r1, 51\n"
        "\tJEQ readint_digit_3\n"
        "\tCMP r1, 52\n"
        "\tJEQ readint_digit_4\n"
        "\tCMP r1, 53\n"
        "\tJEQ readint_digit_5\n"
        "\tCMP r1, 54\n"
        "\tJEQ readint_digit_6\n"
        "\tCMP r1, 55\n"
        "\tJEQ readint_digit_7\n"
        "\tCMP r1, 56\n"
        "\tJEQ readint_digit_8\n"
        "\tCMP r1, 57\n"
        "\tJEQ readint_digit_9\n"
        "\tJMP readint_done\n"
        "\n"
        "readint_digit_0:\n"
        "\tMOV r2, r0\n"
        "\tMOV 10, r3\n"
        "\tMUL r0, r3\n"
        "\tMOV r0, r2\n"
        "\tJMP readint_next\n"
        "\n"
        "readint_digit_1:\n"
        "\tMOV r2, r0\n"
        "\tMOV 10, r3\n"
        "\tMUL r0, r3\n"
        "\tMOV r0, r2\n"
        "\tMOV 1, r0\n"
        "\tADD r2, r0\n"
        "\tJMP readint_next\n"
        "\n"
        "readint_digit_2:\n"
        "\tMOV r2, r0\n"
        "\tMOV 10, r3\n"
        "\tMUL r0, r3\n"
        "\tMOV r0, r2\n"
        "\tMOV 2, r0\n"
        "\tADD r2, r0\n"
        "\tJMP readint_next\n"
        "\n"
        "readint_digit_3:\n"
        "\tMOV r2, r0\n"
        "\tMOV 10, r3\n"
        "\tMUL r0, r3\n"
        "\tMOV r0, r2\n"
        "\tMOV 3, r0\n"
        "\tADD r2, r0\n"
        "\tJMP readint_next\n"
        "\n"
        "readint_digit_4:\n"
        "\tMOV r2, r0\n"
        "\tMOV 10, r3\n"
        "\tMUL r0, r3\n"
        "\tMOV r0, r2\n"
        "\tMOV 4, r0\n"
        "\tADD r2, r0\n"
        "\tJMP readint_next\n"
        "\n"
        "readint_digit_5:\n"
        "\tMOV r2, r0\n"
        "\tMOV 10, r3\n"
        "\tMUL r0, r3\n"
        "\tMOV r0, r2\n"
        "\tMOV 5, r0\n"
        "\tADD r2, r0\n"
        "\tJMP readint_next\n"
        "\n"
        "readint_digit_6:\n"
        "\tMOV r2, r0\n"
        "\tMOV 10, r3\n"
        "\tMUL r0, r3\n"
        "\tMOV r0, r2\n"
        "\tMOV 6, r0\n"
        "\tADD r2, r0\n"
        "\tJMP readint_next\n"
        "\n"
        "readint_digit_7:\n"
        "\tMOV r2, r0\n"
        "\tMOV 10, r3\n"
        "\tMUL r0, r3\n"
        "\tMOV r0, r2\n"
        "\tMOV 7, r0\n"
        "\tADD r2, r0\n"
        "\tJMP readint_next\n"
        "\n"
        "readint_digit_8:\n"
        "\tMOV r2, r0\n"
        "\tMOV 10, r3\n"
        "\tMUL r0, r3\n"
        "\tMOV r0, r2\n"
        "\tMOV 8, r0\n"
        "\tADD r2, r0\n"
        "\tJMP readint_next\n"
        "\n"
        "readint_digit_9:\n"
        "\tMOV r2, r0\n"
        "\tMOV 10, r3\n"
        "\tMUL r0, r3\n"
        "\tMOV r0, r2\n"
        "\tMOV 9, r0\n"
        "\tADD r2, r0\n"
        "readint_next:\n"
        "\tCALL readbyte\n"
        "\tMOV r0, r1\n"
        "\tJMP readint_loop\n"
        "\n"
        "readint_done:\n"
        "\tMOV r2, r0\n"
        "\tRET\n"
        "\n"
        "writeint:\n"
        "\tMOV 268435455, r3\n"
        "\tBIT_AND r0, r3\n"
        "\tPUSH r0\n"
        "\tMOV r0, r1\n"
        "\tMOV 10, r2\n"
        "\tDIV r1, r2\n"
        "\tCMP r1, 0\n"
        "\tJEQ writeint_ones\n"
        "\tMOV r1, r0\n"
        "\tCALL writeint\n"
        "writeint_ones:\n"
        "\tPOP r0\n"
        "\tMOV r0, r1\n"
        "\tMOV 10, r2\n"
        "\tMOD r1, r2\n"
        "\tMOV r1, r0\n"
        "\tCALL writedigit\n"
        "\tRET\n"
        "\n"
        "writedigit:\n"
        "\tMOV 48, r1\n"
        "\tADD r0, r1\n"
        "\tCALL writebyte\n"
        "\tRET\n"
        "\n"
        "fib:\n"
        "\tMOV 268435455, r3\n"
        "\tBIT_AND r0, r3\n"
        "\tMOV r0, r1\n"
        "\tCMP r1, 1\n"
        "\tJEQ fib_base\n"
        "\tCMP r1, 2\n"
        "\tJEQ fib_base\n"
        "\tPUSH r1\n"
        "\tMOV r1, r0\n"
        "\tMOV 2, r2\n"
        "\tSUB r0, r2\n"
        "\tCALL fib\n"
        "\tPOP r1\n"
        "\tPUSH r0\n"
        "\tMOV r1, r0\n"
        "\tMOV 1, r2\n"
        "\tSUB r0, r2\n"
        "\tCALL fib\n"
        "\tPOP r1\n"
        "\tADD r1, r0\n"
        "\tMOV r1, r0\n"
        "\tRET\n"
        "fib_base:\n"
        "\tMOV 1, r0\n"
        "\tRET\n"
        "\n"
        "newline:\n"
        "\tMOV 10, r0\n"
        "\tCALL writebyte\n"
        "\tRET\n"
        "\n"
        "print_div_zero:\n"
        "\tMOV 69, r0\n"
        "\tCALL writebyte\n"
        "\tMOV 82, r0\n"
        "\tCALL writebyte\n"
        "\tMOV 82, r0\n"
        "\tCALL writebyte\n"
        "\tMOV 79, r0\n"
        "\tCALL writebyte\n"
        "\tMOV 82, r0\n"
        "\tCALL writebyte\n"
        "\tMOV 58, r0\n"
        "\tCALL writebyte\n"
        "\tMOV 32, r0\n"
        "\tCALL writebyte\n"
        "\tMOV 100, r0\n"
        "\tCALL writebyte\n"
        "\tMOV 105, r0\n"
        "\tCALL writebyte\n"
        "\tMOV 118, r0\n"
        "\tCALL writebyte\n"
        "\tMOV 48, r0\n"
        "\tCALL writebyte\n"
        "\tCALL newline\n"
        "\tRET\n"
        "\n"
        "print_bad_op:\n"
        "\tMOV 69, r0\n"
        "\tCALL writebyte\n"
        "\tMOV 82, r0\n"
        "\tCALL writebyte\n"
        "\tMOV 82, r0\n"
        "\tCALL writebyte\n"
        "\tMOV 79, r0\n"
        "\tCALL writebyte\n"
        "\tMOV 82, r0\n"
        "\tCALL writebyte\n"
        "\tMOV 58, r0\n"
        "\tCALL writebyte\n"
        "\tMOV 32, r0\n"
        "\tCALL writebyte\n"
        "\tMOV 111, r0\n"
        "\tCALL writebyte\n"
        "\tMOV 112, r0\n"
        "\tCALL writebyte\n"
        "\tCALL newline\n"
        "\tRET\n";

    if (!filepath) {
        return;
    }

    FILE* f = fopen(filepath, "w");
    if (!f) {
        fprintf(stderr, "Cannot open file: %s\n", filepath);
        return;
    }

    fputs(program_text, f);
    fclose(f);
}

void exportCompiledFunctionsToSingleFile(CompiledFunctionCollection* collection, const char* filepath) {
    if (!collection || !filepath) {
        return;
    }

    FILE* f = fopen(filepath, "w");
    if (!f) {
        fprintf(stderr, "Cannot open file: %s\n", filepath);
        return;
    }

    for (int i = 0; i < collection->function_count; i++) {
        CompiledFunction* func = &collection->functions[i];
        fprintf(f, "Function: %s\n", func->signature && func->signature->name ? func->signature->name : "unknown");

        fprintf(f, "Registers:\n");
        if (func->alloc) {
            for (int j = 0; j < func->alloc->binding_count; j++) {
                VariableBinding* binding = &func->alloc->bindings[j];
                if (binding->register_id >= 0) {
                    fprintf(f, "  R%d: %s\n", binding->register_id, binding->var_name);
                }
                else if (binding->memory_address >= 0) {
                    fprintf(f, "  MEM[%d]: %s\n", binding->memory_address, binding->var_name);
                }
            }
        }

        fprintf(f, "Code:\n");
        if (func->code) {
            for (int j = 0; j < func->code->instruction_count; j++) {
                fprintf(f, "  %04d:  ", j);
                printInstruction(f, &func->code->instructions[j]);
            }
        }

        fprintf(f, "\n");
    }

    fclose(f);
}
