/*
 * cfg.c -- Реализация построения графа потока управления (CFG) из дерева разбора (AST)
 */

#include "cfg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Создание одного узла операции для CFG,
 * из которых дальше собирается дерево выражения
 */
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

/* Освобождение операции вместе с поддеревом и цепочкой next */
void freeOperation(Operation* op) {
    if (!op) return;
    free(op->op_type);
    free(op->value);
    freeOperation(op->left);
    freeOperation(op->right);
    freeOperation(op->next);
    free(op);
}

/* Создание basic block */
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

/* Добавление операции в конец списка блока */
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

/* Освобождение блока и его операций */
void freeBasicBlock(BasicBlock* block) {
    if (!block) return;
    freeOperation(block->operations);
    free(block);
}

// CFG

/* Создание пустого CFG для функции или метода */
CFG* createCFG(void) {
    CFG* cfg = (CFG*)malloc(sizeof(CFG));
    if (!cfg) return NULL;
    cfg->entry_block = NULL;
    cfg->exit_block = NULL;
    cfg->blocks = NULL;
    cfg->block_count = 0;
    return cfg;
}

/* Добавление блока в общий список CFG */
void addBlockToCFG(CFG* cfg, BasicBlock* block) {
    if (!cfg || !block) return;
    block->next = cfg->blocks;
    cfg->blocks = block;
    cfg->block_count++;
}

/* Полное освобождение CFG */
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

/* Создание пустой таблицы типов */
TypeCollection* createTypeCollection(void) {
    TypeCollection* collection = (TypeCollection*)malloc(sizeof(TypeCollection));
    if (!collection) return NULL;
    collection->types = NULL;
    collection->type_count = 0;
    return collection;
}

/* Добавление пользовательского типа в общую таблицу */
void addTypeToCollection(TypeCollection* collection, UserType* type_info) {
    if (!collection || !type_info) return;
    type_info->next = collection->types;
    collection->types = type_info;
    collection->type_count++;
}

/* Поиск типа по имени */
UserType* findUserType(TypeCollection* collection, const char* type_name) {
    UserType* current;

    if (!collection || !type_name) return NULL;

    /* поиск типа по имени */
    current = collection->types;
    while (current) {
        if (current->name && strcmp(current->name, type_name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/* Поиск поля в типе и его базовых типах */
UserTypeField* findUserTypeField(TypeCollection* collection, const char* type_name,
    const char* field_name) {
    UserType* type_info;

    if (!collection || !type_name || !field_name) return NULL;

    type_info = findUserType(collection, type_name);
    while (type_info) {
        UserTypeField* field = type_info->fields;
        while (field) {
            if (field->name && strcmp(field->name, field_name) == 0) {
                return field;
            }
            field = field->next;
        }

        if (!type_info->base_type_name) {
            break;
        }
        type_info = findUserType(collection, type_info->base_type_name);
    }

    return NULL;
}

/* Поиск метода в типе и его базовых типах */
UserTypeMethod* findUserTypeMethod(TypeCollection* collection, const char* type_name,
    const char* method_name) {
    UserType* type_info;

    if (!collection || !type_name || !method_name) return NULL;

    type_info = findUserType(collection, type_name);
    while (type_info) {
        UserTypeMethod* method = type_info->methods;
        while (method) {
            if (method->name && strcmp(method->name, method_name) == 0) {
                return method;
            }
            method = method->next;
        }

        if (!type_info->base_type_name) {
            break;
        }
        type_info = findUserType(collection, type_info->base_type_name);
    }

    return NULL;
}

/* Проверка на встроенный тип */
int isBuiltinTypeName(const char* type_name) {
    if (!type_name) return 0;
    return strcmp(type_name, "byte") == 0 ||
        strcmp(type_name, "int") == 0 ||
        strcmp(type_name, "uint") == 0 ||
        strcmp(type_name, "long") == 0 ||
        strcmp(type_name, "ulong") == 0 ||
        strcmp(type_name, "bool") == 0 ||
        strcmp(type_name, "char") == 0 ||
        strcmp(type_name, "string") == 0;
}

/* Определение размера типа в памяти
 * Для user type используется уже посчитанный size_bytes
 */
int getTypeStorageSize(TypeCollection* collection, const char* type_name) {
    UserType* user_type;

    if (!type_name || !*type_name) {
        return 4;
    }

    if (strncmp(type_name, "array(", 6) == 0) {
        const char* comma = strrchr(type_name, ',');
        if (comma) {
            int element_count = atoi(comma + 1);
            if (element_count > 0) {
                return element_count * 4;
            }
        }
        return 4;
    }

    if (isBuiltinTypeName(type_name)) {
        return 4;
    }

    user_type = findUserType(collection, type_name);
    if (user_type && user_type->size_bytes > 0) {
        return user_type->size_bytes;
    }

    return 4;
}

/* Освобождение всей таблицы типов */
void freeTypeCollection(TypeCollection* collection) {
    UserType* current;

    if (!collection) return;

    current = collection->types;
    while (current) {
        UserType* next_type = current->next;
        UserTypeField* field = current->fields;
        UserTypeMethod* method = current->methods;

        while (field) {
            UserTypeField* next_field = field->next;
            free(field->name);
            free(field->type_name);
            free(field->owner_type_name);
            free(field);
            field = next_field;
        }

        while (method) {
            UserTypeMethod* next_method = method->next;
            free(method->name);
            free(method->full_name);
            free(method->return_type);
            free(method);
            method = next_method;
        }

        free(current->name);
        free(current->base_type_name);
        free(current);
        current = next_type;
    }

    free(collection);
}

/* Создание описания одной функции
 * Здесь же хранится ссылка на таблицу типов
 */
Function* createFunction(FunctionSignature* signature, CFG* cfg,
    const char* source_file, const char* owner_type_name,
    int is_method, TypeCollection* types) {
    Function* f = (Function*)malloc(sizeof(Function));
    if (!f) return NULL;
    f->signature = signature;
    f->cfg = cfg;
    f->source_file = source_file ? strdup(source_file) : NULL;
    f->owner_type_name = owner_type_name ? strdup(owner_type_name) : NULL;
    f->is_method = is_method;
    f->types = types;
    f->next = NULL;
    return f;
}

/* Освобождение одной функции */
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
    free(func->owner_type_name);
    free(func);
}

/* Создание списка входных файлов */
FileCollection* createFileCollection(void) {
    FileCollection* fc = (FileCollection*)malloc(sizeof(FileCollection));
    if (!fc) return NULL;
    fc->files = NULL;
    fc->file_count = 0;
    return fc;
}

/* Добавление файла и его AST в общий список */
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

/* Создание пустой коллекции функций */
FunctionCollection* createFunctionCollection(void) {
    FunctionCollection* fc = (FunctionCollection*)malloc(sizeof(FunctionCollection));
    if (!fc) return NULL;
    fc->functions = NULL;
    fc->function_count = 0;
    return fc;
}

/* Добавление функции в общий список результатов */
void addFunctionToCollection(FunctionCollection* collection, Function* func) {
    if (!collection || !func) return;
    func->next = collection->functions;
    collection->functions = func;
    collection->function_count++;
}

/* Создание пустой коллекции ошибок */
ErrorCollection* createErrorCollection(void) {
    ErrorCollection* ec = (ErrorCollection*)malloc(sizeof(ErrorCollection));
    if (!ec) return NULL;
    ec->errors = NULL;
    ec->error_count = 0;
    return ec;
}

/* Добавление ошибки в список */
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

/* Сборка итога анализа в один объект
 * Функции, ошибки и таблица типов
 */
AnalysisResult* createAnalysisResult(FunctionCollection* functions,
    ErrorCollection* errors, TypeCollection* types) {
    AnalysisResult* ar = (AnalysisResult*)malloc(sizeof(AnalysisResult));
    if (!ar) return NULL;
    ar->functions = functions;
    ar->errors = errors;
    ar->types = types;
    return ar;
}

/* Освобождение всего результата анализа */
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
    freeTypeCollection(result->types);
    free(result);
}

typedef struct BuildCtx {
    CFG* cfg;
    ErrorCollection* errors;
    const char* filename;
    int              block_id_counter;
    /* стек блоков-продолжений для break */
    BasicBlock* break_targets[64];
    int              break_depth;
} BuildCtx;

/* Создание нового блока и добавление его в CFG */
static BasicBlock* newBlock(BuildCtx* ctx) {
    BasicBlock* bb = createBasicBlock(ctx->block_id_counter++);
    addBlockToCFG(ctx->cfg, bb);
    return bb;
}

/* Преобразование AST-узла типа в строку
 * Используется для полей, аргументов и return type
 */
static char* buildTypeNameFromNode(Node* node) {
    char buffer[256];
    char* element_name;

    /* делаю строку с именем типа */
    if (!node) return strdup("?");

    if (node->value && *node->value &&
        (strcmp(node->type ? node->type : "", "TYPEDEF") == 0 ||
            strcmp(node->type ? node->type : "", "IDENTIFIER") == 0)) {
        return strdup(node->value);
    }

    if (node->type && strcmp(node->type, "array") == 0) {
        element_name = buildTypeNameFromNode(node->left);
        snprintf(buffer, sizeof(buffer), "array(%s,%s)",
            element_name ? element_name : "?",
            node->value ? node->value : "1");
        free(element_name);
        return strdup(buffer);
    }

    if (node->value && *node->value) {
        return strdup(node->value);
    }

    if (node->type) {
        return strdup(node->type);
    }

    return strdup("?");
}

/* Преобразование AST-выражения в дерево операций для CFG
 * Такой вид уже подходит для codegen
 */
static Operation* exprToOp(Node* node) {
    if (!node) return NULL;

    const char* t = node->type ? node->type : "";
    const char* v = node->value ? node->value : "";

    /* Листья: литералы, идентификаторы */
    /* эти штуки нужны дальше в codegen */
    if (strcmp(t, "IDENTIFIER") == 0 ||
        strcmp(t, "DEC") == 0 ||
        strcmp(t, "HEX") == 0 ||
        strcmp(t, "BIN") == 0 ||
        strcmp(t, "STR") == 0 ||
        strcmp(t, "CHAR") == 0 ||
        strcmp(t, "TRUE") == 0 ||
        strcmp(t, "FALSE") == 0 ||
        strcmp(t, "memberAccess") == 0 ||
        strcmp(t, "METHOD_CALL") == 0) {
        if (strcmp(t, "memberAccess") == 0 || strcmp(t, "METHOD_CALL") == 0) {
            return createOperation((char*)t, exprToOp(node->left),
                exprToOp(node->right), v, 0);
        }
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

/* Объявление внутренних функций для разбора тела и сборки CFG */
static BasicBlock* buildBlock(BuildCtx* ctx, Node* node,
    BasicBlock* current, BasicBlock* after_block);

/* Обход списка statement/sourceItem и добавление в CFG */
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

/* Главная функция разбора тела в CFG
 * Отдельно обрабатываются var, assignment, if, loop, break и block
 */
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
        strcmp(t, "PLUS") == 0 || strcmp(t, "MINUS") == 0 ||
        strcmp(t, "STAR") == 0 || strcmp(t, "SLASH") == 0 ||
        strcmp(t, "PERCENT") == 0 || strcmp(t, "EQUALITY") == 0 ||
        strcmp(t, "NOTEQUAL") == 0 || strcmp(t, "LESSTHAN") == 0 ||
        strcmp(t, "GREATERTHAN") == 0 || strcmp(t, "LESSTHANEQ") == 0 ||
        strcmp(t, "GREATERTHANEQ") == 0 || strcmp(t, "AND") == 0 ||
        strcmp(t, "OR") == 0 || strcmp(t, "BIT_AND") == 0 ||
        strcmp(t, "BIT_OR") == 0 || strcmp(t, "BIT_XOR") == 0 ||
        strcmp(t, "SHIFT_LEFT") == 0 || strcmp(t, "SHIFT_RIGHT") == 0 ||
        strcmp(t, "NOT") == 0 || strcmp(t, "BIT_NOT") == 0 ||
        strcmp(t, "PLUS") == 0 || strcmp(t, "CALL") == 0 ||
        strcmp(t, "METHOD_CALL") == 0 || strcmp(t, "memberAccess") == 0 ||
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

    /* ---- else-chain ---- */
    if (strcmp(t, "else") == 0) {
        current = buildBlock(ctx, node->left, current, after_block);
        if (node->right) {
            current = buildBlock(ctx, node->right, current, after_block);
        }
        return current;
    }

    /* ---- IF ---- */
    if (strcmp(t, "if") == 0) {
        /* Структура: if->left = condition, if->right = ifStatements(left=then, right=else|NULL) */
        Operation* cond_op = exprToOp(node->left);
        addOperationToBlock(current, createOperation("IF_COND", cond_op, NULL, NULL, 0));

        BasicBlock* merge_block = newBlock(ctx);

        Node* if_body = node->right;
        Node* then_node = NULL;
        Node* else_node = NULL;

        /* parser.y stores ifStatements(left = THAN token, right = then statement).
           So the executable body lives in right, not left. */
        if (if_body) {
            then_node = if_body->right;
        }

        /* Создаём then-блок */
        BasicBlock* then_block = newBlock(ctx);
        current->true_target = then_block;

        BasicBlock* then_end = buildBlock(ctx, then_node, then_block, after_block);
        if (then_end && !then_end->true_target && !then_end->false_target)
            then_end->true_target = merge_block;

        if (else_node) {
            BasicBlock* else_block = newBlock(ctx);
            current->false_target = else_block;
            BasicBlock* else_end = buildBlock(ctx, else_node, else_block, after_block);
            if (else_end && !else_end->true_target && !else_end->false_target) {
                else_end->true_target = merge_block;
            }
        }
        else {
            /* Нет else — false ведёт сразу к merge */
            current->false_target = merge_block;
        }

        return merge_block;
    }

    /* ---- loop: pre-check (while/until listStatement end) or post-check (repeat) ---- */
    if (strcmp(t, "loop") == 0) {
        int is_precheck = (node->right && node->right->type &&
            strcmp(node->right->type, "listStatement") == 0);

        BasicBlock* body_block = newBlock(ctx);
        BasicBlock* cond_block = newBlock(ctx);
        BasicBlock* exit_block = newBlock(ctx);

        if (ctx->break_depth < 64) {
            ctx->break_targets[ctx->break_depth++] = exit_block;
        }

        if (is_precheck) {
            if (!current->true_target) {
                current->true_target = cond_block;
            }

            addOperationToBlock(cond_block,
                createOperation("LOOP_COND", exprToOp(node->left), NULL, "while", 0));
            cond_block->true_target = body_block;
            cond_block->false_target = exit_block;

            BasicBlock* body_end = buildBlock(ctx, node->right, body_block, exit_block);
            if (body_end && !body_end->true_target && !body_end->false_target) {
                body_end->true_target = cond_block;
            }
        } else {
            if (!current->true_target) {
                current->true_target = body_block;
            }

            BasicBlock* body_end = buildBlock(ctx, node->right, body_block, exit_block);
            if (body_end && !body_end->true_target && !body_end->false_target) {
                body_end->true_target = cond_block;
            }

            addOperationToBlock(cond_block,
                createOperation("REPEAT_COND", exprToOp(node->left), NULL, "while", 0));
            cond_block->true_target = body_block;
            cond_block->false_target = exit_block;
        }

        if (ctx->break_depth > 0) {
            ctx->break_depth--;
        }

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
        /*
         * Код после break в той же ветке недостижим.
         * Возвращаем изолированный dead-блок: buildList продолжит обход AST,
         * но ни одно ребро к нему не ведёт — в граф он не попадёт.
         */
        BasicBlock* dead = newBlock(ctx);
        return dead;
    }

    if (strcmp(t, "return") == 0) {
        addOperationToBlock(current,
            createOperation("RETURN", exprToOp(node->left), NULL, NULL, 0));
        BasicBlock* dead = newBlock(ctx);
        return dead;
    }

    /* ---- sourceItem: определение функции внутри блока ---- */
    if (strcmp(t, "sourceItem") == 0) {
        /* Пропускаем вложенные определения функций */
        return current;
    }

    if (strcmp(t, "typeDecl") == 0 || strcmp(t, "methodDecl") == 0) {
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

/* Добавление поля в тип без расчёта offset */
static void addTypeField(UserType* type_info, const char* field_name,
    const char* field_type_name) {
    UserTypeField* field;

    if (!type_info || !field_name) return;

    /* добавляю поле в список */
    field = (UserTypeField*)malloc(sizeof(UserTypeField));
    if (!field) return;

    field->name = strdup(field_name);
    field->type_name = field_type_name ? strdup(field_type_name) : strdup("?");
    field->offset = 0;
    field->owner_type_name = type_info->name ? strdup(type_info->name) : NULL;
    field->next = NULL;

    if (!type_info->fields) {
        type_info->fields = field;
    }
    else {
        UserTypeField* tail = type_info->fields;
        while (tail->next) tail = tail->next;
        tail->next = field;
    }
}

/* Добавление метода в тип
 * Сохраняется полное имя для asm и AST узел для будущей сборки CFG
 */
static void addTypeMethod(UserType* type_info, const char* method_name,
    const char* full_name, const char* return_type, Node* ast_node) {
    UserTypeMethod* method;

    if (!type_info || !method_name || !full_name) return;

    /* храню и короткое, и полное имя */
    method = (UserTypeMethod*)malloc(sizeof(UserTypeMethod));
    if (!method) return;

    method->name = strdup(method_name);
    method->full_name = strdup(full_name);
    method->return_type = return_type ? strdup(return_type) : strdup("?");
    method->ast_node = ast_node;
    method->next = NULL;

    if (!type_info->methods) {
        type_info->methods = method;
    }
    else {
        UserTypeMethod* tail = type_info->methods;
        while (tail->next) tail = tail->next;
        tail->next = method;
    }
}

/* Сбор всех верхнеуровневых def из AST файла */
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

/* Сбор всех typeDecl из AST файла */
static void collectTypeDecls(Node* node, Node** items, int* count, int max) {
    if (!node) return;
    if (node->type && strcmp(node->type, "source") == 0) {
        collectTypeDecls(node->left, items, count, max);
        collectTypeDecls(node->right, items, count, max);
    }
    else if (node->type && strcmp(node->type, "typeDecl") == 0) {
        if (*count < max) items[(*count)++] = node;
    }
}

/* Сбор полей и методов из одного typeDecl */
static void collectTypeMembers(UserType* type_info, Node* node) {
    if (!type_info || !node) return;

    /* список членов идёт цепочкой */
    if (node->type && strcmp(node->type, "listTypeMember") == 0) {
        collectTypeMembers(type_info, node->left);
        collectTypeMembers(type_info, node->right);
        return;
    }

    if (node->type && strcmp(node->type, "fieldDecl") == 0) {
        char* field_type_name = buildTypeNameFromNode(node->left);
        addTypeField(type_info, node->value ? node->value : "?",
            field_type_name);
        free(field_type_name);
        return;
    }

    if (node->type && strcmp(node->type, "methodDecl") == 0 && node->left) {
        char full_name[256];
        char* return_type_name = buildTypeNameFromNode(node->left->right);
        /* такое имя потом удобнее в asm */
        snprintf(full_name, sizeof(full_name), "%s__%s",
            type_info->name ? type_info->name : "type",
            node->left->value ? node->left->value : "method");
        addTypeMethod(type_info,
            node->left->value ? node->left->value : "method",
            full_name, return_type_name, node);
        free(return_type_name);
    }
}

/* Преобразование AST объявления type в структуру UserType */
static UserType* buildTypeFromDecl(Node* type_decl, ErrorCollection* errors,
    const char* filename) {
    UserType* type_info;
    Node* header;

    if (!type_decl || !type_decl->left) return NULL;

    /* из AST делаю UserType */
    header = type_decl->left;
    type_info = (UserType*)malloc(sizeof(UserType));
    if (!type_info) return NULL;

    type_info->name = header->value ? strdup(header->value) : strdup("?");
    type_info->base_type_name = (header->left && header->left->value)
        ? strdup(header->left->value) : NULL;
    type_info->fields = NULL;
    type_info->methods = NULL;
    type_info->size_bytes = 0;
    type_info->resolved = 0;
    type_info->resolving = 0;
    type_info->next = NULL;

    if (type_info->base_type_name &&
        strcmp(type_info->base_type_name, type_info->name) == 0) {
        addErrorToCollection(errors, "Type cannot inherit from itself",
            filename, 0);
    }

    collectTypeMembers(type_info, type_decl->right);
    return type_info;
}

/* Расчёт layout типов
 * Вычисление размера типа и смещений его полей
 * При наличии базового типа свои поля идут после него
 * Для поля user type сначала считается размер вложенного типа
 */
static int resolveUserTypeLayout(TypeCollection* types, UserType* type_info,
    ErrorCollection* errors, const char* filename) {
    int offset = 0;
    UserTypeField* field;

    /* считаю размер типа и offsets полей */
    if (!type_info) return 0;
    if (type_info->resolved) return 1;

    if (type_info->resolving) {
        addErrorToCollection(errors, "Cyclic type inheritance detected",
            filename, 0);
        return 0;
    }

    type_info->resolving = 1;

    if (type_info->base_type_name) {
        UserType* base_type = findUserType(types, type_info->base_type_name);
        if (!base_type) {
            addErrorToCollection(errors, "Unknown base type",
                filename, 0);
        }
        else {
            /* сначала база */
            resolveUserTypeLayout(types, base_type, errors, filename);
            offset = base_type->size_bytes;
        }
    }

    field = type_info->fields;
    while (field) {
        int field_size = 4;

        if (type_info->base_type_name &&
            findUserTypeField(types, type_info->base_type_name, field->name)) {
            addErrorToCollection(errors, "Field hides inherited field",
                filename, 0);
        }

        /* если поле тоже user type, сначала считаю его */
        if (field->type_name &&
            !isBuiltinTypeName(field->type_name) &&
            strncmp(field->type_name, "array(", 6) != 0) {
            UserType* field_type = findUserType(types, field->type_name);
            if (!field_type) {
                addErrorToCollection(errors, "Unknown field type",
                    filename, 0);
            }
            else {
                resolveUserTypeLayout(types, field_type, errors, filename);
            }
        }

        /* тут уже можно сдвигать offset */
        field_size = getTypeStorageSize(types, field->type_name);
        field->offset = offset;
        offset += field_size;
        field = field->next;
    }

    type_info->size_bytes = offset > 0 ? offset : 4;
    type_info->resolved = 1;
    type_info->resolving = 0;
    return 1;
}

/* Запуск расчёта layout для всех типов */
static void resolveAllUserTypes(TypeCollection* types, ErrorCollection* errors,
    const char* filename) {
    UserType* current;

    if (!types) return;

    current = types->types;
    while (current) {
        resolveUserTypeLayout(types, current, errors, filename);
        current = current->next;
    }
}

/* Преобразование AST сигнатуры в FunctionSignature */
static FunctionSignature* buildSignatureFromNode(Node* sig_node,
    const char* name_override) {
    FunctionSignature* sig;
    Node* arg_list;
    FunctionArg* last_arg = NULL;

    if (!sig_node) return NULL;

    sig = (FunctionSignature*)malloc(sizeof(FunctionSignature));
    if (!sig) return NULL;

    sig->name = strdup(name_override ? name_override :
        (sig_node->value ? sig_node->value : "<unknown>"));
    sig->args = NULL;
    sig->return_type = buildTypeNameFromNode(sig_node->right);

    arg_list = sig_node->left;
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

        if (arg_node) {
            FunctionArg* fa = (FunctionArg*)malloc(sizeof(FunctionArg));
            if (!fa) break;
            fa->name = (arg_node->left && arg_node->left->value)
                ? strdup(arg_node->left->value) : strdup("?");
            fa->type = buildTypeNameFromNode(arg_node->right);
            fa->next = NULL;

            if (!sig->args) sig->args = fa;
            else            last_arg->next = fa;
            last_arg = fa;
        }
    }

    return sig;
}

/* Сборка CFG одной функции из def или methodDecl
 * Используется и для обычных функций, и для методов типа
 */
static Function* buildFunctionCFG(Node* source_item, const char* filename,
    ErrorCollection* errors, TypeCollection* types,
    const char* owner_type_name, const char* name_override, int is_method) {
    Node* sig_node;
    Node* body_node;
    FunctionSignature* sig;
    CFG* cfg;
    BuildCtx ctx;
    BasicBlock* entry;
    BasicBlock* last;
    BasicBlock* exit_block;

    /* и для def, и для методов */
    if (!source_item) return NULL;

    sig_node = source_item->left;
    body_node = source_item->right;

    if (!sig_node) {
        addErrorToCollection(errors, "Function-like node without signature",
            filename, 0);
        return NULL;
    }

    sig = buildSignatureFromNode(sig_node, name_override);
    if (!sig) return NULL;

    cfg = createCFG();
    if (!cfg) return NULL;

    ctx.cfg = cfg;
    ctx.errors = errors;
    ctx.filename = filename;
    ctx.block_id_counter = 0;
    ctx.break_depth = 0;

    entry = newBlock(&ctx);
    entry->is_entry = 1;
    cfg->entry_block = entry;

    last = buildBlock(&ctx, body_node, entry, NULL);

    exit_block = newBlock(&ctx);
    exit_block->is_exit = 1;
    cfg->exit_block = exit_block;

    if (last && !last->true_target && !last->is_exit)
        last->true_target = exit_block;

    if (!entry->true_target)
        entry->true_target = exit_block;

    return createFunction(sig, cfg, filename, owner_type_name, is_method, types);
}

/* Общий вход в модуль CFG
 * Сначала собираются все типы
 * Потом считается их layout
 * Потом отдельно строится CFG обычных функций и методов
 */
AnalysisResult* buildCFGFromAST(FileCollection* file_collection) {
    FunctionCollection* funcs = createFunctionCollection();
    ErrorCollection* errors = createErrorCollection();
    TypeCollection* types = createTypeCollection();

    if (!file_collection) {
        addErrorToCollection(errors, "Null file collection", NULL, 0);
        return createAnalysisResult(funcs, errors, types);
    }

    /* сначала собираю все type */
    {
        FileInfo* fi = file_collection->files;
        while (fi) {
            Node* root = fi->ast;
            if (root) {
                Node* type_items[256];
                int type_count = 0;
                collectTypeDecls(root, type_items, &type_count, 256);
                for (int i = 0; i < type_count; i++) {
                    UserType* type_info = buildTypeFromDecl(type_items[i], errors,
                        fi->filename);
                    if (!type_info) continue;
                    if (findUserType(types, type_info->name)) {
                        addErrorToCollection(errors, "Duplicate type declaration",
                            fi->filename, 0);
                    }
                    else {
                        addTypeToCollection(types, type_info);
                    }
                }
            }
            fi = fi->next;
        }
    }

    resolveAllUserTypes(types, errors,
        file_collection->files ? file_collection->files->filename : NULL);

    {
        FileInfo* fi = file_collection->files;
        while (fi) {
            Node* root = fi->ast;
            if (!root) {
                addErrorToCollection(errors, "Null AST for file",
                    fi->filename, 0);
                fi = fi->next;
                continue;
            }

            /* обычные функции */
            {
                Node* items[256];
                int item_count = 0;
                collectSourceItems(root, items, &item_count, 256);

                for (int i = 0; i < item_count; i++) {
                    Function* f = buildFunctionCFG(items[i], fi->filename, errors,
                        types, NULL, NULL, 0);
                    if (f) addFunctionToCollection(funcs, f);
                }
            }

            /* методы типа как отдельные функции */
            {
                Node* type_items[256];
                int type_count = 0;
                collectTypeDecls(root, type_items, &type_count, 256);

                for (int i = 0; i < type_count; i++) {
                    Node* header = type_items[i] ? type_items[i]->left : NULL;
                    UserType* type_info = findUserType(types,
                        header ? header->value : NULL);
                    UserTypeMethod* method = type_info ? type_info->methods : NULL;

                    while (method) {
                        Function* f = buildFunctionCFG(method->ast_node, fi->filename,
                            errors, types, type_info->name, method->full_name, 1);
                        if (f) addFunctionToCollection(funcs, f);
                        method = method->next;
                    }
                }
            }

            fi = fi->next;
        }
    }

    return createAnalysisResult(funcs, errors, types);
}
