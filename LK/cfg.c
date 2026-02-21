/*
 * cfg.c
 * Реализация построения графа потока управления (CFG) из дерева разбора (AST)
 */

#include "cfg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

 /* ===================== Operation ===================== */

Operation* createOperation(char* op_type, Operation* left, Operation* right,
    const char* value, int line_number) {
    Operation* op = (Operation*)malloc(sizeof(Operation));
    if (!op) return NULL;
    op->op_type = op_type ? strdup(op_type) : NULL;
    op->left = left;
    op->right = right;
    op->value = value ? strdup(value) : NULL;
    op->line_number = line_number;
    op->next = NULL;
    return op;
}

void freeOperation(Operation* op) {
    if (!op) return;
    free(op->op_type);
    free(op->value);
    freeOperation(op->left);
    freeOperation(op->right);
    freeOperation(op->next);
    free(op);
}

/* ===================== BasicBlock ===================== */

BasicBlock* createBasicBlock(int id) {
    BasicBlock* bb = (BasicBlock*)malloc(sizeof(BasicBlock));
    if (!bb) return NULL;
    bb->id = id;
    bb->operations = NULL;
    bb->true_target = NULL;
    bb->false_target = NULL;
    bb->next = NULL;
    bb->is_entry = 0;
    bb->is_exit = 0;
    bb->ast_node = NULL;
    return bb;
}

/* Присоединить операцию к блоку (список через next-указатель) */
void addOperationToBlock(BasicBlock* block, Operation* op) {
    if (!block || !op) return;
    if (!block->operations) {
        block->operations = op;
    }
    else {
        /* идём до конца цепочки по next */
        Operation* cur = block->operations;
        while (cur->next) cur = cur->next;
        cur->next = op;
    }
}

void freeBasicBlock(BasicBlock* block) {
    if (!block) return;
    freeOperation(block->operations);
    free(block);
}

/* ===================== CFG ===================== */

CFG* createCFG(void) {
    CFG* cfg = (CFG*)malloc(sizeof(CFG));
    if (!cfg) return NULL;
    cfg->entry_block = NULL;
    cfg->exit_block = NULL;
    cfg->blocks = NULL;
    cfg->block_count = 0;
    return cfg;
}

void addBlockToCFG(CFG* cfg, BasicBlock* block) {
    if (!cfg || !block) return;
    block->next = cfg->blocks;
    cfg->blocks = block;
    cfg->block_count++;
}

void freeCFG(CFG* cfg) {
    if (!cfg) return;
    BasicBlock* cur = cfg->blocks;
    while (cur) {
        BasicBlock* nxt = cur->next;
        freeBasicBlock(cur);
        cur = nxt;
    }
    free(cfg);
}

/* ===================== Function ===================== */

Function* createFunction(FunctionSignature* signature, CFG* cfg,
    const char* source_file) {
    Function* f = (Function*)malloc(sizeof(Function));
    if (!f) return NULL;
    f->signature = signature;
    f->cfg = cfg;
    f->source_file = source_file ? strdup(source_file) : NULL;
    f->next = NULL;
    return f;
}

void freeFunction(Function* func) {
    if (!func) return;
    if (func->signature) {
        free(func->signature->name);
        free(func->signature->return_type);
        FunctionArg* arg = func->signature->args;
        while (arg) {
            FunctionArg* na = arg->next;
            free(arg->name);
            free(arg->type);
            free(arg);
            arg = na;
        }
        free(func->signature);
    }
    freeCFG(func->cfg);
    free(func->source_file);
    free(func);
}

/* ===================== Collections ===================== */

FileCollection* createFileCollection(void) {
    FileCollection* fc = (FileCollection*)malloc(sizeof(FileCollection));
    if (!fc) return NULL;
    fc->files = NULL;
    fc->file_count = 0;
    return fc;
}

void addFileToCollection(FileCollection* collection, const char* filename,
    Node* ast) {
    if (!collection) return;
    FileInfo* fi = (FileInfo*)malloc(sizeof(FileInfo));
    if (!fi) return;
    fi->filename = filename ? strdup(filename) : NULL;
    fi->ast = ast;
    fi->next = collection->files;
    collection->files = fi;
    collection->file_count++;
}

FunctionCollection* createFunctionCollection(void) {
    FunctionCollection* fc = (FunctionCollection*)malloc(sizeof(FunctionCollection));
    if (!fc) return NULL;
    fc->functions = NULL;
    fc->function_count = 0;
    return fc;
}

void addFunctionToCollection(FunctionCollection* collection, Function* func) {
    if (!collection || !func) return;
    func->next = collection->functions;
    collection->functions = func;
    collection->function_count++;
}

ErrorCollection* createErrorCollection(void) {
    ErrorCollection* ec = (ErrorCollection*)malloc(sizeof(ErrorCollection));
    if (!ec) return NULL;
    ec->errors = NULL;
    ec->error_count = 0;
    return ec;
}

void addErrorToCollection(ErrorCollection* collection, const char* message,
    const char* filename, int line_number) {
    if (!collection) return;
    Error* e = (Error*)malloc(sizeof(Error));
    if (!e) return;
    e->message = message ? strdup(message) : NULL;
    e->filename = filename ? strdup(filename) : NULL;
    e->line_number = line_number;
    e->next = collection->errors;
    collection->errors = e;
    collection->error_count++;
}

AnalysisResult* createAnalysisResult(FunctionCollection* functions,
    ErrorCollection* errors) {
    AnalysisResult* ar = (AnalysisResult*)malloc(sizeof(AnalysisResult));
    if (!ar) return NULL;
    ar->functions = functions;
    ar->errors = errors;
    return ar;
}

void freeAnalysisResult(AnalysisResult* result) {
    if (!result) return;
    if (result->functions) {
        Function* f = result->functions->functions;
        while (f) {
            Function* nf = f->next;
            freeFunction(f);
            f = nf;
        }
        free(result->functions);
    }
    if (result->errors) {
        Error* e = result->errors->errors;
        while (e) {
            Error* ne = e->next;
            free(e->message);
            free(e->filename);
            free(e);
            e = ne;
        }
        free(result->errors);
    }
    free(result);
}

/* ============================================================
 *  Вспомогательный контекст обхода AST
 * ============================================================ */

typedef struct BuildCtx {
    CFG* cfg;
    ErrorCollection* errors;
    const char* filename;
    int              block_id_counter;
    /* стек блоков-продолжений для break */
    BasicBlock* break_targets[64];
    int              break_depth;
} BuildCtx;

static BasicBlock* newBlock(BuildCtx* ctx) {
    BasicBlock* bb = createBasicBlock(ctx->block_id_counter++);
    addBlockToCFG(ctx->cfg, bb);
    return bb;
}

/* Превратить узел AST-выражения в дерево операций */
static Operation* exprToOp(Node* node) {
    if (!node) return NULL;

    const char* t = node->type ? node->type : "";
    const char* v = node->value ? node->value : "";

    /* Листья: литералы, идентификаторы */
    if (strcmp(t, "IDENTIFIER") == 0 ||
        strcmp(t, "DEC") == 0 ||
        strcmp(t, "HEX") == 0 ||
        strcmp(t, "BIN") == 0 ||
        strcmp(t, "STR") == 0 ||
        strcmp(t, "CHAR") == 0 ||
        strcmp(t, "TRUE") == 0 ||
        strcmp(t, "FALSE") == 0) {
        return createOperation((char*)t, NULL, NULL, v, 0);
    }

    /* Бинарные/унарные операции */
    Operation* left = exprToOp(node->left);
    Operation* right = exprToOp(node->right);

    /* Для именованных операций (SUM, MINUS, MUL, ...) */
    const char* op_label = (*t != '\0') ? t : ((*v != '\0') ? v : "<op>");
    Operation* op = createOperation((char*)op_label, left, right, v, 0);
    return op;
}

/*
 * Основной рекурсивный обход.
 * current  — текущий (активный) базовый блок, в который пишем инструкции
 * Возвращает блок, который является «выходом» из данного поддерева —
 * следующий код нужно добавлять в него.
 * after_block — блок, который следует после всей конструкции (для break)
 */
static BasicBlock* buildBlock(BuildCtx* ctx, Node* node,
    BasicBlock* current, BasicBlock* after_block);

static BasicBlock* buildList(BuildCtx* ctx, Node* node,
    BasicBlock* current, BasicBlock* after_block) {
    if (!node) return current;
    /* listStatement / listSourceItem — левый потомок = statement, правый = tail */
    if (strcmp(node->type ? node->type : "", "listStatement") == 0 ||
        strcmp(node->type ? node->type : "", "listSourceItem") == 0) {
        current = buildBlock(ctx, node->left, current, after_block);
        current = buildList(ctx, node->right, current, after_block);
        return current;
    }
    return buildBlock(ctx, node, current, after_block);
}

static BasicBlock* buildBlock(BuildCtx* ctx, Node* node,
    BasicBlock* current, BasicBlock* after_block) {
    if (!node || !current) return current;

    const char* t = node->type ? node->type : "";

    /* ---- listStatement / listSourceItem ---- */
    if (strcmp(t, "listStatement") == 0 || strcmp(t, "listSourceItem") == 0) {
        return buildList(ctx, node, current, after_block);
    }

    /* ---- var declaration ---- */
    if (strcmp(t, "var") == 0) {
        Operation* op = createOperation("VAR_DECL", exprToOp(node->left),
            exprToOp(node->right), NULL, 0);
        addOperationToBlock(current, op);
        if (!current->ast_node) current->ast_node = node;
        return current;
    }

    /* ---- assignment / expression ---- */
    if (strcmp(t, "assignment") == 0 ||
        strcmp(t, "SUM") == 0 || strcmp(t, "MINUS") == 0 ||
        strcmp(t, "MUL") == 0 || strcmp(t, "SLASH") == 0 ||
        strcmp(t, "PERCENT") == 0 || strcmp(t, "EQUALITY") == 0 ||
        strcmp(t, "NOTEQUAL") == 0 || strcmp(t, "LESSTHAN") == 0 ||
        strcmp(t, "GREATERTHAN") == 0 || strcmp(t, "LESSTHANEQ") == 0 ||
        strcmp(t, "GREATERTHANEQ") == 0 || strcmp(t, "AND") == 0 ||
        strcmp(t, "OR") == 0 || strcmp(t, "BIT_AND") == 0 ||
        strcmp(t, "BIT_OR") == 0 || strcmp(t, "BIT_XOR") == 0 ||
        strcmp(t, "SHIFT_LEFT") == 0 || strcmp(t, "SHIFT_RIGHT") == 0 ||
        strcmp(t, "NOT") == 0 || strcmp(t, "BIT_NOT") == 0 ||
        strcmp(t, "PLUS") == 0 || strcmp(t, "CALL") == 0 ||
        strcmp(t, "braces") == 0 || strcmp(t, "slice") == 0 ||
        strcmp(t, "IDENTIFIER") == 0 || strcmp(t, "DEC") == 0 ||
        strcmp(t, "HEX") == 0 || strcmp(t, "BIN") == 0 ||
        strcmp(t, "STR") == 0 || strcmp(t, "CHAR") == 0 ||
        strcmp(t, "TRUE") == 0 || strcmp(t, "FALSE") == 0) {
        Operation* op = exprToOp(node);
        if (op) {
            addOperationToBlock(current, op);
            if (!current->ast_node) current->ast_node = node;
        }
        return current;
    }

    /* ---- block { ... } / begin ... end ---- */
    if (strcmp(t, "block") == 0) {
        return buildBlock(ctx, node->left, current, after_block);
    }

    /* ---- IF ---- */
    if (strcmp(t, "if") == 0) {
        /*
         * node->left  = condition expr
         * node->right = then-statement  (может быть listStatement или else)
         * В парсере: if: IF expr THAN statement optionalElseStatement
         * В createNode("if", $2, $3, NULL) — $2=condition, $3=then
         * optionalElseStatement идёт в $$->right = $4  (внутри action)
         *
         * Смотрим на right: если там "else" — разбираем.
         */
        Operation* cond_op = exprToOp(node->left);
        addOperationToBlock(current, createOperation("IF_COND", cond_op, NULL, NULL, 0));

        BasicBlock* then_block = newBlock(ctx);
        BasicBlock* merge_block = newBlock(ctx);

        current->true_target = then_block;
        current->false_target = merge_block;

        /* then */
        BasicBlock* then_end = buildBlock(ctx, node->right, then_block, after_block);
        if (then_end) then_end->true_target = merge_block;

        /* else (если есть) — правый потомок then-узла — нет,
         * в парсере else сохраняется в $$->right после создания "if".
         * Проверяем: node->right может быть или statement, или listStatement.
         * else сохраняется отдельно — в parser.y: $$->right = $4 (optionalElse)
         * $4 имеет тип "else". Разберём отдельно.
         */
         /* На самом деле в parser.y:
          *   $$ = createNode("if", $2, $3, NULL);  // left=cond, right=then
          *   if ($4) $$->right = $4;               // right перезаписывается на else!
          * Значит если есть else, node->right = else-node, а then потерян?
          * Это баг парсера, но работаем с тем что есть.
          * Если node->right->type == "else", то then = NULL (потерян), else = node->right->left
          * Иначе then = node->right.
          */
          /* Переделаем: */
        then_end = then_block; /* сброс */
        if (node->right && node->right->type &&
            strcmp(node->right->type, "else") == 0) {
            /* then потерян — только else */
            BasicBlock* else_block = newBlock(ctx);
            current->false_target = else_block;
            /* then_block остаётся пустым, ведёт в merge */
            then_block->true_target = merge_block;

            BasicBlock* else_end = buildBlock(ctx, node->right->left,
                else_block, after_block);
            if (else_end) else_end->true_target = merge_block;
        }
        else {
            /* Нет else */
            BasicBlock* tend = buildBlock(ctx, node->right, then_block, after_block);
            if (tend) tend->true_target = merge_block;
        }

        return merge_block;
    }

    /* ---- loop: while / until ---- */
    if (strcmp(t, "loop") == 0) {
        /*
         * node->left  = condition
         * node->right = body (listStatement)
         * node->value = "while" | "until"
         */
        BasicBlock* cond_block = newBlock(ctx);
        BasicBlock* body_block = newBlock(ctx);
        BasicBlock* exit_block = newBlock(ctx);

        current->true_target = cond_block;

        Operation* cond_op = exprToOp(node->left);
        addOperationToBlock(cond_block,
            createOperation("LOOP_COND", cond_op, NULL,
                node->value ? node->value : "while", 0));

        /* while: вход в тело если условие истинно */
        cond_block->true_target = body_block;
        cond_block->false_target = exit_block;

        /* break внутри тела выходит в exit_block */
        if (ctx->break_depth < 64)
            ctx->break_targets[ctx->break_depth++] = exit_block;

        BasicBlock* body_end = buildBlock(ctx, node->right, body_block, exit_block);
        if (body_end) body_end->true_target = cond_block; /* обратная дуга */

        if (ctx->break_depth > 0) ctx->break_depth--;

        return exit_block;
    }

    /* ---- repeat: statement while/until expr; ---- */
    if (strcmp(t, "repeat") == 0) {
        /*
         * node->left  = body statement
         * node->right = condition
         * node->value = "while" | "until"
         */
        BasicBlock* body_block = newBlock(ctx);
        BasicBlock* cond_block = newBlock(ctx);
        BasicBlock* exit_block = newBlock(ctx);

        current->true_target = body_block;

        if (ctx->break_depth < 64)
            ctx->break_targets[ctx->break_depth++] = exit_block;

        BasicBlock* body_end = buildBlock(ctx, node->left, body_block, exit_block);
        if (body_end) body_end->true_target = cond_block;

        if (ctx->break_depth > 0) ctx->break_depth--;

        Operation* cond_op = exprToOp(node->right);
        addOperationToBlock(cond_block,
            createOperation("REPEAT_COND", cond_op, NULL,
                node->value ? node->value : "while", 0));
        cond_block->true_target = body_block; /* ещё раз */
        cond_block->false_target = exit_block;

        return exit_block;
    }

    /* ---- break ---- */
    if (strcmp(t, "break") == 0) {
        addOperationToBlock(current, createOperation("BREAK", NULL, NULL, NULL, 0));
        if (ctx->break_depth > 0) {
            current->true_target = ctx->break_targets[ctx->break_depth - 1];
        }
        else {
            addErrorToCollection(ctx->errors,
                "break outside of loop", ctx->filename, 0);
        }
        /* После break создаём «мёртвый» блок чтобы не прерывать обход */
        return newBlock(ctx);
    }

    /* ---- sourceItem: определение функции внутри блока ---- */
    if (strcmp(t, "sourceItem") == 0) {
        /* Пропускаем вложенные определения функций */
        return current;
    }

    /* Неизвестный узел — добавляем как Generic */
    {
        char label[256];
        snprintf(label, sizeof(label), "<%s>", t);
        addOperationToBlock(current,
            createOperation(label, exprToOp(node->left),
                exprToOp(node->right),
                node->value, 0));
    }
    return current;
}

/* ============================================================
 *  Построение CFG одной функции из sourceItem
 * ============================================================ */
static Function* buildFunctionCFG(Node* sourceItem, const char* filename,
    ErrorCollection* errors) {
    if (!sourceItem) return NULL;

    /* sourceItem: left = funcSignature, right = listStatement */
    Node* sig_node = sourceItem->left;
    Node* body_node = sourceItem->right;

    if (!sig_node) {
        addErrorToCollection(errors, "sourceItem without signature", filename, 0);
        return NULL;
    }

    /* Извлекаем имя функции: funcSignature хранит имя в value */
    const char* func_name = sig_node->value ? sig_node->value : "<unknown>";

    /* Сигнатура */
    FunctionSignature* sig = (FunctionSignature*)malloc(sizeof(FunctionSignature));
    sig->name = strdup(func_name);
    sig->args = NULL;
    sig->return_type = NULL;

    /* Тип возврата — sig_node->right (optionalTypeRef) */
    if (sig_node->right && sig_node->right->value)
        sig->return_type = strdup(sig_node->right->value);
    else if (sig_node->right && sig_node->right->type)
        sig->return_type = strdup(sig_node->right->type);

    /* Аргументы — sig_node->left (listArgDef) */
    Node* arg_list = sig_node->left;
    FunctionArg* last_arg = NULL;
    while (arg_list) {
        Node* arg_node = NULL;
        if (arg_list->type && strcmp(arg_list->type, "listArgDef") == 0) {
            arg_node = arg_list->left;
            arg_list = arg_list->right;
        }
        else if (arg_list->type && strcmp(arg_list->type, "argDef") == 0) {
            arg_node = arg_list;
            arg_list = NULL;
        }
        else {
            break;
        }
        if (!arg_node) break;

        FunctionArg* fa = (FunctionArg*)malloc(sizeof(FunctionArg));
        fa->name = (arg_node->left && arg_node->left->value)
            ? strdup(arg_node->left->value) : strdup("?");
        fa->type = (arg_node->right && arg_node->right->value)
            ? strdup(arg_node->right->value)
            : (arg_node->right && arg_node->right->type)
            ? strdup(arg_node->right->type) : strdup("?");
        fa->next = NULL;

        if (!sig->args) sig->args = fa;
        else            last_arg->next = fa;
        last_arg = fa;
    }

    /* CFG */
    CFG* cfg = createCFG();

    BuildCtx ctx;
    ctx.cfg = cfg;
    ctx.errors = errors;
    ctx.filename = filename;
    ctx.block_id_counter = 0;
    ctx.break_depth = 0;

    BasicBlock* entry = newBlock(&ctx);
    entry->is_entry = 1;
    cfg->entry_block = entry;

    BasicBlock* last = buildBlock(&ctx, body_node, entry, NULL);

    /* Всегда создаём отдельный EXIT-блок, чтобы ENTRY не совпадал с EXIT */
    BasicBlock* exit_block = newBlock(&ctx);
    exit_block->is_exit = 1;
    cfg->exit_block = exit_block;

    if (last && last != entry) {
        if (!last->true_target)
            last->true_target = exit_block;
    }
    else {
        if (!entry->true_target)
            entry->true_target = exit_block;
    }

    Function* func = createFunction(sig, cfg, filename);

    /* Освобождаем сигнатуру — она скопирована в createFunction? Нет, передаём владение */
    return func;
}

/* ============================================================
 *  Обход source: source -> source sourceItem | пусто
 * ============================================================ */
static void collectSourceItems(Node* node, Node** items, int* count, int max) {
    if (!node) return;
    if (node->type && strcmp(node->type, "source") == 0) {
        collectSourceItems(node->left, items, count, max);
        collectSourceItems(node->right, items, count, max);
    }
    else if (node->type && strcmp(node->type, "sourceItem") == 0) {
        if (*count < max) items[(*count)++] = node;
    }
}

/* ============================================================
 *  Главный интерфейс: buildCFGFromAST
 * ============================================================ */
AnalysisResult* buildCFGFromAST(FileCollection* file_collection) {
    FunctionCollection* funcs = createFunctionCollection();
    ErrorCollection* errors = createErrorCollection();

    if (!file_collection) {
        addErrorToCollection(errors, "Null file collection", NULL, 0);
        return createAnalysisResult(funcs, errors);
    }

    FileInfo* fi = file_collection->files;
    while (fi) {
        Node* root = fi->ast;
        if (!root) {
            addErrorToCollection(errors, "Null AST for file",
                fi->filename, 0);
            fi = fi->next;
            continue;
        }

        /* Собираем все sourceItem */
        Node* items[256];
        int   item_count = 0;
        collectSourceItems(root, items, &item_count, 256);

        for (int i = 0; i < item_count; i++) {
            Function* f = buildFunctionCFG(items[i], fi->filename, errors);
            if (f) addFunctionToCollection(funcs, f);
        }

        fi = fi->next;
    }

    return createAnalysisResult(funcs, errors);
}