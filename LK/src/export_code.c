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
