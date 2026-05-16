#include "codegen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_label_counter = 0;
static Function* g_current_function = NULL;

/* Создание уникальной метки для внутренних переходов */
static void make_auto_label(char* buf, size_t size, const char* prefix) {
    snprintf(buf, size, "%s_%d", prefix, g_label_counter++);
}

/* Набор helper-функций для создания операндов
 * Упрощает заполнение структуры операнда
 */
Operand createRegisterOperand(int reg_id) {
    Operand op;
    op.type = OPERAND_REGISTER;
    op.value.register_id = reg_id;
    return op;
}

Operand createVariableOperand(const char* var_name) {
    Operand op;
    op.type = OPERAND_VARIABLE;
    op.value.name = var_name ? strdup(var_name) : NULL;
    return op;
}

Operand createConstantOperand(long value) {
    Operand op;
    op.type = OPERAND_CONSTANT;
    op.value.value = value;
    return op;
}

Operand createLabelOperand(const char* label) {
    Operand op;
    op.type = OPERAND_LABEL;
    op.value.name = label ? strdup(label) : NULL;
    return op;
}

Operand createStringOperand(const char* string) {
    Operand op;
    op.type = OPERAND_STRING;
    op.value.name = string ? strdup(string) : NULL;
    return op;
}

/* Создание пустого буфера линейного кода */
LinearCode* createLinearCode(void) {
    LinearCode* code = (LinearCode*)malloc(sizeof(LinearCode));
    if (!code) {
        return NULL;
    }

    code->max_instructions = 256;
    code->instruction_count = 0;
    code->instructions = (Instruction*)malloc((size_t)code->max_instructions * sizeof(Instruction));
    if (!code->instructions) {
        free(code);
        return NULL;
    }

    return code;
}

/* Добавление инструкции в линейный код
 * При нехватке места массив расширяется
 */
void addInstruction(LinearCode* code, InstructionType type, Operand op1, Operand op2) {
    if (!code) {
        return;
    }

    if (code->instruction_count >= code->max_instructions) {
        Instruction* resized;
        code->max_instructions *= 2;
        resized = (Instruction*)realloc(code->instructions,
            (size_t)code->max_instructions * sizeof(Instruction));
        if (!resized) {
            return;
        }
        code->instructions = resized;
    }

    code->instructions[code->instruction_count].type = type;
    code->instructions[code->instruction_count].operand1 = op1;
    code->instructions[code->instruction_count].operand2 = op2;
    code->instructions[code->instruction_count].line_number = 0;
    code->instruction_count++;
}

/* Освобождение линейного кода вместе со строковыми операндами */
void freeLinearCode(LinearCode* code) {
    int i;

    if (!code) {
        return;
    }

    for (i = 0; i < code->instruction_count; ++i) {
        Instruction* instr = &code->instructions[i];

        if ((instr->operand1.type == OPERAND_VARIABLE ||
            instr->operand1.type == OPERAND_LABEL ||
            instr->operand1.type == OPERAND_STRING) &&
            instr->operand1.value.name) {
            free(instr->operand1.value.name);
        }

        if ((instr->operand2.type == OPERAND_VARIABLE ||
            instr->operand2.type == OPERAND_LABEL ||
            instr->operand2.type == OPERAND_STRING) &&
            instr->operand2.value.name) {
            free(instr->operand2.value.name);
        }
    }

    free(code->instructions);
    free(code);
}

/* Создание allocator для регистров и памяти
 * Здесь хранятся и переменные, и временные привязки полей
 */
RegisterAllocator* createRegisterAllocator(int max_registers, int max_memory) {
    RegisterAllocator* alloc = (RegisterAllocator*)malloc(sizeof(RegisterAllocator));
    if (!alloc) {
        return NULL;
    }

    alloc->max_bindings = 256;
    alloc->binding_count = 0;
    alloc->bindings = (VariableBinding*)malloc((size_t)alloc->max_bindings * sizeof(VariableBinding));
    if (!alloc->bindings) {
        free(alloc);
        return NULL;
    }

    alloc->next_register = 0;
    alloc->next_memory_addr = 0;
    alloc->max_registers = max_registers;
    alloc->max_memory = max_memory;
    return alloc;
}

/* Выдача следующего свободного регистра */
int allocateRegister(RegisterAllocator* alloc) {
    if (!alloc || alloc->next_register >= alloc->max_registers) {
        return -1;
    }
    return alloc->next_register++;
}

/* Заглушка для освобождения регистра */
void freeRegister(RegisterAllocator* alloc, int reg) {
    (void)alloc;
    (void)reg;
}

/* Старый вариант выделения памяти для совместимости */
int allocateMemory(RegisterAllocator* alloc) {
    return allocateMemorySize(alloc, 4);
}

/* Выделение памяти с учётом размера объекта
 * Важно для user type, которые могут занимать больше 4 байт
 */
int allocateMemorySize(RegisterAllocator* alloc, int size_bytes) {
    int aligned_size;
    int addr;

    if (!alloc) {
        return -1;
    }

    aligned_size = size_bytes <= 0 ? 4 : size_bytes;
    if (aligned_size % 4 != 0) {
        aligned_size += 4 - (aligned_size % 4);
    }

    if (alloc->next_memory_addr + aligned_size > alloc->max_memory) {
        return -1;
    }

    addr = alloc->next_memory_addr;
    alloc->next_memory_addr += aligned_size;
    return addr;
}

/* Привязка имени переменной к адресу и типу
 * Так же хранятся и обычные переменные, и поля вида c.a
 */
void bindVariable(RegisterAllocator* alloc, const char* var_name,
    const char* type_name, int register_id, int memory_address,
    int size_bytes, int is_argument, int is_user_type) {
    VariableBinding* binding;

    if (!alloc || !var_name) {
        return;
    }

    if (alloc->binding_count >= alloc->max_bindings) {
        VariableBinding* resized;
        alloc->max_bindings *= 2;
        resized = (VariableBinding*)realloc(alloc->bindings,
            (size_t)alloc->max_bindings * sizeof(VariableBinding));
        if (!resized) {
            return;
        }
        alloc->bindings = resized;
    }

    binding = &alloc->bindings[alloc->binding_count++];
    binding->var_name = strdup(var_name);
    binding->type_name = type_name ? strdup(type_name) : NULL;
    binding->register_id = register_id;
    binding->memory_address = memory_address;
    binding->size_bytes = size_bytes > 0 ? size_bytes : 4;
    binding->is_argument = is_argument;
    binding->is_user_type = is_user_type;
}

/* Поиск уже созданной привязки переменной по имени */
VariableBinding* findVariableBinding(RegisterAllocator* alloc, const char* var_name) {
    int i;

    if (!alloc || !var_name) {
        return NULL;
    }

    for (i = 0; i < alloc->binding_count; ++i) {
        if (alloc->bindings[i].var_name &&
            strcmp(alloc->bindings[i].var_name, var_name) == 0) {
            return &alloc->bindings[i];
        }
    }

    return NULL;
}

/* Освобождение allocator и всех привязок переменных */
void freeRegisterAllocator(RegisterAllocator* alloc) {
    int i;

    if (!alloc) {
        return;
    }

    for (i = 0; i < alloc->binding_count; ++i) {
        free(alloc->bindings[i].var_name);
        free(alloc->bindings[i].type_name);
    }

    free(alloc->bindings);
    free(alloc);
}

/* Проверка на обычное имя переменной */
static int is_leaf_identifier(const Operation* op) {
    return op && op->op_type && strcmp(op->op_type, "IDENTIFIER") == 0;
}

/* Проверка на литерал, который можно сразу превратить в число */
static int is_leaf_literal(const Operation* op) {
    return op && op->op_type &&
        (strcmp(op->op_type, "DEC") == 0 ||
            strcmp(op->op_type, "HEX") == 0 ||
            strcmp(op->op_type, "BIN") == 0 ||
            strcmp(op->op_type, "TRUE") == 0 ||
            strcmp(op->op_type, "FALSE") == 0 ||
            strcmp(op->op_type, "CHAR") == 0);
}

/* Преобразование литерала из AST/CFG в числовое значение */
static long parse_literal_value(const Operation* op) {
    if (!op || !op->op_type) {
        return 0;
    }

    if (strcmp(op->op_type, "TRUE") == 0) {
        return 1;
    }
    if (strcmp(op->op_type, "FALSE") == 0) {
        return 0;
    }
    if (strcmp(op->op_type, "CHAR") == 0) {
        return (op->value && op->value[0]) ? (long)(unsigned char)op->value[0] : 0;
    }

    if (!op->value) {
        return 0;
    }

    return strtol(op->value, NULL, 10);
}

/* Получение типа аргумента из сигнатуры функции */
static const char* current_type_name_for_variable(const char* name) {
    FunctionArg* arg;

    if (!g_current_function || !g_current_function->signature || !name) {
        return NULL;
    }

    arg = g_current_function->signature->args;
    while (arg) {
        if (arg->name && strcmp(arg->name, name) == 0) {
            return arg->type;
        }
        arg = arg->next;
    }

    return NULL;
}

/* Отделение user type от builtin и array */
static int is_user_type_name(const char* type_name) {
    return type_name && !isBuiltinTypeName(type_name) &&
        strncmp(type_name, "array(", 6) != 0;
}

/* Создание привязки переменной, если она встретилась в выражении
 * и явной привязки ещё нет
 */
static void ensure_variable_binding(RegisterAllocator* alloc, const char* name) {
    const char* inferred_type;
    int size_bytes;

    /* если переменной ещё нет, выделяю место */
    if (!alloc || !name || !*name) {
        return;
    }

    if (findVariableBinding(alloc, name)) {
        return;
    }

    inferred_type = current_type_name_for_variable(name);
    size_bytes = getTypeStorageSize(g_current_function ? g_current_function->types : NULL,
        inferred_type);
    bindVariable(alloc, name, inferred_type, -1,
        allocateMemorySize(alloc, size_bytes), size_bytes,
        0, is_user_type_name(inferred_type));
}

/* Вариант для случаев, где тип уже точно известен
 * Например, на объявлении переменной или аргумента
 */
static void ensure_variable_binding_with_type(RegisterAllocator* alloc, const char* name,
    const char* type_name, int is_argument) {
    VariableBinding* binding;
    int size_bytes;

    /* тут уже размер зависит от типа */
    if (!alloc || !name || !*name) {
        return;
    }

    binding = findVariableBinding(alloc, name);
    size_bytes = getTypeStorageSize(g_current_function ? g_current_function->types : NULL,
        type_name);

    if (binding) {
        if (!binding->type_name && type_name) {
            binding->type_name = strdup(type_name);
            binding->size_bytes = size_bytes;
            binding->is_user_type = is_user_type_name(type_name);
        }
        if (is_argument) {
            binding->is_argument = 1;
        }
        return;
    }

    bindVariable(alloc, name, type_name, -1,
        allocateMemorySize(alloc, size_bytes), size_bytes,
        is_argument, is_user_type_name(type_name));
}

/* Расчёт адреса поля
 * Из цепочки вроде l.start.x получается точный адрес в памяти
 */
static int resolve_member_address(RegisterAllocator* alloc, Operation* op,
    const char** out_type_name) {
    const char* path[32];
    int path_count = 0;
    Operation* current = op;
    VariableBinding* base_binding;
    const char* current_type;
    int address;
    int i;

    /* разбираю цепочку a.b.c */
    while (current && current->op_type &&
        strcmp(current->op_type, "memberAccess") == 0 &&
        path_count < 32) {
        path[path_count++] = current->value;
        current = current->left;
    }

    /* внизу должна быть обычная переменная */
    if (!current || !is_leaf_identifier(current) || !current->value) {
        return -1;
    }

    base_binding = findVariableBinding(alloc, current->value);
    if (!base_binding) {
        ensure_variable_binding(alloc, current->value);
        base_binding = findVariableBinding(alloc, current->value);
    }

    if (!base_binding || !base_binding->type_name) {
        return -1;
    }

    current_type = base_binding->type_name;
    address = base_binding->memory_address;

    for (i = path_count - 1; i >= 0; --i) {
        UserTypeField* field = findUserTypeField(
            g_current_function ? g_current_function->types : NULL,
            current_type, path[i]);
        if (!field) {
            return -1;
        }
        /* добираю адрес по offset */
        address += field->offset;
        current_type = field->type_name;
    }

    if (out_type_name) {
        *out_type_name = current_type;
    }

    return address;
}

/* Преобразование операции типа в строку
 * Используется при объявлении переменной и расчёте её размера
 */
static char* type_name_from_operation(Operation* op) {
    char buffer[256];
    char* element_name;

    if (!op) return strdup("?");

    if ((strcmp(op->op_type, "IDENTIFIER") == 0 ||
        strcmp(op->op_type, "TYPEDEF") == 0) &&
        op->value) {
        return strdup(op->value);
    }

    if (strcmp(op->op_type, "array") == 0) {
        element_name = type_name_from_operation(op->left);
        snprintf(buffer, sizeof(buffer), "array(%s,%s)",
            element_name ? element_name : "?",
            op->value ? op->value : "1");
        free(element_name);
        return strdup(buffer);
    }

    if (op->value) {
        return strdup(op->value);
    }

    return strdup(op->op_type ? op->op_type : "?");
}

/* Определение типа выражения
 * Нужен в первую очередь для вызова методов и полей
 */
static const char* resolve_expression_type(RegisterAllocator* alloc, Operation* op) {
    VariableBinding* binding;
    const char* field_type = NULL;

    if (!op) {
        return NULL;
    }

    if (is_leaf_identifier(op)) {
        binding = findVariableBinding(alloc, op->value);
        if (!binding) {
            ensure_variable_binding(alloc, op->value);
            binding = findVariableBinding(alloc, op->value);
        }
        return binding ? binding->type_name : NULL;
    }

    if (strcmp(op->op_type, "memberAccess") == 0) {
        resolve_member_address(alloc, op, &field_type);
        return field_type;
    }

    return NULL;
}

/* Сборка строки вида c.a или l.start.x
 * Потом это имя используется как ключ во внутренней таблице привязок
 */
static char* member_binding_name_from_operation(Operation* op) {
    char buffer[256];
    Operation* chain[32];
    int chain_len = 0;
    Operation* current = op;
    int used;
    int i;

    while (current && current->op_type &&
        strcmp(current->op_type, "memberAccess") == 0 &&
        chain_len < 32) {
        chain[chain_len++] = current;
        current = current->left;
    }

    if (!current || !is_leaf_identifier(current) || !current->value) {
        return NULL;
    }

    used = snprintf(buffer, sizeof(buffer), "%s", current->value);
    for (i = chain_len - 1; i >= 0 && used < (int)sizeof(buffer); --i) {
        used += snprintf(buffer + used, sizeof(buffer) - (size_t)used,
            ".%s", chain[i]->value ? chain[i]->value : "?");
    }

    return strdup(buffer);
}

/* Создание временной привязки для поля
 * После этого с полем можно работать почти как с обычной переменной
 */
static char* ensure_member_binding(RegisterAllocator* alloc, Operation* op) {
    const char* member_type = NULL;
    int member_address;
    char* binding_name;

    /* делаю временную запись для поля */
    member_address = resolve_member_address(alloc, op, &member_type);
    if (member_address < 0) {
        return NULL;
    }

    binding_name = member_binding_name_from_operation(op);
    if (!binding_name) {
        return NULL;
    }

    if (!findVariableBinding(alloc, binding_name)) {
        bindVariable(alloc, binding_name, member_type, -1,
            member_address, 4, 0, is_user_type_name(member_type));
    }

    return binding_name;
}

/* Общий генератор выражений
 * Почти вся логика codegen сводится к рекурсивному обходу через эту функцию
 */
static void emit_expression(LinearCode* code, RegisterAllocator* alloc, Operation* op, int target_reg);

/* Связка внутреннего имени операции с asm-инструкцией */
static InstructionType map_binary_instr(const char* op_type) {
    if (!op_type) return INSTR_END;
    if (strcmp(op_type, "PLUS") == 0) return INSTR_ADD;
    if (strcmp(op_type, "MINUS") == 0) return INSTR_SUB;
    if (strcmp(op_type, "STAR") == 0) return INSTR_MUL;
    if (strcmp(op_type, "SLASH") == 0) return INSTR_DIV;
    if (strcmp(op_type, "PERCENT") == 0) return INSTR_MOD;
    if (strcmp(op_type, "AND") == 0) return INSTR_AND;
    if (strcmp(op_type, "OR") == 0) return INSTR_OR;
    if (strcmp(op_type, "BIT_AND") == 0) return INSTR_BIT_AND;
    if (strcmp(op_type, "BIT_OR") == 0) return INSTR_BIT_OR;
    if (strcmp(op_type, "BIT_XOR") == 0) return INSTR_BIT_XOR;
    if (strcmp(op_type, "SHIFT_LEFT") == 0) return INSTR_SHIFT_LEFT;
    if (strcmp(op_type, "SHIFT_RIGHT") == 0) return INSTR_SHIFT_RIGHT;
    return INSTR_END;
}

/* Проверка операций сравнения */
static int is_compare_op(const char* op_type) {
    if (!op_type) return 0;
    return strcmp(op_type, "EQUALITY") == 0 ||
        strcmp(op_type, "NOTEQUAL") == 0 ||
        strcmp(op_type, "LESSTHAN") == 0 ||
        strcmp(op_type, "GREATERTHAN") == 0 ||
        strcmp(op_type, "LESSTHANEQ") == 0 ||
        strcmp(op_type, "GREATERTHANEQ") == 0;
}

/* Выбор условного перехода для сравнения */
static InstructionType jump_for_compare(const char* op_type) {
    if (strcmp(op_type, "EQUALITY") == 0) return INSTR_JEQ;
    if (strcmp(op_type, "NOTEQUAL") == 0) return INSTR_JNE;
    if (strcmp(op_type, "LESSTHAN") == 0) return INSTR_JLT;
    if (strcmp(op_type, "GREATERTHAN") == 0) return INSTR_JGT;
    if (strcmp(op_type, "LESSTHANEQ") == 0) return INSTR_JLE;
    if (strcmp(op_type, "GREATERTHANEQ") == 0) return INSTR_JGE;
    return INSTR_JEQ;
}

/* Генерация сравнения через CMP
 * После этого результат переводится в 0 или 1
 */
static void emit_compare_to_bool(LinearCode* code, const char* compare_op, int target_reg) {
    char true_lbl[64];
    char end_lbl[64];

    make_auto_label(true_lbl, sizeof(true_lbl), "cmp_true");
    make_auto_label(end_lbl, sizeof(end_lbl), "cmp_end");

    addInstruction(code, jump_for_compare(compare_op), createLabelOperand(true_lbl), createConstantOperand(0));
    addInstruction(code, INSTR_LOAD_CONST, createRegisterOperand(target_reg), createConstantOperand(0));
    addInstruction(code, INSTR_JMP, createLabelOperand(end_lbl), createConstantOperand(0));
    addInstruction(code, INSTR_LABEL, createLabelOperand(true_lbl), createConstantOperand(0));
    addInstruction(code, INSTR_LOAD_CONST, createRegisterOperand(target_reg), createConstantOperand(1));
    addInstruction(code, INSTR_LABEL, createLabelOperand(end_lbl), createConstantOperand(0));
}

/* Генерация обычного CALL
 * Пока используется только первый аргумент
 */
static void emit_call(LinearCode* code, RegisterAllocator* alloc, Operation* op, int target_reg) {
    /* пока беру только первый аргумент */
    if (op && op->right && strcmp(op->right->op_type, "optionalListExpr") == 0 &&
        op->right->left && strcmp(op->right->left->op_type, "listExpr") == 0 &&
        op->right->left->left) {
        emit_expression(code, alloc, op->right->left->left, 0);
    }

    if (op && op->left && op->left->value) {
        addInstruction(code, INSTR_CALL, createLabelOperand(op->left->value), createConstantOperand(0));
    }

    if (target_reg != 0) {
        addInstruction(code, INSTR_MOV, createRegisterOperand(target_reg), createRegisterOperand(0));
    }
}

/* Генерация вызова метода
 * Сначала определяется тип объекта слева, потом ищется полное имя метода
 */
static void emit_method_call(LinearCode* code, RegisterAllocator* alloc, Operation* op, int target_reg) {
    const char* object_type;
    UserTypeMethod* method;

    /* сначала пытаюсь понять тип слева */
    if (!op || !op->value) {
        return;
    }

    object_type = resolve_expression_type(alloc, op->left);
    method = findUserTypeMethod(g_current_function ? g_current_function->types : NULL,
        object_type, op->value);

    if (op->right && strcmp(op->right->op_type, "optionalListExpr") == 0 &&
        op->right->left && strcmp(op->right->left->op_type, "listExpr") == 0 &&
        op->right->left->left) {
        emit_expression(code, alloc, op->right->left->left, 0);
    }

    if (method && method->full_name) {
        /* если тип найден, беру полное имя */
        addInstruction(code, INSTR_CALL, createLabelOperand(method->full_name), createConstantOperand(0));
    }
    else {
        addInstruction(code, INSTR_CALL, createLabelOperand(op->value), createConstantOperand(0));
    }

    if (target_reg != 0) {
        addInstruction(code, INSTR_MOV, createRegisterOperand(target_reg), createRegisterOperand(0));
    }
}

/* Главный рекурсивный генератор выражений
 * Здесь обрабатываются литералы, переменные, поля, вызовы, присваивания и бинарные операции
 */
static void emit_expression(LinearCode* code, RegisterAllocator* alloc, Operation* op, int target_reg) {
    char* member_binding_name;
    int temp_reg;
    InstructionType instr;

    if (!code) {
        return;
    }

    if (!op) {
        addInstruction(code, INSTR_LOAD_CONST, createRegisterOperand(target_reg), createConstantOperand(0));
        return;
    }

    if (strcmp(op->op_type, "braces") == 0) {
        emit_expression(code, alloc, op->left, target_reg);
        return;
    }

    if (is_leaf_identifier(op)) {
        ensure_variable_binding(alloc, op->value);
        addInstruction(code, INSTR_MOV, createRegisterOperand(target_reg), createVariableOperand(op->value));
        return;
    }

    if (is_leaf_literal(op)) {
        addInstruction(code, INSTR_LOAD_CONST, createRegisterOperand(target_reg), createConstantOperand(parse_literal_value(op)));
        return;
    }

    if (strcmp(op->op_type, "STR") == 0) {
        addInstruction(code, INSTR_PRINT_STR, createStringOperand(op->value ? op->value : ""), createConstantOperand(0));
        addInstruction(code, INSTR_LOAD_CONST, createRegisterOperand(target_reg), createConstantOperand(0));
        return;
    }

    if (strcmp(op->op_type, "CALL") == 0) {
        emit_call(code, alloc, op, target_reg);
        return;
    }

    if (strcmp(op->op_type, "METHOD_CALL") == 0) {
        emit_method_call(code, alloc, op, target_reg);
        return;
    }

    if (strcmp(op->op_type, "memberAccess") == 0) {
        /* чтение поля */
        member_binding_name = ensure_member_binding(alloc, op);
        if (member_binding_name) {
            addInstruction(code, INSTR_MOV, createRegisterOperand(target_reg), createVariableOperand(member_binding_name));
            free(member_binding_name);
            return;
        }
    }

    if (strcmp(op->op_type, "NOT") == 0 && op->right == NULL) {
        emit_expression(code, alloc, op->left, target_reg);
        addInstruction(code, INSTR_NOT, createRegisterOperand(target_reg), createConstantOperand(0));
        return;
    }

    if (strcmp(op->op_type, "MINUS") == 0 && op->right == NULL) {
        emit_expression(code, alloc, op->left, target_reg);
        addInstruction(code, INSTR_LOAD_CONST, createRegisterOperand(3), createConstantOperand(-1));
        addInstruction(code, INSTR_MUL, createRegisterOperand(target_reg), createRegisterOperand(3));
        return;
    }

    if (strcmp(op->op_type, "assignment") == 0) {
        if (op->left && is_leaf_identifier(op->left)) {
            ensure_variable_binding(alloc, op->left->value);
            emit_expression(code, alloc, op->right, target_reg);
            addInstruction(code, INSTR_MOV, createVariableOperand(op->left->value), createRegisterOperand(target_reg));
        }
        else if (op->left && op->left->op_type &&
            strcmp(op->left->op_type, "memberAccess") == 0) {
            /* запись в поле */
            emit_expression(code, alloc, op->right, target_reg);
            member_binding_name = ensure_member_binding(alloc, op->left);
            if (member_binding_name) {
                addInstruction(code, INSTR_MOV, createVariableOperand(member_binding_name), createRegisterOperand(target_reg));
                free(member_binding_name);
            }
        }
        return;
    }

    if (strcmp(op->op_type, "IF_COND") == 0 ||
        strcmp(op->op_type, "LOOP_COND") == 0 ||
        strcmp(op->op_type, "REPEAT_COND") == 0) {
        emit_expression(code, alloc, op->left, target_reg);
        addInstruction(code, INSTR_CMP, createRegisterOperand(target_reg), createConstantOperand(0));
        return;
    }

    if (strcmp(op->op_type, "VAR_DECL") == 0) {
        return;
    }

    temp_reg = (target_reg == 0) ? 1 : 0;
    emit_expression(code, alloc, op->left, target_reg);
    addInstruction(code, INSTR_MOV, createRegisterOperand(temp_reg), createRegisterOperand(target_reg));
    emit_expression(code, alloc, op->right, target_reg);

    if (is_compare_op(op->op_type)) {
        addInstruction(code, INSTR_CMP, createRegisterOperand(temp_reg), createRegisterOperand(target_reg));
        emit_compare_to_bool(code, op->op_type, target_reg);
        return;
    }

    instr = map_binary_instr(op->op_type);
    if (instr != INSTR_END) {
        addInstruction(code, instr, createRegisterOperand(temp_reg), createRegisterOperand(target_reg));
        addInstruction(code, INSTR_MOV, createRegisterOperand(target_reg), createRegisterOperand(temp_reg));
        return;
    }

    addInstruction(code, INSTR_LOAD_CONST, createRegisterOperand(target_reg), createConstantOperand(0));
}

/* Разбор объявления переменных
 * Здесь же сразу выделяется память нужного размера для user type
 */
static void emit_var_decls(RegisterAllocator* alloc, Operation* type_op,
    Operation* node, LinearCode* code) {
    char* declared_type_name;

    /* тут сразу выделяю память */
    if (!node) {
        return;
    }

    if (node->op_type && strcmp(node->op_type, "listVarDeclared") == 0) {
        emit_var_decls(alloc, type_op, node->left, code);
        emit_var_decls(alloc, type_op, node->right, code);
        return;
    }

    declared_type_name = type_name_from_operation(type_op);

    if (is_leaf_identifier(node) && node->value) {
        ensure_variable_binding_with_type(alloc, node->value, declared_type_name, 0);
        free(declared_type_name);
        return;
    }

    if (node->op_type && strcmp(node->op_type, "assignment") == 0 &&
        node->left && is_leaf_identifier(node->left)) {
        ensure_variable_binding_with_type(alloc, node->left->value, declared_type_name, 0);
        free(declared_type_name);
        emit_expression(code, alloc, node, 0);
        return;
    }

    free(declared_type_name);
    emit_var_decls(alloc, type_op, node->left, code);
    emit_var_decls(alloc, type_op, node->right, code);
}

/* Использование переменной r как результата функции */
static void emit_implicit_return(LinearCode* code, RegisterAllocator* alloc) {
    VariableBinding* result_binding;

    /* если есть r, считаю её результатом */
    if (!code || !alloc) {
        return;
    }

    result_binding = findVariableBinding(alloc, "r");
    if (result_binding) {
        addInstruction(code, INSTR_MOV, createRegisterOperand(0), createVariableOperand("r"));
    }
}

/* Внутренние имена меток для последующего префикса именем функции */
static int is_internal_label_name(const char* name) {
    if (!name) {
        return 0;
    }

    return (name[0] == 'L') || strncmp(name, "cmp_", 4) == 0;
}

/* Добавление имени функции к внутренним меткам */
static void namespace_internal_labels(CompiledFunction* compiled) {
    int i;

    if (!compiled || !compiled->code || !compiled->signature || !compiled->signature->name) {
        return;
    }

    for (i = 0; i < compiled->code->instruction_count; i++) {
        Instruction* instr = &compiled->code->instructions[i];

        if (instr->operand1.type == OPERAND_LABEL &&
            instr->operand1.value.name &&
            is_internal_label_name(instr->operand1.value.name)) {
            char namespaced[256];
            char* old_name = instr->operand1.value.name;

            snprintf(namespaced, sizeof(namespaced), "%s_%s",
                compiled->signature->name,
                old_name);

            instr->operand1.value.name = strdup(namespaced);
            free(old_name);
        }
    }
}

/* Генерация одной операции верхнего уровня в block */
static void emit_operation(LinearCode* code, RegisterAllocator* alloc, Operation* op) {
    if (!code || !op) {
        return;
    }

    if (strcmp(op->op_type, "VAR_DECL") == 0) {
        emit_var_decls(alloc, op->left, op->right, code);
        return;
    }

    if (strcmp(op->op_type, "BREAK") == 0) {
        return;
    }

    if (strcmp(op->op_type, "RETURN") == 0) {
        if (op->left) {
            emit_expression(code, alloc, op->left, 0);
        }
        else {
            addInstruction(code, INSTR_LOAD_CONST,
                createRegisterOperand(0), createConstantOperand(0));
        }
        addInstruction(code, INSTR_RET,
            createRegisterOperand(0), createConstantOperand(0));
        return;
    }

    emit_expression(code, alloc, op, 0);
}

/* Поиск максимального id блока для visited-массива */
static int max_block_id(const CFG* cfg) {
    int max_id = -1;
    BasicBlock* bb = cfg ? cfg->blocks : NULL;
    while (bb) {
        if (bb->id > max_id) {
            max_id = bb->id;
        }
        bb = bb->next;
    }
    return max_id;
}

/* Печать метки блока в линейный код */
static void emit_block_label(LinearCode* code, int block_id) {
    char label[64];
    snprintf(label, sizeof(label), "L%d", block_id);
    addInstruction(code, INSTR_LABEL, createLabelOperand(label), createConstantOperand(0));
}

/* Переход в другой basic block по его номеру */
static void emit_jump_to_block(LinearCode* code, InstructionType jump_type, const BasicBlock* target) {
    char label[64];
    snprintf(label, sizeof(label), "L%d", target->id);
    addInstruction(code, jump_type, createLabelOperand(label), createConstantOperand(0));
}

/* Обход CFG в глубину и раскладка в линейный код */
static void generate_block_recursive(BasicBlock* block,
    LinearCode* code,
    RegisterAllocator* alloc,
    unsigned char* visited,
    int visited_size) {
    Operation* op;

    if (!block || !code || !alloc ||
        block->id < 0 || block->id >= visited_size ||
        visited[block->id]) {
        return;
    }

    visited[block->id] = 1;
    emit_block_label(code, block->id);

    op = block->operations;
    while (op) {
        emit_operation(code, alloc, op);
        op = op->next;
    }

    if (block->true_target && block->false_target) {
        emit_jump_to_block(code, INSTR_JNE, block->true_target);
        emit_jump_to_block(code, INSTR_JMP, block->false_target);
    }
    else if (block->true_target) {
        emit_jump_to_block(code, INSTR_JMP, block->true_target);
    }
    else if (block->is_exit) {
        emit_implicit_return(code, alloc);
        addInstruction(code, INSTR_RET, createRegisterOperand(0), createConstantOperand(0));
    }

    generate_block_recursive(block->true_target, code, alloc, visited, visited_size);
    generate_block_recursive(block->false_target, code, alloc, visited, visited_size);
}

/* Генерация линейного кода для одной функции
 * Здесь создаётся свой allocator, ставится имя функции и обходится её CFG
 */
CompiledFunction* generateCodeFromFunction(Function* cfg_func) {
    CompiledFunction* compiled;
    int max_id;
    int visited_size;
    unsigned char* visited;

    /* новая функция - новый набор всего */
    if (!cfg_func || !cfg_func->cfg) {
        return NULL;
    }

    compiled = (CompiledFunction*)malloc(sizeof(CompiledFunction));
    if (!compiled) {
        return NULL;
    }

    compiled->signature = cfg_func->signature;
    compiled->code = createLinearCode();
    compiled->alloc = createRegisterAllocator(16, 1024);

    if (!compiled->code || !compiled->alloc) {
        if (compiled->code) freeLinearCode(compiled->code);
        if (compiled->alloc) freeRegisterAllocator(compiled->alloc);
        free(compiled);
        return NULL;
    }

    g_current_function = cfg_func;

    if (cfg_func->signature && cfg_func->signature->name) {
        addInstruction(compiled->code, INSTR_LABEL, createLabelOperand(cfg_func->signature->name), createConstantOperand(0));
    }

    if (cfg_func->signature && cfg_func->signature->args) {
        FunctionArg* arg = cfg_func->signature->args;
        if (arg && arg->name) {
            /* первый аргумент сразу сохраняю */
            ensure_variable_binding_with_type(compiled->alloc, arg->name, arg->type, 1);
            addInstruction(compiled->code, INSTR_MOV, createVariableOperand(arg->name), createRegisterOperand(0));
        }
    }

    max_id = max_block_id(cfg_func->cfg);
    visited_size = max_id + 1;
    if (visited_size < 1) {
        visited_size = 1;
    }

    visited = (unsigned char*)calloc((size_t)visited_size, sizeof(unsigned char));
    if (!visited) {
        freeLinearCode(compiled->code);
        freeRegisterAllocator(compiled->alloc);
        free(compiled);
        g_current_function = NULL;
        return NULL;
    }

    generate_block_recursive(cfg_func->cfg->entry_block, compiled->code, compiled->alloc, visited, visited_size);
    free(visited);

    if (compiled->code->instruction_count == 0 ||
        compiled->code->instructions[compiled->code->instruction_count - 1].type != INSTR_RET) {
        emit_implicit_return(compiled->code, compiled->alloc);
        addInstruction(compiled->code, INSTR_RET, createRegisterOperand(0), createConstantOperand(0));
    }

    namespace_internal_labels(compiled);
    g_current_function = NULL;
    return compiled;
}

/* Генерация кода сразу для всех функций программы */
CompiledFunctionCollection* generateCodeFromAST(FunctionCollection* functions) {
    CompiledFunctionCollection* collection;
    Function* func;

    if (!functions) {
        return NULL;
    }

    collection = (CompiledFunctionCollection*)malloc(sizeof(CompiledFunctionCollection));
    if (!collection) {
        return NULL;
    }

    collection->max_functions = functions->function_count + 10;
    collection->function_count = 0;
    collection->functions = (CompiledFunction*)malloc((size_t)collection->max_functions * sizeof(CompiledFunction));
    if (!collection->functions) {
        free(collection);
        return NULL;
    }

    func = functions->functions;
    while (func) {
        CompiledFunction* compiled = generateCodeFromFunction(func);
        if (compiled) {
            collection->functions[collection->function_count++] = *compiled;
            free(compiled);
        }
        func = func->next;
    }

    return collection;
}

/* Освобождение одной compiled function */
void freeCompiledFunction(CompiledFunction* func) {
    if (!func) {
        return;
    }
    if (func->code) {
        freeLinearCode(func->code);
    }
    if (func->alloc) {
        freeRegisterAllocator(func->alloc);
    }
    free(func);
}

/* Освобождение всей коллекции compiled functions */
void freeCompiledFunctionCollection(CompiledFunctionCollection* collection) {
    int i;

    if (!collection) {
        return;
    }

    for (i = 0; i < collection->function_count; ++i) {
        if (collection->functions[i].code) {
            freeLinearCode(collection->functions[i].code);
        }
        if (collection->functions[i].alloc) {
            freeRegisterAllocator(collection->functions[i].alloc);
        }
    }

    free(collection->functions);
    free(collection);
}
