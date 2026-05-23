#include "export_code.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Поиск привязки переменной для получения адреса в памяти */
static VariableBinding* findBindingByName(RegisterAllocator* alloc, const char* name) {
    /* ищу переменную по имени */
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

/* Преобразование внутреннего операнда в текст для asm-экспорта
 * Для переменной подставляется не имя, а адрес
 */
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
        /* в asm тут уже нужен адрес */
        VariableBinding* binding = findBindingByName(alloc, operand->value.name);
        if (binding && binding->memory_address >= 0) {
            snprintf(buffer, (size_t)buffer_size, "%d", binding->memory_address);
        }
        else {
            fprintf(stderr, "Unknown variable binding in asm export: %s\n",
                operand->value.name ? operand->value.name : "?");
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

static void emitAsmWriteByte(FILE* f, int value) {
    if (!f) {
        return;
    }

    fprintf(f, "MOV %d, r0\n", value);
    fprintf(f, "CALL writeByte\n");
}

static void emitAsmString(FILE* f, const char* text) {
    size_t i = 0;

    if (!f || !text) {
        return;
    }

    while (text[i] != '\0') {
        unsigned char ch = (unsigned char)text[i];

        if (ch == '\\' && text[i + 1] != '\0') {
            switch (text[i + 1]) {
            case 'n':
                emitAsmWriteByte(f, 10);
                i += 2;
                continue;
            case 'r':
                emitAsmWriteByte(f, 13);
                i += 2;
                continue;
            case 't':
                emitAsmWriteByte(f, 9);
                i += 2;
                continue;
            case '\\':
                emitAsmWriteByte(f, '\\');
                i += 2;
                continue;
            case '"':
                emitAsmWriteByte(f, '"');
                i += 2;
                continue;
            case '0':
                emitAsmWriteByte(f, 0);
                i += 2;
                continue;
            default:
                emitAsmWriteByte(f, '\\');
                i += 1;
                continue;
            }
        }

        emitAsmWriteByte(f, ch);
        i += 1;
    }
}

/* Печать одной asm-инструкции
 * Внутренние MOV для переменных здесь превращаются в LD/ST
 */
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
        /* здесь MOV превращается в LD или ST */
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
        emitAsmString(f, instr->operand1.value.name ? instr->operand1.value.name : "");
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


/* Возврат текстового имени инструкции */
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

/* Вывод операнда в текстовом виде */
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


/* Добавление runtime в конец общей asm-программы
 * Это стандартные подпрограммы проекта
 */
static void appendRuntime(FILE* f) {
    if (!f) {
        return;
    }

    /* в конце добавляю runtime */
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

/* Экспорт одной функции в отдельный .linear_code.txt */
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

/* Экспорт всех функций по отдельным файлам */
void exportAllCompiledFunctions(CompiledFunctionCollection* collection, const char* output_directory) {
    if (!collection || !output_directory) {
        return;
    }

    /* отдельный файл на каждую функцию */
    for (int i = 0; i < collection->function_count; i++) {
        CompiledFunction* func = &collection->functions[i];
        const char* func_name =
            (func->generated_name && *func->generated_name) ? func->generated_name :
            ((func->signature && func->signature->name) ? func->signature->name : "unknown");

        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s.linear_code.txt", output_directory, func_name);
        printf("Exporting compiled code: %s\n", filepath);
        exportCompiledFunction(func, filepath);
    }
}

/* Общий экспорт всей программы в один program.asm
 * Сначала идёт описание user type, потом старт, функции и runtime
 */
void exportProgramAsm(AnalysisResult* result,
    CompiledFunctionCollection* collection, const char* filepath) {
    if (!collection || !filepath) {
        return;
    }

    FILE* f = fopen(filepath, "w");
    if (!f) {
        fprintf(stderr, "Cannot open file: %s\n", filepath);
        return;
    }

    fprintf(f, "[section code_ram]\n");
    fprintf(f, "; program structure metadata\n");
    if (result && result->types) {
        UserType* type_info = result->types->types;
        while (type_info) {
            TypeNameNode* interface_name = type_info->interfaces;
            UserTypeField* field = type_info->fields;
            /* кратко печатаю типы и поля */
            fprintf(f, "; .%s %s",
                type_info->is_interface ? "interface" : "type",
                type_info->name ? type_info->name : "?");
            if (type_info->base_type_name && *type_info->base_type_name) {
                fprintf(f, " of %s", type_info->base_type_name);
            }
            if (interface_name) {
                fprintf(f, " implements ");
                while (interface_name) {
                    fprintf(f, "%s%s",
                        interface_name != type_info->interfaces ? "," : "",
                        interface_name->name ? interface_name->name : "?");
                    interface_name = interface_name->next;
                }
            }
            fprintf(f, " size=%d\n", type_info->size_bytes);
            while (field) {
                fprintf(f, "; .field %s.%s : %s offset=%d owner=%s\n",
                    type_info->name ? type_info->name : "?",
                    field->name ? field->name : "?",
                    field->type_name ? field->type_name : "?",
                    field->offset,
                    field->owner_type_name ? field->owner_type_name : "?");
                field = field->next;
            }

            type_info = type_info->next;
        }
    }
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
