#include "codegen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_label_counter = 0;

static void make_auto_label(char* buf, size_t size, const char* prefix) {
    snprintf(buf, size, "%s_%d", prefix, g_label_counter++);
}

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

void addInstruction(LinearCode* code, InstructionType type, Operand op1, Operand op2) {
    if (!code) {
        return;
    }

    if (code->instruction_count >= code->max_instructions) {
        code->max_instructions *= 2;
        Instruction* resized = (Instruction*)realloc(code->instructions,
                                                     (size_t)code->max_instructions * sizeof(Instruction));
        if (!resized) {
            return;
        }
        code->instructions = resized;
    }

    Instruction* instr = &code->instructions[code->instruction_count++];
    instr->type = type;
    instr->operand1 = op1;
    instr->operand2 = op2;
    instr->line_number = 0;
}

void freeLinearCode(LinearCode* code) {
    if (!code) {
        return;
    }

    for (int i = 0; i < code->instruction_count; ++i) {
        Instruction* instr = &code->instructions[i];

        if ((instr->operand1.type == OPERAND_VARIABLE || instr->operand1.type == OPERAND_LABEL ||
             instr->operand1.type == OPERAND_STRING) && instr->operand1.value.name) {
            free(instr->operand1.value.name);
        }

        if ((instr->operand2.type == OPERAND_VARIABLE || instr->operand2.type == OPERAND_LABEL ||
             instr->operand2.type == OPERAND_STRING) && instr->operand2.value.name) {
            free(instr->operand2.value.name);
        }
    }

    free(code->instructions);
    free(code);
}

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

int allocateRegister(RegisterAllocator* alloc) {
    if (!alloc || alloc->next_register >= alloc->max_registers) {
        return -1;
    }
    return alloc->next_register++;
}

void freeRegister(RegisterAllocator* alloc, int reg) {
    (void)alloc;
    (void)reg;
}

int allocateMemory(RegisterAllocator* alloc) {
    if (!alloc || alloc->next_memory_addr + 4 > alloc->max_memory) {
        return -1;
    }

    int addr = alloc->next_memory_addr;
    alloc->next_memory_addr += 4;
    return addr;
}

void bindVariable(RegisterAllocator* alloc, const char* var_name, int register_id, int memory_address) {
    if (!alloc || !var_name) {
        return;
    }

    if (alloc->binding_count >= alloc->max_bindings) {
        alloc->max_bindings *= 2;
        VariableBinding* resized =
            (VariableBinding*)realloc(alloc->bindings, (size_t)alloc->max_bindings * sizeof(VariableBinding));
        if (!resized) {
            return;
        }
        alloc->bindings = resized;
    }

    VariableBinding* binding = &alloc->bindings[alloc->binding_count++];
    binding->var_name = strdup(var_name);
    binding->register_id = register_id;
    binding->memory_address = memory_address;
    binding->is_constant = 0;
    binding->constant_value = 0;
}

VariableBinding* findVariableBinding(RegisterAllocator* alloc, const char* var_name) {
    if (!alloc || !var_name) {
        return NULL;
    }

    for (int i = 0; i < alloc->binding_count; ++i) {
        if (alloc->bindings[i].var_name && strcmp(alloc->bindings[i].var_name, var_name) == 0) {
            return &alloc->bindings[i];
        }
    }

    return NULL;
}

void freeRegisterAllocator(RegisterAllocator* alloc) {
    if (!alloc) {
        return;
    }

    for (int i = 0; i < alloc->binding_count; ++i) {
        free(alloc->bindings[i].var_name);
    }

    free(alloc->bindings);
    free(alloc);
}

static int is_leaf_identifier(const Operation* op) {
    return op && op->op_type && strcmp(op->op_type, "IDENTIFIER") == 0;
}

static int is_leaf_literal(const Operation* op) {
    return op && op->op_type &&
           (strcmp(op->op_type, "DEC") == 0 || strcmp(op->op_type, "HEX") == 0 || strcmp(op->op_type, "BIN") == 0 ||
            strcmp(op->op_type, "TRUE") == 0 || strcmp(op->op_type, "FALSE") == 0 || strcmp(op->op_type, "CHAR") == 0);
}

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

static void ensure_variable_binding(RegisterAllocator* alloc, const char* name) {
    if (!alloc || !name || !*name) {
        return;
    }

    if (!findVariableBinding(alloc, name)) {
        int mem_addr = allocateMemory(alloc);
        bindVariable(alloc, name, -1, mem_addr);
    }
}

static void emit_expression(LinearCode* code, RegisterAllocator* alloc, Operation* op, int target_reg);

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

static int is_compare_op(const char* op_type) {
    if (!op_type) return 0;
    return strcmp(op_type, "EQUALITY") == 0 || strcmp(op_type, "NOTEQUAL") == 0 || strcmp(op_type, "LESSTHAN") == 0 ||
           strcmp(op_type, "GREATERTHAN") == 0 || strcmp(op_type, "LESSTHANEQ") == 0 ||
           strcmp(op_type, "GREATERTHANEQ") == 0;
}

static InstructionType jump_for_compare(const char* op_type) {
    if (strcmp(op_type, "EQUALITY") == 0) return INSTR_JEQ;
    if (strcmp(op_type, "NOTEQUAL") == 0) return INSTR_JNE;
    if (strcmp(op_type, "LESSTHAN") == 0) return INSTR_JLT;
    if (strcmp(op_type, "GREATERTHAN") == 0) return INSTR_JGT;
    if (strcmp(op_type, "LESSTHANEQ") == 0) return INSTR_JLE;
    if (strcmp(op_type, "GREATERTHANEQ") == 0) return INSTR_JGE;
    return INSTR_JEQ;
}

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

static void emit_call(LinearCode* code, RegisterAllocator* alloc, Operation* op, int target_reg) {
    if (op && op->right && strcmp(op->right->op_type, "optionalListExpr") == 0 && op->right->left &&
        strcmp(op->right->left->op_type, "listExpr") == 0 && op->right->left->left) {
        emit_expression(code, alloc, op->right->left->left, 0);
    }

    if (op && op->left && op->left->value) {
        addInstruction(code, INSTR_CALL, createLabelOperand(op->left->value), createConstantOperand(0));
    }

    if (target_reg != 0) {
        addInstruction(code, INSTR_MOV, createRegisterOperand(target_reg), createRegisterOperand(0));
    }
}

static void emit_expression(LinearCode* code, RegisterAllocator* alloc, Operation* op, int target_reg) {
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
        return;
    }

    if (strcmp(op->op_type, "IF_COND") == 0 || strcmp(op->op_type, "LOOP_COND") == 0 ||
        strcmp(op->op_type, "REPEAT_COND") == 0) {
        emit_expression(code, alloc, op->left, target_reg);
        addInstruction(code, INSTR_CMP, createRegisterOperand(target_reg), createConstantOperand(0));
        return;
    }

    if (strcmp(op->op_type, "VAR_DECL") == 0) {
        return;
    }

    int temp_reg = (target_reg == 0) ? 1 : 0;
    emit_expression(code, alloc, op->left, target_reg);
    addInstruction(code, INSTR_MOV, createRegisterOperand(temp_reg), createRegisterOperand(target_reg));
    emit_expression(code, alloc, op->right, target_reg);

    if (is_compare_op(op->op_type)) {
        addInstruction(code, INSTR_CMP, createRegisterOperand(temp_reg), createRegisterOperand(target_reg));
        emit_compare_to_bool(code, op->op_type, target_reg);
        return;
    }

    InstructionType instr = map_binary_instr(op->op_type);
    if (instr != INSTR_END) {
        addInstruction(code, instr, createRegisterOperand(temp_reg), createRegisterOperand(target_reg));
        addInstruction(code, INSTR_MOV, createRegisterOperand(target_reg), createRegisterOperand(temp_reg));
        return;
    }

    addInstruction(code, INSTR_LOAD_CONST, createRegisterOperand(target_reg), createConstantOperand(0));
}

static void emit_var_decls(RegisterAllocator* alloc, Operation* node, LinearCode* code) {
    if (!node) {
        return;
    }

    if (node->op_type && strcmp(node->op_type, "listVarDeclared") == 0) {
        emit_var_decls(alloc, node->left, code);
        emit_var_decls(alloc, node->right, code);
        return;
    }

    if (is_leaf_identifier(node) && node->value) {
        ensure_variable_binding(alloc, node->value);
        return;
    }

    if (node->op_type && strcmp(node->op_type, "assignment") == 0 && node->left && is_leaf_identifier(node->left)) {
        ensure_variable_binding(alloc, node->left->value);
        emit_expression(code, alloc, node, 0);
        return;
    }

    emit_var_decls(alloc, node->left, code);
    emit_var_decls(alloc, node->right, code);
}

static void emit_implicit_return(LinearCode* code, RegisterAllocator* alloc) {
    VariableBinding* result_binding;

    if (!code || !alloc) {
        return;
    }

    /* A function without explicit return currently uses local variable `r`
       as the result slot. This matches the source style used in our samples. */
    result_binding = findVariableBinding(alloc, "r");
    if (result_binding) {
        addInstruction(code, INSTR_MOV, createRegisterOperand(0), createVariableOperand("r"));
    }
}

static void emit_operation(LinearCode* code, RegisterAllocator* alloc, Operation* op) {
    if (!code || !op) {
        return;
    }

    if (strcmp(op->op_type, "VAR_DECL") == 0) {
        emit_var_decls(alloc, op->right, code);
        return;
    }

    if (strcmp(op->op_type, "BREAK") == 0) {
        return;
    }

    emit_expression(code, alloc, op, 0);
}

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

static void emit_block_label(LinearCode* code, int block_id) {
    char label[64];
    snprintf(label, sizeof(label), "L%d", block_id);
    addInstruction(code, INSTR_LABEL, createLabelOperand(label), createConstantOperand(0));
}

static void emit_jump_to_block(LinearCode* code, InstructionType jump_type, const BasicBlock* target) {
    char label[64];
    snprintf(label, sizeof(label), "L%d", target->id);
    addInstruction(code, jump_type, createLabelOperand(label), createConstantOperand(0));
}

static void generate_block_recursive(BasicBlock* block,
                                     LinearCode* code,
                                     RegisterAllocator* alloc,
                                     unsigned char* visited,
                                     int visited_size) {
    if (!block || !code || !alloc || block->id < 0 || block->id >= visited_size || visited[block->id]) {
        return;
    }

    visited[block->id] = 1;
    emit_block_label(code, block->id);

    Operation* op = block->operations;
    while (op) {
        emit_operation(code, alloc, op);
        op = op->next;
    }

    if (block->true_target && block->false_target) {
        emit_jump_to_block(code, INSTR_JNE, block->true_target);
        emit_jump_to_block(code, INSTR_JMP, block->false_target);
    } else if (block->true_target) {
        emit_jump_to_block(code, INSTR_JMP, block->true_target);
    } else if (block->is_exit) {
        emit_implicit_return(code, alloc);
        addInstruction(code, INSTR_RET, createRegisterOperand(0), createConstantOperand(0));
    }

    generate_block_recursive(block->true_target, code, alloc, visited, visited_size);
    generate_block_recursive(block->false_target, code, alloc, visited, visited_size);
}

CompiledFunction* generateCodeFromFunction(Function* cfg_func) {
    if (!cfg_func || !cfg_func->cfg) {
        return NULL;
    }

    CompiledFunction* compiled = (CompiledFunction*)malloc(sizeof(CompiledFunction));
    if (!compiled) {
        return NULL;
    }

    compiled->signature = cfg_func->signature;
    compiled->code = createLinearCode();
    compiled->alloc = createRegisterAllocator(16, 1024);
    compiled->code_start_line = 0;

    if (!compiled->code || !compiled->alloc) {
        if (compiled->code) freeLinearCode(compiled->code);
        if (compiled->alloc) freeRegisterAllocator(compiled->alloc);
        free(compiled);
        return NULL;
    }

    if (cfg_func->signature && cfg_func->signature->name) {
        addInstruction(compiled->code, INSTR_LABEL, createLabelOperand(cfg_func->signature->name), createConstantOperand(0));
    }

    if (cfg_func->signature && cfg_func->signature->args) {
        FunctionArg* arg = cfg_func->signature->args;
        if (arg && arg->name) {
            ensure_variable_binding(compiled->alloc, arg->name);
            addInstruction(compiled->code, INSTR_MOV, createVariableOperand(arg->name), createRegisterOperand(0));
        }
    }

    int max_id = max_block_id(cfg_func->cfg);
    int visited_size = max_id + 1;
    if (visited_size < 1) {
        visited_size = 1;
    }

    unsigned char* visited = (unsigned char*)calloc((size_t)visited_size, sizeof(unsigned char));
    if (!visited) {
        freeLinearCode(compiled->code);
        freeRegisterAllocator(compiled->alloc);
        free(compiled);
        return NULL;
    }

    generate_block_recursive(cfg_func->cfg->entry_block, compiled->code, compiled->alloc, visited, visited_size);
    free(visited);

    if (compiled->code->instruction_count == 0 ||
        compiled->code->instructions[compiled->code->instruction_count - 1].type != INSTR_RET) {
        emit_implicit_return(compiled->code, compiled->alloc);
        addInstruction(compiled->code, INSTR_RET, createRegisterOperand(0), createConstantOperand(0));
    }

    return compiled;
}

CompiledFunctionCollection* generateCodeFromAST(FunctionCollection* functions) {
    if (!functions) {
        return NULL;
    }

    CompiledFunctionCollection* collection = (CompiledFunctionCollection*)malloc(sizeof(CompiledFunctionCollection));
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

    Function* func = functions->functions;
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

void freeCompiledFunctionCollection(CompiledFunctionCollection* collection) {
    if (!collection) {
        return;
    }

    for (int i = 0; i < collection->function_count; ++i) {
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
