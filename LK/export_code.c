#include "export_code.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    fprintf(f, "==================================================\n");
    fprintf(f, "      COMPILED CODE FOR: %s\n", func->signature && func->signature->name ? func->signature->name : "unknown");
    fprintf(f, "==================================================\n\n");

    if (func->signature) {
        fprintf(f, "; Function signature: %s", func->signature->name ? func->signature->name : "unknown");
        if (func->signature->return_type) {
            fprintf(f, " -> %s", func->signature->return_type);
        }
        fprintf(f, "\n");

        if (func->signature->args) {
            fprintf(f, "; Parameters: ");
            FunctionArg* arg = func->signature->args;
            int first = 1;
            while (arg) {
                if (!first) {
                    fprintf(f, ", ");
                }
                fprintf(f, "%s: %s", arg->name ? arg->name : "?", arg->type ? arg->type : "?");
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
            } else if (binding->memory_address >= 0) {
                fprintf(f, "MEM[%d]: %s", binding->memory_address, binding->var_name);
            }
            fprintf(f, "\n");
        }
    }
    fprintf(f, "\n");

    fprintf(f, "ASSEMBLY LISTING\n");
    if (func->code) {
        for (int i = 0; i < func->code->instruction_count; i++) {
            printInstruction(f, &func->code->instructions[i]);
        }
    }

    fprintf(f, "\nEND OF CODE\n");
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
                } else if (binding->memory_address >= 0) {
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
