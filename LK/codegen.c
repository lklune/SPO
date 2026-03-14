//
// codegen.c - генератор линейного кода из CFG
//

#include "codegen.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
    if (!code) return NULL;

    code->max_instructions = 256;
    code->instruction_count = 0;
    code->instructions = (Instruction*)malloc(code->max_instructions *
        sizeof(Instruction));

    if (!code->instructions) {
        free(code);
        return NULL;
    }

    return code;
}

void addInstruction(LinearCode* code, InstructionType type,
    Operand op1, Operand op2) {
    if (!code) return;

    // Увеличить буфер если нужно
    if (code->instruction_count >= code->max_instructions) {
        code->max_instructions *= 2;
        Instruction* new_instrs = (Instruction*)realloc(code->instructions,
            code->max_instructions * sizeof(Instruction));
        if (!new_instrs) return;
        code->instructions = new_instrs;
    }

    Instruction* instr = &code->instructions[code->instruction_count++];
    instr->type = type;
    instr->operand1 = op1;
    instr->operand2 = op2;
    instr->line_number = 0;
}

void freeLinearCode(LinearCode* code) {
    if (!code) return;

    for (int i = 0; i < code->instruction_count; i++) {
        Instruction* instr = &code->instructions[i];

        if (instr->operand1.type == OPERAND_VARIABLE ||
            instr->operand1.type == OPERAND_LABEL ||
            instr->operand1.type == OPERAND_STRING) {
            free(instr->operand1.value.name);
        }

        if (instr->operand2.type == OPERAND_VARIABLE ||
            instr->operand2.type == OPERAND_LABEL ||
            instr->operand2.type == OPERAND_STRING) {
            free(instr->operand2.value.name);
        }
    }

    free(code->instructions);
    free(code);
}

RegisterAllocator* createRegisterAllocator(int max_registers, int max_memory) {
    RegisterAllocator* alloc = (RegisterAllocator*)malloc(sizeof(RegisterAllocator));
    if (!alloc) return NULL;

    alloc->max_bindings = 256;
    alloc->binding_count = 0;
    alloc->bindings = (VariableBinding*)malloc(alloc->max_bindings *
        sizeof(VariableBinding));

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
    if (!alloc || reg < 0) return;
}

int allocateMemory(RegisterAllocator* alloc) {
    if (!alloc || alloc->next_memory_addr >= alloc->max_memory) {
        return -1;
    }
    return alloc->next_memory_addr++;
}

void bindVariable(RegisterAllocator* alloc, const char* var_name,
    int register_id, int memory_address) {
    if (!alloc || !var_name) return;

    if (alloc->binding_count >= alloc->max_bindings) {
        alloc->max_bindings *= 2;
        VariableBinding* new_bindings = (VariableBinding*)realloc(alloc->bindings,
            alloc->max_bindings * sizeof(VariableBinding));
        if (!new_bindings) return;
        alloc->bindings = new_bindings;
    }

    VariableBinding* binding = &alloc->bindings[alloc->binding_count++];
    binding->var_name = strdup(var_name);
    binding->register_id = register_id;
    binding->memory_address = memory_address;
    binding->is_constant = 0;
    binding->constant_value = 0;
}

VariableBinding* findVariableBinding(RegisterAllocator* alloc,
    const char* var_name) {
    if (!alloc || !var_name) return NULL;

    for (int i = 0; i < alloc->binding_count; i++) {
        if (strcmp(alloc->bindings[i].var_name, var_name) == 0) {
            return &alloc->bindings[i];
        }
    }

    return NULL;
}

void freeRegisterAllocator(RegisterAllocator* alloc) {
    if (!alloc) return;

    for (int i = 0; i < alloc->binding_count; i++) {
        free(alloc->bindings[i].var_name);
    }
    free(alloc->bindings);
    free(alloc);
}

CompiledFunctionCollection* generateCodeFromAST(FunctionCollection* functions) {
    if (!functions) return NULL;

    CompiledFunctionCollection* collection =
        (CompiledFunctionCollection*)malloc(sizeof(CompiledFunctionCollection));
    if (!collection) return NULL;

    collection->max_functions = functions->function_count + 10;
    collection->function_count = 0;
    collection->functions = (CompiledFunction*)malloc(collection->max_functions *
        sizeof(CompiledFunction));

    if (!collection->functions) {
        free(collection);
        return NULL;
    }

    // Скомпилировать каждую функцию
    Function* func = functions->functions;
    while (func) {
        CompiledFunction* compiled = generateCodeFromFunction(func);
        if (compiled) {
            collection->functions[collection->function_count++] = *compiled;
            free(compiled);  // освободить временный указатель
        }
        func = func->next;
    }

    return collection;
}

typedef struct {
    BasicBlock** blocks;
    int count;
    int max;
} BlockStack;

static BlockStack* createBlockStack(void) {
    BlockStack* stack = (BlockStack*)malloc(sizeof(BlockStack));
    stack->max = 256;
    stack->count = 0;
    stack->blocks = (BasicBlock**)malloc(sizeof(BasicBlock*) * stack->max);
    return stack;
}

static void pushBlock(BlockStack* stack, BasicBlock* block) {
    if (stack->count < stack->max) {
        stack->blocks[stack->count++] = block;
    }
}

static BasicBlock* popBlock(BlockStack* stack) {
    if (stack->count > 0) {
        return stack->blocks[--stack->count];
    }
    return NULL;
}

static void freeBlockStack(BlockStack* stack) {
    free(stack->blocks);
    free(stack);
}

static void generateCodeFromBlock(BasicBlock* block, LinearCode* code,
    RegisterAllocator* alloc, int* visited, int* visited_count) {

    if (!block) return;

    // Проверить, был ли блок уже обработан
    for (int i = 0; i < *visited_count; i++) {
        if (visited[i] == block->id) return;
    }
    visited[(*visited_count)++] = block->id;

    if (block->operations) {
        Operation* op = block->operations;
        while (op) {
            op = op->next;
        }
    }

    if (block->true_target) {
        generateCodeFromBlock(block->true_target, code, alloc, visited, visited_count);
    }

    if (block->false_target) {
        generateCodeFromBlock(block->false_target, code, alloc, visited, visited_count);
    }
}

CompiledFunction* generateCodeFromFunction(Function* cfg_func) {
    if (!cfg_func || !cfg_func->cfg) return NULL;

    CompiledFunction* compiled = (CompiledFunction*)malloc(sizeof(CompiledFunction));
    if (!compiled) return NULL;

    compiled->signature = cfg_func->signature;
    compiled->code = createLinearCode();
    compiled->alloc = createRegisterAllocator(16, 256);
    compiled->code_start_line = 0;

    if (!compiled->code || !compiled->alloc) {
        if (compiled->code) freeLinearCode(compiled->code);
        if (compiled->alloc) freeRegisterAllocator(compiled->alloc);
        free(compiled);
        return NULL;
    }

    int* visited = (int*)malloc(sizeof(int) * 256);
    int visited_count = 0;

    if (cfg_func->cfg->entry_block) {
        generateCodeFromBlock(cfg_func->cfg->entry_block, compiled->code,
            compiled->alloc, visited, &visited_count);
    }

    free(visited);

    if (compiled->code->instruction_count == 0 ||
        compiled->code->instructions[compiled->code->instruction_count - 1].type != INSTR_RET) {
        Operand no_op = createConstantOperand(0);
        addInstruction(compiled->code, INSTR_RET, no_op, no_op);
    }

    return compiled;
}

void freeCompiledFunction(CompiledFunction* func) {
    if (!func) return;
    if (func->code) freeLinearCode(func->code);
    if (func->alloc) freeRegisterAllocator(func->alloc);
    free(func);
}

void freeCompiledFunctionCollection(CompiledFunctionCollection* collection) {
    if (!collection) return;

    for (int i = 0; i < collection->function_count; i++) {
        freeCompiledFunction(&collection->functions[i]);
    }
    free(collection->functions);
    free(collection);
}