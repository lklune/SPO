/*
 * export_cfg.c
 */
#include "export_cfg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void esc(FILE* f, const char* s) {
    if (!s) return;
    for (; *s; s++) {
        if (*s == '"')  fprintf(f, "\\\"");
        else if (*s == '\\') fprintf(f, "\\\\");
        else if (*s == '\n') fprintf(f, "\\n");
        else               fputc(*s, f);
    }
}

static void opStr(Operation* op, char* buf, int sz) {
    if (!op) { buf[0] = '\0'; return; }
    const char* t = op->op_type ? op->op_type : "";
    const char* v = op->value ? op->value : "";
    if (strcmp(t, "IDENTIFIER") == 0 || strcmp(t, "DEC") == 0 || strcmp(t, "HEX") == 0 ||
        strcmp(t, "BIN") == 0 || strcmp(t, "STR") == 0 || strcmp(t, "CHAR") == 0 ||
        strcmp(t, "TRUE") == 0 || strcmp(t, "FALSE") == 0) {
        snprintf(buf, sz, "%s", *v ? v : t); return;
    }
    char l[256] = "", r[256] = "";
    if (op->left)  opStr(op->left, l, sizeof(l));
    if (op->right) opStr(op->right, r, sizeof(r));
    if (strcmp(t, "assignment") == 0)       snprintf(buf, sz, "%s = %s", l, r);
    else if (strcmp(t, "PLUS") == 0)             snprintf(buf, sz, "%s + %s", l, r);
    else if (strcmp(t, "MINUS") == 0 && op->right) snprintf(buf, sz, "%s - %s", l, r);
    else if (strcmp(t, "MINUS") == 0)            snprintf(buf, sz, "-%s", l);
    else if (strcmp(t, "STAR") == 0)             snprintf(buf, sz, "%s * %s", l, r);
    else if (strcmp(t, "SLASH") == 0)            snprintf(buf, sz, "%s / %s", l, r);
    else if (strcmp(t, "PERCENT") == 0)          snprintf(buf, sz, "%s %% %s", l, r);
    else if (strcmp(t, "EQUALITY") == 0)         snprintf(buf, sz, "%s == %s", l, r);
    else if (strcmp(t, "NOTEQUAL") == 0)         snprintf(buf, sz, "%s != %s", l, r);
    else if (strcmp(t, "LESSTHAN") == 0)         snprintf(buf, sz, "%s < %s", l, r);
    else if (strcmp(t, "GREATERTHAN") == 0)      snprintf(buf, sz, "%s > %s", l, r);
    else if (strcmp(t, "LESSTHANEQ") == 0)       snprintf(buf, sz, "%s <= %s", l, r);
    else if (strcmp(t, "GREATERTHANEQ") == 0)    snprintf(buf, sz, "%s >= %s", l, r);
    else if (strcmp(t, "AND") == 0)              snprintf(buf, sz, "%s && %s", l, r);
    else if (strcmp(t, "OR") == 0)               snprintf(buf, sz, "%s || %s", l, r);
    else if (strcmp(t, "NOT") == 0)              snprintf(buf, sz, "!%s", l);
    else if (strcmp(t, "BIT_AND") == 0)          snprintf(buf, sz, "%s & %s", l, r);
    else if (strcmp(t, "BIT_OR") == 0)           snprintf(buf, sz, "%s | %s", l, r);
    else if (strcmp(t, "BIT_XOR") == 0)          snprintf(buf, sz, "%s ^ %s", l, r);
    else if (strcmp(t, "BIT_NOT") == 0)          snprintf(buf, sz, "~%s", l);
    else if (strcmp(t, "SHIFT_LEFT") == 0)       snprintf(buf, sz, "%s << %s", l, r);
    else if (strcmp(t, "SHIFT_RIGHT") == 0)      snprintf(buf, sz, "%s >> %s", l, r);
    else if (strcmp(t, "CALL") == 0)             snprintf(buf, sz, "%s(...)", l);
    else if (strcmp(t, "IF_COND") == 0)          snprintf(buf, sz, "if (%s)", l);
    else if (strcmp(t, "LOOP_COND") == 0)        snprintf(buf, sz, "%s (%s)", v, l);
    else if (strcmp(t, "REPEAT_COND") == 0)      snprintf(buf, sz, "do..%s (%s)", v, l);
    else if (strcmp(t, "VAR_DECL") == 0)         snprintf(buf, sz, "var %s: %s", r, l);
    else if (strcmp(t, "BREAK") == 0)            snprintf(buf, sz, "break");
    else if (*v) snprintf(buf, sz, "%s", v);
    else         buf[0] = '\0';
}

static void bbLabel(BasicBlock* bb, char* buf, int sz) {
    buf[0] = '\0';
    Operation* op = bb->operations;
    while (op) {
        char tmp[512] = "";
        opStr(op, tmp, sizeof(tmp));
        if (tmp[0] != '\0') {
            if (buf[0] != '\0') strncat(buf, "\\n", sz - strlen(buf) - 1);
            strncat(buf, tmp, sz - strlen(buf) - 1);
        }
        op = op->next;
    }
}

/* --- AST кластер --- */
static int ast_cnt = 0;

static int drawAstNode(FILE* f, Node* node) {
    if (!node) return -1;
    int id = ast_cnt++;
    const char* lbl = (node->value && node->value[0]) ? node->value
        : (node->type ? node->type : "?");
    fprintf(f, "    ast%d [label=\"", id);
    esc(f, lbl);
    fprintf(f, "\", shape=none, fontsize=9, fontcolor=blue4];\n");
    if (node->left) {
        int c = drawAstNode(f, node->left);
        if (c >= 0) fprintf(f, "    ast%d -> ast%d [color=gray80,arrowhead=none,penwidth=0.5];\n", id, c);
    }
    if (node->right) {
        int c = drawAstNode(f, node->right);
        if (c >= 0) fprintf(f, "    ast%d -> ast%d [color=gray80,arrowhead=none,penwidth=0.5];\n", id, c);
    }
    return id;
}

static void drawAstCluster(FILE* f, int bb_id, Node* ast) {
    if (!ast) return;
    fprintf(f, "  subgraph cluster_ast_%d {\n", bb_id);
    fprintf(f, "    label=\"AST for BB%d\";\n", bb_id);
    fprintf(f, "    style=dashed; color=gray70; fontcolor=gray50; fontsize=10;\n");
    drawAstNode(f, ast);
    fprintf(f, "  }\n");
}

/*
 * Пометить все блоки, достижимые из entry, с помощью обхода в глубину.
 * Используется для исключения «мёртвых» блоков из экспорта. // правда так и не работает как задумывалось
 */
#define MAX_BLOCKS 4096

static void markReachable(BasicBlock* bb, int* visited, int max_id) {
    if (!bb || bb->id < 0 || bb->id >= max_id) return;
    if (visited[bb->id]) return;
    visited[bb->id] = 1;
    markReachable(bb->true_target, visited, max_id);
    markReachable(bb->false_target, visited, max_id);
}

/*
 * Возвращает 1, если CFG тривиален: только ENTRY и EXIT,
 * соединённые прямым ребром (нет никакого реального кода).
 * Такие CFG не нужно экспортировать в .dot.
 */
static int isTrivialCFG(CFG* cfg) {
    if (!cfg || !cfg->entry_block) return 0;
    BasicBlock* entry = cfg->entry_block;
    /* У entry должен быть ровно один переход (true_target) */
    if (!entry->true_target || entry->false_target) return 0;
    /* И этот переход должен вести прямо в exit */
    return entry->true_target->is_exit ? 1 : 0;
}

void exportCFGToDot(Function* func, const char* filepath) {
    if (!func || !func->cfg) return;

    /* Проверяем, что CFG не совсем пустой */
    if (!func->cfg->entry_block) return;

    /* Пропускаем тривиальные CFG: ENTRY → EXIT без кода */
    if (isTrivialCFG(func->cfg)) return;

    FILE* f = fopen(filepath, "w");
    if (!f) { fprintf(stderr, "Cannot open: %s\n", filepath); return; }

    /* Вычисляем максимальный id блока для таблицы достижимости */
    int max_id = 0;
    {
        BasicBlock* bb = func->cfg->blocks;
        while (bb) { if (bb->id >= max_id) max_id = bb->id + 1; bb = bb->next; }
    }

    /* Помечаем только достижимые блоки (из entry) */
    int* reachable = (int*)calloc(max_id, sizeof(int));
    if (func->cfg->entry_block)
        markReachable(func->cfg->entry_block, reachable, max_id);

    ast_cnt = 0;
    const char* fname = (func->signature && func->signature->name)
        ? func->signature->name : "unknown";

    fprintf(f, "digraph CFG {\n");
    fprintf(f, "  node [fontname=\"Courier\", fontsize=11];\n");
    fprintf(f, "  edge [fontname=\"Courier\", fontsize=10];\n\n");

    CFG* cfg = func->cfg;
    BasicBlock* bb = cfg->blocks;
    while (bb) {
        /* Пропускаем недостижимые (мёртвые) блоки */
        if (!reachable[bb->id]) { bb = bb->next; continue; }

        if (bb->is_entry) {
            fprintf(f, "  cfg_%d [label=\"ENTRY\", shape=circle, style=filled, fillcolor=palegreen];\n", bb->id);
            bb = bb->next; continue;
        }
        if (bb->is_exit) {
            fprintf(f, "  cfg_%d [label=\"EXIT\", shape=circle, style=filled, fillcolor=indianred1];\n", bb->id);
            bb = bb->next; continue;
        }

        const char* shape = "box";
        if (bb->operations) {
            const char* ot = bb->operations->op_type ? bb->operations->op_type : "";
            if (strcmp(ot, "IF_COND") == 0 || strcmp(ot, "LOOP_COND") == 0 || strcmp(ot, "REPEAT_COND") == 0)
                shape = "diamond";
        }

        char lbl[1024] = "";
        bbLabel(bb, lbl, sizeof(lbl));

        fprintf(f, "  cfg_%d [label=\"", bb->id);
        if (lbl[0] != '\0') esc(f, lbl);
        else fprintf(f, "BB%d", bb->id);
        fprintf(f, "\", shape=%s, style=filled, fillcolor=white];\n", shape);

        if (bb->ast_node)
            drawAstCluster(f, bb->id, bb->ast_node);

        bb = bb->next;
    }

    fprintf(f, "\n");

    bb = cfg->blocks;
    while (bb) {
        /* Рёбра только из достижимых блоков */
        if (!reachable[bb->id]) { bb = bb->next; continue; }

        if (bb->true_target && bb->false_target) {
            fprintf(f, "  cfg_%d -> cfg_%d [label=\"true\",  color=green];\n", bb->id, bb->true_target->id);
            fprintf(f, "  cfg_%d -> cfg_%d [label=\"false\", color=red];\n", bb->id, bb->false_target->id);
        }
        else if (bb->true_target) {
            fprintf(f, "  cfg_%d -> cfg_%d;\n", bb->id, bb->true_target->id);
        }
        bb = bb->next;
    }

    free(reachable);

    fprintf(f, "\n  labelloc=\"b\";\n  label=\"CFG: %s\";\n  fontsize=12;\n", fname);
    fprintf(f, "}\n");
    fclose(f);
}

static void collectCalls(Operation* op, char** calls, int* cnt, int max) {
    if (!op) return;
    if (op->op_type && strcmp(op->op_type, "CALL") == 0)
        if (op->left && op->left->value && *cnt < max)
            calls[(*cnt)++] = op->left->value;
    collectCalls(op->left, calls, cnt, max);
    collectCalls(op->right, calls, cnt, max);
}

static void collectCallsFromCFG(CFG* cfg, char** calls, int* cnt, int max) {
    if (!cfg) return;
    BasicBlock* bb = cfg->blocks;
    while (bb) {
        Operation* op = bb->operations;
        while (op) { collectCalls(op, calls, cnt, max); op = op->next; }
        bb = bb->next;
    }
}

void exportCallGraphToDot(FunctionCollection* functions, const char* filepath) {
    if (!functions) return;
    FILE* f = fopen(filepath, "w");
    if (!f) { fprintf(stderr, "Cannot open: %s\n", filepath); return; }

    fprintf(f, "digraph CallGraph {\n");
    fprintf(f, "  graph [label=\"Call Graph\", fontsize=14];\n");
    fprintf(f, "  node  [shape=box, style=filled, fillcolor=lightblue, fontname=\"Courier\"];\n\n");

    Function* func = functions->functions;
    while (func) {
        if (func->signature && func->signature->name)
            fprintf(f, "  \"%s\";\n", func->signature->name);
        func = func->next;
    }
    fprintf(f, "\n");

    func = functions->functions;
    while (func) {
        if (!func->signature || !func->signature->name) { func = func->next; continue; }
        char* calls[256]; int cc = 0;
        collectCallsFromCFG(func->cfg, calls, &cc, 256);
        for (int i = 0; i < cc; i++)
            fprintf(f, "  \"%s\" -> \"%s\";\n", func->signature->name, calls[i]);
        func = func->next;
    }
    fprintf(f, "}\n");
    fclose(f);
}
