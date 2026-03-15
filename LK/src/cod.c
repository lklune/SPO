#include "codegen.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int getRegisterForVariable(RegisterAllocator* alloc, const char* var_name) {
    if (!var_name) return -1;

    VariableBinding* binding = findVariableBinding(alloc, var_name);
    if (binding && binding->register_id >= 0) {
        return binding->register_id;
    }

    int reg = allocateRegister(alloc);
    if (reg >= 0) {
        bindVariable(alloc, var_name, reg, -1);
    }
    return reg;
}

static void loadValueIntoRegister(LinearCode* code, int reg,
    const char* value_str,
    RegisterAllocator* alloc) {
    if (!code || reg < 0) return;

    char* endptr;
    long val = strtol(value_str, &endptr, 10);

    if (*endptr == '\0') {
        addInstruction(code, INSTR_LOAD_CONST,
            createRegisterOperand(reg),
            createConstantOperand(val));
    }
    else {
        addInstruction(code, INSTR_MOV,
            createRegisterOperand(reg),
            createVariableOperand(value_str));
    }
}

static void generateCodeFromOperation(LinearCode* code, Operation* op,
    RegisterAllocator* alloc) {
    if (!code || !op) return;

    const char* op_type = op->op_type ? op->op_type : "";

    if (strcmp(op_type, "IDENTIFIER") == 0 ||
        strcmp(op_type, "DEC") == 0 ||
        strcmp(op_type, "HEX") == 0 ||
        strcmp(op_type, "BIN") == 0) {
        return;
    }

    if (strcmp(op_type, "SUM") == 0) {
        // a + b
        int reg1 = allocateRegister(alloc);
        int reg2 = allocateRegister(alloc);

        if (op->left) generateCodeFromOperation(code, op->left, alloc);
        if (op->right) generateCodeFromOperation(code, op->right, alloc);

        addInstruction(code, INSTR_ADD,
            createRegisterOperand(reg1),
            createRegisterOperand(reg2));

        return;
    }

    if (strcmp(op_type, "MINUS") == 0) {
        if (op->right) {
            int reg1 = allocateRegister(alloc);
            int reg2 = allocateRegister(alloc);

            if (op->left) generateCodeFromOperation(code, op->left, alloc);
            if (op->right) generateCodeFromOperation(code, op->right, alloc);

            addInstruction(code, INSTR_SUB,
                createRegisterOperand(reg1),
                createRegisterOperand(reg2));
        }
        else if (op->left) {
            int reg = allocateRegister(alloc);
            generateCodeFromOperation(code, op->left, alloc);
            addInstruction(code, INSTR_LOAD_CONST,
                createRegisterOperand(reg),
                createConstantOperand(-1));
        }
        return;
    }

    if (strcmp(op_type, "MUL") == 0) {
        int reg1 = allocateRegister(alloc);
        int reg2 = allocateRegister(alloc);

        if (op->left) generateCodeFromOperation(code, op->left, alloc);
        if (op->right) generateCodeFromOperation(code, op->right, alloc);

        addInstruction(code, INSTR_MUL,
            createRegisterOperand(reg1),
            createRegisterOperand(reg2));
        return;
    }

    if (strcmp(op_type, "SLASH") == 0) {
        int reg1 = allocateRegister(alloc);
        int reg2 = allocateRegister(alloc);

        if (op->left) generateCodeFromOperation(code, op->left, alloc);
        if (op->right) generateCodeFromOperation(code, op->right, alloc);

        addInstruction(code, INSTR_DIV,
            createRegisterOperand(reg1),
            createRegisterOperand(reg2));
        return;
    }

    if (strcmp(op_type, "AND") == 0) {
        int reg1 = allocateRegister(alloc);
        int reg2 = allocateRegister(alloc);

        if (op->left) generateCodeFromOperation(code, op->left, alloc);
        if (op->right) generateCodeFromOperation(code, op->right, alloc);

        addInstruction(code, INSTR_AND,
            createRegisterOperand(reg1),
            createRegisterOperand(reg2));
        return;
    }

    if (strcmp(op_type, "OR") == 0) {
        int reg1 = allocateRegister(alloc);
        int reg2 = allocateRegister(alloc);

        if (op->left) generateCodeFromOperation(code, op->left, alloc);
        if (op->right) generateCodeFromOperation(code, op->right, alloc);

        addInstruction(code, INSTR_OR,
            createRegisterOperand(reg1),
            createRegisterOperand(reg2));
        return;
    }

    if (strcmp(op_type, "NOT") == 0) {
        int reg = allocateRegister(alloc);

        if (op->left) generateCodeFromOperation(code, op->left, alloc);

        addInstruction(code, INSTR_NOT,
            createRegisterOperand(reg),
            createConstantOperand(0));
        return;
    }

    if (strcmp(op_type, "EQUALITY") == 0) {
        int reg1 = allocateRegister(alloc);
        int reg2 = allocateRegister(alloc);

        if (op->left) generateCodeFromOperation(code, op->left, alloc);
        if (op->right) generateCodeFromOperation(code, op->right, alloc);

        addInstruction(code, INSTR_CMP,
            createRegisterOperand(reg1),
            createRegisterOperand(reg2));
        return;
    }

    if (strcmp(op_type, "LESSTHAN") == 0 ||
        strcmp(op_type, "GREATERTHAN") == 0 ||
        strcmp(op_type, "LESSTHANEQ") == 0 ||
        strcmp(op_type, "GREATERTHANEQ") == 0) {

        int reg1 = allocateRegister(alloc);
        int reg2 = allocateRegister(alloc);

        if (op->left) generateCodeFromOperation(code, op->left, alloc);
        if (op->right) generateCodeFromOperation(code, op->right, alloc);

        addInstruction(code, INSTR_CMP,
            createRegisterOperand(reg1),
            createRegisterOperand(reg2));
        return;
    }

    if (strcmp(op_type, "assignment") == 0) {
        if (op->left && op->right) {
            int reg = allocateRegister(alloc);

            generateCodeFromOperation(code, op->right, alloc);

            if (op->left->op_type && strcmp(op->left->op_type, "IDENTIFIER") == 0) {
                char* var_name = op->left->value ? op->left->value : "?";
                addInstruction(code, INSTR_MOV,
                    createVariableOperand(var_name),
                    createRegisterOperand(reg));
            }
        }
        return;
    }
}

static int block_counter = 0;

static void generateCodeFromBlock(LinearCode* code, BasicBlock* block,
    RegisterAllocator* alloc) {
    if (!code || !block) return;

    char label[64];
    snprintf(label, sizeof(label), "L%d", block->id);
    addInstruction(code, INSTR_LABEL,
        createLabelOperand(label),
        createConstantOperand(0));

    Operation* op = block->operations;
    while (op) {
        generateCodeFromOperation(code, op, alloc);
        op = op->next;
    }

    if (block->true_target && block->false_target) {
        char true_label[64], false_label[64];
        snprintf(true_label, sizeof(true_label), "L%d", block->true_target->id);
        snprintf(false_label, sizeof(false_label), "L%d", block->false_target->id);

        addInstruction(code, INSTR_JEQ,
            createLabelOperand(true_label),
            createConstantOperand(0));
        addInstruction(code, INSTR_JMP,
            createLabelOperand(false_label),
            createConstantOperand(0));
    }
    else if (block->true_target) {
        char target_label[64];
        snprintf(target_label, sizeof(target_label), "L%d", block->true_target->id);
        addInstruction(code, INSTR_JMP,
            createLabelOperand(target_label),
            createConstantOperand(0));
    }
    else if (block->is_exit) {
        addInstruction(code, INSTR_RET,
            createConstantOperand(0),
            createConstantOperand(0));
    }
}