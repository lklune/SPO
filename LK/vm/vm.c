//
// vm.c - Реализация виртуальной машины
// Задание 3: Динамическая типизация, двухадресный формат, 4 банка памяти
//

#include "vm.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

static VMValue vmCopyValue(VMValue src) {
    VMValue out = src;
    if (src.type == VM_TYPE_STRING && src.value.string_val) {
        out.value.string_val = strdup(src.value.string_val);
    }
    return out;
}

VMMemory* vmMemoryCreate(int constants_size, int data_size, int stack_size) {
    VMMemory* mem = (VMMemory*)malloc(sizeof(VMMemory));
    if (!mem) return NULL;

    // Банк кода
    mem->code_bank.addresses = (int*)malloc(sizeof(int) * 1024);
    mem->code_bank.count = 0;

    // Банк констант
    mem->constants_bank.values = (VMValue*)malloc(sizeof(VMValue) * constants_size);
    mem->constants_bank.count = 0;
    mem->constants_bank.max_count = constants_size;

    // Банк данных
    mem->data_bank.values = (VMValue*)malloc(sizeof(VMValue) * data_size);
    mem->data_bank.var_names = (char**)malloc(sizeof(char*) * data_size);
    mem->data_bank.count = 0;
    mem->data_bank.max_count = data_size;

    // Стек
    mem->stack.values = (VMValue*)malloc(sizeof(VMValue) * stack_size);
    mem->stack.pointer = 0;
    mem->stack.size = stack_size;

    return mem;
}

void vmMemoryFree(VMMemory* mem) {
    if (!mem) return;

    free(mem->code_bank.addresses);
    for (int i = 0; i < mem->constants_bank.count; i++) {
        if (mem->constants_bank.values[i].type == VM_TYPE_STRING &&
            mem->constants_bank.values[i].value.string_val) {
            free(mem->constants_bank.values[i].value.string_val);
        }
    }
    free(mem->constants_bank.values);

    for (int i = 0; i < mem->data_bank.count; i++) {
        if (mem->data_bank.var_names[i]) free(mem->data_bank.var_names[i]);
        if (mem->data_bank.values[i].type == VM_TYPE_STRING && mem->data_bank.values[i].value.string_val) {
            free(mem->data_bank.values[i].value.string_val);
        }
    }
    free(mem->data_bank.values);
    free(mem->data_bank.var_names);

    free(mem->stack.values);
    free(mem);
}

// Константы

int vmAddConstant(VMMemory* mem, VMValue value) {
    if (!mem || mem->constants_bank.count >= mem->constants_bank.max_count) {
        return -1;
    }

    int idx = mem->constants_bank.count++;
    mem->constants_bank.values[idx] = vmCopyValue(value);
    return idx;
}

VMValue* vmGetConstant(VMMemory* mem, int index) {
    if (!mem || index < 0 || index >= mem->constants_bank.count) {
        return NULL;
    }
    return &mem->constants_bank.values[index];
}

// банк данных

int vmAddVariable(VMMemory* mem, const char* var_name, VMValue value) {
    if (!mem || !var_name || mem->data_bank.count >= mem->data_bank.max_count) {
        return -1;
    }

    int idx = mem->data_bank.count++;
    mem->data_bank.var_names[idx] = strdup(var_name);
    mem->data_bank.values[idx] = vmCopyValue(value);
    return idx;
}

int vmFindVariable(VMMemory* mem, const char* var_name) {
    if (!mem || !var_name) return -1;

    for (int i = 0; i < mem->data_bank.count; i++) {
        if (strcmp(mem->data_bank.var_names[i], var_name) == 0) {
            return i;
        }
    }
    return -1;
}

VMValue* vmGetVariable(VMMemory* mem, const char* var_name) {
    if (!mem || !var_name) return NULL;

    int idx = vmFindVariable(mem, var_name);
    if (idx < 0) return NULL;
    return &mem->data_bank.values[idx];
}

void vmSetVariable(VMMemory* mem, const char* var_name, VMValue value) {
    if (!mem || !var_name) return;

    int idx = vmFindVariable(mem, var_name);
    if (idx < 0) {
        vmAddVariable(mem, var_name, value);
    }
    else {
        if (mem->data_bank.values[idx].type == VM_TYPE_STRING &&
            mem->data_bank.values[idx].value.string_val) {
            free(mem->data_bank.values[idx].value.string_val);
        }
        mem->data_bank.values[idx] = vmCopyValue(value);
    }
}

// Стек

void vmPush(VMMemory* mem, VMValue value) {
    if (!mem || mem->stack.pointer >= mem->stack.size) {
        fprintf(stderr, "Stack overflow!\n");
        return;
    }
    mem->stack.values[mem->stack.pointer++] = value;
}

VMValue vmPop(VMMemory* mem) {
    if (!mem || mem->stack.pointer <= 0) {
        fprintf(stderr, "Stack underflow!\n");
        return vmMakeNone();
    }
    return mem->stack.values[--mem->stack.pointer];
}

VMValue* vmPeek(VMMemory* mem) {
    if (!mem || mem->stack.pointer <= 0) {
        return NULL;
    }
    return &mem->stack.values[mem->stack.pointer - 1];
}

VMValue vmMakeInt(long int_val) {
    VMValue val;
    val.type = VM_TYPE_INT;
    val.value.int_val = int_val;
    return val;
}

VMValue vmMakeFloat(double float_val) {
    VMValue val;
    val.type = VM_TYPE_FLOAT;
    val.value.float_val = float_val;
    return val;
}

VMValue vmMakeBool(int bool_val) {
    VMValue val;
    val.type = VM_TYPE_BOOL;
    val.value.bool_val = bool_val ? 1 : 0;
    return val;
}

VMValue vmMakeString(const char* string_val) {
    VMValue val;
    val.type = VM_TYPE_STRING;
    val.value.string_val = string_val ? strdup(string_val) : NULL;
    return val;
}

VMValue vmMakeNone(void) {
    VMValue val;
    val.type = VM_TYPE_NONE;
    return val;
}

// Конвертация типов

long vmToInt(VMValue val) {
    switch (val.type) {
    case VM_TYPE_INT:
        return val.value.int_val;
    case VM_TYPE_FLOAT:
        return (long)val.value.float_val;
    case VM_TYPE_BOOL:
        return val.value.bool_val;
    case VM_TYPE_STRING:
        return atol(val.value.string_val ? val.value.string_val : "0");
    default:
        return 0;
    }
}

double vmToFloat(VMValue val) {
    switch (val.type) {
    case VM_TYPE_INT:
        return (double)val.value.int_val;
    case VM_TYPE_FLOAT:
        return val.value.float_val;
    case VM_TYPE_BOOL:
        return (double)val.value.bool_val;
    case VM_TYPE_STRING:
        return atof(val.value.string_val ? val.value.string_val : "0");
    default:
        return 0.0;
    }
}

int vmToBool(VMValue val) {
    switch (val.type) {
    case VM_TYPE_INT:
        return val.value.int_val != 0;
    case VM_TYPE_FLOAT:
        return val.value.float_val != 0.0;
    case VM_TYPE_BOOL:
        return val.value.bool_val;
    case VM_TYPE_STRING:
        return val.value.string_val && strlen(val.value.string_val) > 0;
    default:
        return 0;
    }
}

const char* vmToString(VMValue val) {
    static char buffer[256];

    switch (val.type) {
    case VM_TYPE_INT:
        snprintf(buffer, sizeof(buffer), "%ld", val.value.int_val);
        return buffer;
    case VM_TYPE_FLOAT:
        snprintf(buffer, sizeof(buffer), "%g", val.value.float_val);
        return buffer;
    case VM_TYPE_BOOL:
        return val.value.bool_val ? "true" : "false";
    case VM_TYPE_STRING:
        return val.value.string_val ? val.value.string_val : "";
    default:
        return "";
    }
}

int vmEqual(VMValue a, VMValue b) {
    // Если типы одинаковые - прямое сравнение
    if (a.type == b.type) {
        switch (a.type) {
        case VM_TYPE_INT:
            return a.value.int_val == b.value.int_val;
        case VM_TYPE_FLOAT:
            return fabs(a.value.float_val - b.value.float_val) < 1e-9;
        case VM_TYPE_BOOL:
            return a.value.bool_val == b.value.bool_val;
        case VM_TYPE_STRING:
            return strcmp(a.value.string_val ? a.value.string_val : "",
                b.value.string_val ? b.value.string_val : "") == 0;
        default:
            return 1;
        }
    }

    // Разные типы - конвертируем и сравниваем
    // Сначала пробуем числовое сравнение
    if ((a.type == VM_TYPE_INT || a.type == VM_TYPE_FLOAT) &&
        (b.type == VM_TYPE_INT || b.type == VM_TYPE_FLOAT)) {
        return fabs(vmToFloat(a) - vmToFloat(b)) < 1e-9;
    }

    return 0;
}

int vmLess(VMValue a, VMValue b) {
    // Конвертируем в числа для сравнения
    if ((a.type == VM_TYPE_INT || a.type == VM_TYPE_FLOAT) &&
        (b.type == VM_TYPE_INT || b.type == VM_TYPE_FLOAT)) {
        return vmToFloat(a) < vmToFloat(b);
    }

    // Для строк - лексикографическое сравнение
    if (a.type == VM_TYPE_STRING && b.type == VM_TYPE_STRING) {
        return strcmp(a.value.string_val ? a.value.string_val : "",
            b.value.string_val ? b.value.string_val : "") < 0;
    }

    return 0;
}

int vmGreater(VMValue a, VMValue b) {
    // Конвертируем в числа для сравнения
    if ((a.type == VM_TYPE_INT || a.type == VM_TYPE_FLOAT) &&
        (b.type == VM_TYPE_INT || b.type == VM_TYPE_FLOAT)) {
        return vmToFloat(a) > vmToFloat(b);
    }

    // Для строк - лексикографическое сравнение
    if (a.type == VM_TYPE_STRING && b.type == VM_TYPE_STRING) {
        return strcmp(a.value.string_val ? a.value.string_val : "",
            b.value.string_val ? b.value.string_val : "") > 0;
    }

    return 0;
}

VMContext* vmContextCreate(LinearCode* code, VMMemory* memory) {
    VMContext* ctx = (VMContext*)malloc(sizeof(VMContext));
    if (!ctx) return NULL;

    ctx->code = code;
    ctx->memory = memory;
    ctx->pc = 0;
    ctx->compare_result = 0;
    ctx->running = 1;

    // Инициализируем таблицу меток
    ctx->label_addresses = (int*)malloc(sizeof(int) * 1024);
    ctx->label_names = (char**)malloc(sizeof(char*) * 1024);
    ctx->label_count = 0;

    // Первый проход: собираем все метки и их адреса
    for (int i = 0; i < code->instruction_count; i++) {
        if (code->instructions[i].type == INSTR_LABEL) {
            if (code->instructions[i].operand1.type == OPERAND_LABEL &&
                code->instructions[i].operand1.value.name &&
                ctx->label_count < 1024) {
                ctx->label_names[ctx->label_count] = code->instructions[i].operand1.value.name;
                ctx->label_addresses[ctx->label_count] = i;
                ctx->label_count++;
            }
        }
    }

    return ctx;
}

void vmContextFree(VMContext* ctx) {
    if (!ctx) return;
    free(ctx->label_addresses);
    free(ctx->label_names);
    free(ctx);
}

// Найти адрес метки по имени; возвращает -1 если не найдена
static int findLabelAddress(VMContext* ctx, const char* name) {
    if (!ctx || !name) return -1;
    for (int i = 0; i < ctx->label_count; i++) {
        if (ctx->label_names[i] && strcmp(ctx->label_names[i], name) == 0)
            return ctx->label_addresses[i];
    }
    return -1;
}

static VMValue getOperandValue(VMContext* ctx, Operand* op) {
    if (!op) return vmMakeNone();

    switch (op->type) {
    case OPERAND_REGISTER:
    {
        char reg_name[32];
        snprintf(reg_name, sizeof(reg_name), "R%d", op->value.register_id);
        VMValue* val = vmGetVariable(ctx->memory, reg_name);
        return val ? *val : vmMakeInt(0);
    }

    case OPERAND_VARIABLE:
    {
        VMValue* val = vmGetVariable(ctx->memory, op->value.name);
        return val ? *val : vmMakeInt(0);
    }

    case OPERAND_CONSTANT:
        return vmMakeInt(op->value.value);

    case OPERAND_STRING:
    {
        VMValue v = vmMakeNone();
        v.type = VM_TYPE_STRING;
        v.value.string_val = op->value.name;
        return v;
    }

    default:
        return vmMakeNone();
    }
}

static void setOperandValue(VMContext* ctx, Operand* op, VMValue val) {
    if (!op) return;

    switch (op->type) {
    case OPERAND_REGISTER:
    {
        char reg_name[32];
        snprintf(reg_name, sizeof(reg_name), "R%d", op->value.register_id);
        vmSetVariable(ctx->memory, reg_name, val);
    }
    break;

    case OPERAND_VARIABLE:
        vmSetVariable(ctx->memory, op->value.name, val);
        break;

    default:
        break;
    }
}

int vmExecuteInstruction(VMContext* ctx) {
    if (!ctx || !ctx->code || ctx->pc >= ctx->code->instruction_count) {
        ctx->running = 0;
        return 0;
    }

    Instruction* instr = &ctx->code->instructions[ctx->pc];
    VMValue result = vmMakeNone();
    VMValue op1_val = getOperandValue(ctx, &instr->operand1);
    VMValue op2_val = getOperandValue(ctx, &instr->operand2);

    switch (instr->type) {
    case INSTR_MOV:
        // MOV dst, src
        setOperandValue(ctx, &instr->operand1, op2_val);
        break;

    case INSTR_LOAD_CONST:
        // LOAD_CONST reg, const
        setOperandValue(ctx, &instr->operand1, op2_val);
        break;

    case INSTR_ADD:
        // ADD reg1, reg2
        result = vmMakeInt(vmToInt(op1_val) + vmToInt(op2_val));
        setOperandValue(ctx, &instr->operand1, result);
        break;

    case INSTR_SUB:
        // SUB reg1, reg2
        result = vmMakeInt(vmToInt(op1_val) - vmToInt(op2_val));
        setOperandValue(ctx, &instr->operand1, result);
        break;

    case INSTR_MUL:
        // MUL reg1, reg2
        result = vmMakeInt(vmToInt(op1_val) * vmToInt(op2_val));
        setOperandValue(ctx, &instr->operand1, result);
        break;

    case INSTR_DIV:
        // DIV reg1, reg2
    {
        long divisor = vmToInt(op2_val);
        if (divisor != 0) {
            result = vmMakeInt(vmToInt(op1_val) / divisor);
        }
        else {
            fprintf(stderr, "Division by zero!\n");
            result = vmMakeInt(0);
        }
        setOperandValue(ctx, &instr->operand1, result);
    }
    break;

    case INSTR_MOD:
        // MOD reg1, reg2
    {
        long divisor = vmToInt(op2_val);
        if (divisor != 0) {
            result = vmMakeInt(vmToInt(op1_val) % divisor);
        }
        else {
            result = vmMakeInt(0);
        }
        setOperandValue(ctx, &instr->operand1, result);
    }
    break;

    case INSTR_AND:
        result = vmMakeBool(vmToBool(op1_val) && vmToBool(op2_val));
        setOperandValue(ctx, &instr->operand1, result);
        break;

    case INSTR_OR:
        result = vmMakeBool(vmToBool(op1_val) || vmToBool(op2_val));
        setOperandValue(ctx, &instr->operand1, result);
        break;

    case INSTR_XOR:
        result = vmMakeBool(vmToBool(op1_val) ^ vmToBool(op2_val));
        setOperandValue(ctx, &instr->operand1, result);
        break;

    case INSTR_NOT:
        result = vmMakeBool(!vmToBool(op1_val));
        setOperandValue(ctx, &instr->operand1, result);
        break;

    case INSTR_BIT_AND:
        result = vmMakeInt(vmToInt(op1_val) & vmToInt(op2_val));
        setOperandValue(ctx, &instr->operand1, result);
        break;

    case INSTR_BIT_OR:
        result = vmMakeInt(vmToInt(op1_val) | vmToInt(op2_val));
        setOperandValue(ctx, &instr->operand1, result);
        break;

    case INSTR_BIT_XOR:
        result = vmMakeInt(vmToInt(op1_val) ^ vmToInt(op2_val));
        setOperandValue(ctx, &instr->operand1, result);
        break;

    case INSTR_BIT_NOT:
        result = vmMakeInt(~vmToInt(op1_val));
        setOperandValue(ctx, &instr->operand1, result);
        break;

    case INSTR_SHIFT_LEFT:
        result = vmMakeInt(vmToInt(op1_val) << vmToInt(op2_val));
        setOperandValue(ctx, &instr->operand1, result);
        break;

    case INSTR_SHIFT_RIGHT:
        result = vmMakeInt(vmToInt(op1_val) >> vmToInt(op2_val));
        setOperandValue(ctx, &instr->operand1, result);
        break;

    case INSTR_CALL:
        // CALL function_name — ищем метку входа функции и прыгаем
        // Адрес возврата (следующая инструкция) кладём на стек как INT
    {
        vmPush(ctx->memory, vmMakeInt(ctx->pc + 1));
        int addr = findLabelAddress(ctx, instr->operand1.value.name);
        if (addr >= 0) {
            ctx->pc = addr;
            return ctx->running;
        }
        // Метка не найдена — просто продолжаем (внешняя функция)
        vmPop(ctx->memory); // убираем адрес возврата
        fprintf(stderr, "CALL: function label not found: %s\n",
            instr->operand1.value.name ? instr->operand1.value.name : "?");
    }
    break;

    case INSTR_CMP:
        // CMP reg1, reg2 — сохраняем разницу для условных переходов
        ctx->compare_result = vmToInt(op1_val) - vmToInt(op2_val);
        break;

    case INSTR_JEQ:
        if (ctx->compare_result == 0) {
            int addr = findLabelAddress(ctx, instr->operand1.value.name);
            if (addr >= 0) { ctx->pc = addr; return ctx->running; }
        }
        break;

    case INSTR_JNE:
        if (ctx->compare_result != 0) {
            int addr = findLabelAddress(ctx, instr->operand1.value.name);
            if (addr >= 0) { ctx->pc = addr; return ctx->running; }
        }
        break;

    case INSTR_JLT:
        if (ctx->compare_result < 0) {
            int addr = findLabelAddress(ctx, instr->operand1.value.name);
            if (addr >= 0) { ctx->pc = addr; return ctx->running; }
        }
        break;

    case INSTR_JGT:
        if (ctx->compare_result > 0) {
            int addr = findLabelAddress(ctx, instr->operand1.value.name);
            if (addr >= 0) { ctx->pc = addr; return ctx->running; }
        }
        break;

    case INSTR_JLE:
        if (ctx->compare_result <= 0) {
            int addr = findLabelAddress(ctx, instr->operand1.value.name);
            if (addr >= 0) { ctx->pc = addr; return ctx->running; }
        }
        break;

    case INSTR_JGE:
        if (ctx->compare_result >= 0) {
            int addr = findLabelAddress(ctx, instr->operand1.value.name);
            if (addr >= 0) { ctx->pc = addr; return ctx->running; }
        }
        break;

    case INSTR_JMP:
    {
        int addr = findLabelAddress(ctx, instr->operand1.value.name);
        if (addr >= 0) { ctx->pc = addr; return ctx->running; }
        fprintf(stderr, "JMP: label not found: %s\n",
            instr->operand1.value.name ? instr->operand1.value.name : "?");
    }
    break;

    case INSTR_PRINT:
        // PRINT reg - вывести значение
        printf("%s\n", vmToString(op1_val));
        break;

    case INSTR_PRINT_STR:
        // PRINT_STR string - вывести строку
        printf("%s\n", vmToString(op1_val));
        break;

    case INSTR_RET:
        // RET value - return to caller if CALL frame exists
    {
        VMValue ret_val = op1_val;
        if (ctx->memory->stack.pointer > 0) {
            VMValue return_addr = vmPop(ctx->memory);
            if (return_addr.type == VM_TYPE_INT &&
                return_addr.value.int_val >= 0 &&
                return_addr.value.int_val < ctx->code->instruction_count) {
                vmSetVariable(ctx->memory, "R0", ret_val);
                ctx->pc = (int)return_addr.value.int_val;
                return ctx->running;
            }
        }

        vmPush(ctx->memory, ret_val);
        ctx->running = 0;
        return 0;
    }

    case INSTR_END:
        ctx->running = 0;
        return 0;

    case INSTR_LABEL:
        // Метка - ничего не делать
        break;

    default:
        break;
    }

    ctx->pc++;
    return ctx->running;
}

int vmRun(VMContext* ctx) {
    if (!ctx) return 0;

    while (ctx->running && ctx->pc < ctx->code->instruction_count) {
        vmExecuteInstruction(ctx);
    }

    return 1;
}

VMValue vmGetReturnValue(VMContext* ctx) {
    if (!ctx || !ctx->memory) return vmMakeNone();

    VMValue* result = vmPeek(ctx->memory);
    return result ? *result : vmMakeNone();
}
