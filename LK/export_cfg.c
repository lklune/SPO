/*
 * export_cfg.c
 * Экспорт CFG одной функции и графа вызовов в формат GraphViz DOT
 */

#include "export_cfg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ================================================================
 *  Вспомогательные функции
 * ================================================================ */

/* Экранирование строки для DOT-меток */
static void writeDotEscaped(FILE* f, const char* s) {
    if (!s) return;
    for (; *s; s++) {
        switch (*s) {
            case '"':  fprintf(f, "\\\""); break;
            case '\\': fprintf(f, "\\\\"); break;
            case '\n': fprintf(f, "\\n");  break;
            case '<':  fprintf(f, "\\<"); break;
            case '>':  fprintf(f, "\\>"); break;
            case '{':  fprintf(f, "\\{"); break;
            case '}':  fprintf(f, "\\}"); break;
            case '|':  fprintf(f, "\\|"); break;
            default:   fputc(*s, f);      break;
        }
    }
}

/* ================================================================
 *  Построение текстового представления дерева операций
 * ================================================================ */

static void opToString(Operation* op, char* buf, int buf_size) {
    if (!op) { strncpy(buf, "?", buf_size); return; }

    const char* t = op->op_type ? op->op_type : "";
    const char* v = op->value   ? op->value   : "";

    /* Листья */
    if (strcmp(t, "IDENTIFIER") == 0 || strcmp(t, "DEC") == 0 ||
        strcmp(t, "HEX")        == 0 || strcmp(t, "BIN") == 0 ||
        strcmp(t, "STR")        == 0 || strcmp(t, "CHAR")== 0 ||
        strcmp(t, "TRUE")       == 0 || strcmp(t, "FALSE")== 0) {
        snprintf(buf, buf_size, "%s", *v ? v : t);
        return;
    }

    char lbuf[256] = "", rbuf[256] = "";
    if (op->left)  opToString(op->left,  lbuf, sizeof(lbuf));
    if (op->right) opToString(op->right, rbuf, sizeof(rbuf));

    if (strcmp(t, "assignment") == 0)
        snprintf(buf, buf_size, "%s = %s", lbuf, rbuf);
    else if (strcmp(t, "SUM")   == 0) snprintf(buf, buf_size, "%s + %s",  lbuf, rbuf);
    else if (strcmp(t, "MINUS") == 0 && op->right)
        snprintf(buf, buf_size, "%s - %s", lbuf, rbuf);
    else if (strcmp(t, "MINUS") == 0)
        snprintf(buf, buf_size, "-%s", lbuf);
    else if (strcmp(t, "MUL")   == 0) snprintf(buf, buf_size, "%s * %s",  lbuf, rbuf);
    else if (strcmp(t, "SLASH") == 0) snprintf(buf, buf_size, "%s / %s",  lbuf, rbuf);
    else if (strcmp(t, "PERCENT") == 0) snprintf(buf, buf_size, "%s %% %s", lbuf, rbuf);
    else if (strcmp(t, "EQUALITY")    == 0) snprintf(buf, buf_size, "%s == %s", lbuf, rbuf);
    else if (strcmp(t, "NOTEQUAL")    == 0) snprintf(buf, buf_size, "%s != %s", lbuf, rbuf);
    else if (strcmp(t, "LESSTHAN")    == 0) snprintf(buf, buf_size, "%s < %s",  lbuf, rbuf);
    else if (strcmp(t, "GREATERTHAN") == 0) snprintf(buf, buf_size, "%s > %s",  lbuf, rbuf);
    else if (strcmp(t, "LESSTHANEQ")  == 0) snprintf(buf, buf_size, "%s <= %s", lbuf, rbuf);
    else if (strcmp(t, "GREATERTHANEQ")==0) snprintf(buf, buf_size, "%s >= %s", lbuf, rbuf);
    else if (strcmp(t, "AND") == 0)  snprintf(buf, buf_size, "%s && %s",  lbuf, rbuf);
    else if (strcmp(t, "OR")  == 0)  snprintf(buf, buf_size, "%s || %s",  lbuf, rbuf);
    else if (strcmp(t, "NOT") == 0)  snprintf(buf, buf_size, "!%s",       lbuf);
    else if (strcmp(t, "BIT_AND")    == 0) snprintf(buf, buf_size, "%s & %s",  lbuf, rbuf);
    else if (strcmp(t, "BIT_OR")     == 0) snprintf(buf, buf_size, "%s | %s",  lbuf, rbuf);
    else if (strcmp(t, "BIT_XOR")    == 0) snprintf(buf, buf_size, "%s ^ %s",  lbuf, rbuf);
    else if (strcmp(t, "BIT_NOT")    == 0) snprintf(buf, buf_size, "~%s",       lbuf);
    else if (strcmp(t, "SHIFT_LEFT") == 0) snprintf(buf, buf_size, "%s << %s", lbuf, rbuf);
    else if (strcmp(t, "SHIFT_RIGHT")== 0) snprintf(buf, buf_size, "%s >> %s", lbuf, rbuf);
    else if (strcmp(t, "CALL")       == 0) snprintf(buf, buf_size, "%s(%s)",    lbuf, rbuf);
    else if (strcmp(t, "IF_COND")    == 0) snprintf(buf, buf_size, "if (%s)",   lbuf);
    else if (strcmp(t, "LOOP_COND")  == 0) snprintf(buf, buf_size, "%s (%s)", v, lbuf);
    else if (strcmp(t, "REPEAT_COND")== 0) snprintf(buf, buf_size, "do..%s (%s)", v, lbuf);
    else if (strcmp(t, "VAR_DECL")   == 0) snprintf(buf, buf_size, "var %s: %s", rbuf, lbuf);
    else if (strcmp(t, "BREAK")      == 0) snprintf(buf, buf_size, "break");
    else if (*v) snprintf(buf, buf_size, "%s(%s)", t, v);
    else         snprintf(buf, buf_size, "%s", t);
}

/* ================================================================
 *  Экспорт CFG функции
 * ================================================================ */

void exportCFGToDot(Function* func, const char* filepath) {
    if (!func || !func->cfg) return;

    FILE* f = fopen(filepath, "w");
    if (!f) {
        fprintf(stderr, "Cannot open file for writing: %s\n", filepath);
        return;
    }

    const char* fname = (func->signature && func->signature->name)
                        ? func->signature->name : "unknown";

    fprintf(f, "digraph CFG_%s {\n", fname);
    fprintf(f, "  graph [label=\"CFG: %s\", fontsize=14, rankdir=TB];\n", fname);
    fprintf(f, "  node  [shape=record, fontname=\"Courier\", fontsize=11];\n");
    fprintf(f, "  edge  [fontname=\"Courier\", fontsize=10];\n\n");

    CFG* cfg = func->cfg;

    /* Собираем все блоки в массив для упорядоченного вывода */
    /* Блоки хранятся в обратном порядке добавления — обходим список */
    /* Для красивого вывода нумеруем по id */
    BasicBlock* bb = cfg->blocks;
    while (bb) {
        /* Метка блока */
        fprintf(f, "  BB%d [", bb->id);

        if (bb->is_entry)
            fprintf(f, "style=filled, fillcolor=lightgreen, ");
        else if (bb->is_exit)
            fprintf(f, "style=filled, fillcolor=lightsalmon, ");
        else
            fprintf(f, "style=filled, fillcolor=lightyellow, ");

        fprintf(f, "label=\"{BB%d", bb->id);
        if (bb->is_entry) fprintf(f, " [ENTRY]");
        if (bb->is_exit)  fprintf(f, " [EXIT]");

        /* Операции */
        Operation* op = bb->operations;
        while (op) {
            char buf[512] = "";
            opToString(op, buf, sizeof(buf));
            fprintf(f, " | ");
            writeDotEscaped(f, buf);
            /* В цепочке операции связаны через right если addOperationToBlock
               не создал отдельную структуру. Но у нас addOperationToBlock
               связывает через right. Поэтому идём по right. */
            op = op->right;
        }

        fprintf(f, "}\"];\n");
        bb = bb->next;
    }

    fprintf(f, "\n");

    /* Рёбра */
    bb = cfg->blocks;
    while (bb) {
        if (bb->true_target && bb->false_target) {
            fprintf(f, "  BB%d -> BB%d [label=\"true\",  color=green];\n",
                    bb->id, bb->true_target->id);
            fprintf(f, "  BB%d -> BB%d [label=\"false\", color=red];\n",
                    bb->id, bb->false_target->id);
        } else if (bb->true_target) {
            fprintf(f, "  BB%d -> BB%d;\n", bb->id, bb->true_target->id);
        }
        bb = bb->next;
    }

    fprintf(f, "}\n");
    fclose(f);
}

/* ================================================================
 *  Граф вызовов
 * ================================================================ */

/* Сбор всех CALL-операций из дерева операций */
static void collectCalls(Operation* op, char** calls, int* count, int max) {
    if (!op) return;
    if (op->op_type && strcmp(op->op_type, "CALL") == 0) {
        /* Имя функции — левый потомок (IDENTIFIER) */
        if (op->left && op->left->value && *count < max) {
            calls[(*count)++] = op->left->value;
        }
    }
    /* Не идём через right — там цепочка операций блока, не дерево */
    collectCalls(op->left, calls, count, max);
    /* right для дерева операций */
    /* Осторожно: right в блоке — это следующая операция, не правый операнд!
       В exprToOp мы создаём деревья (right = правый операнд).
       В addOperationToBlock мы идём по right как по списку.
       Эти два использования конфликтуют.
       Для сбора вызовов идём только по структуре дерева операций,
       не по цепочке блока. */
}

/* Сбор вызовов из всех блоков CFG */
static void collectCallsFromCFG(CFG* cfg, char** calls, int* count, int max) {
    if (!cfg) return;
    BasicBlock* bb = cfg->blocks;
    while (bb) {
        Operation* op = bb->operations;
        while (op) {
            collectCalls(op, calls, count, max);
            op = op->right; /* следующая операция в блоке */
        }
        bb = bb->next;
    }
}

void exportCallGraphToDot(FunctionCollection* functions, const char* filepath) {
    if (!functions) return;

    FILE* f = fopen(filepath, "w");
    if (!f) {
        fprintf(stderr, "Cannot open file for writing: %s\n", filepath);
        return;
    }

    fprintf(f, "digraph CallGraph {\n");
    fprintf(f, "  graph [label=\"Call Graph\", fontsize=14];\n");
    fprintf(f, "  node  [shape=box, style=filled, fillcolor=lightblue,"
               " fontname=\"Courier\"];\n\n");

    /* Вывести все известные функции как узлы */
    Function* func = functions->functions;
    while (func) {
        if (func->signature && func->signature->name) {
            fprintf(f, "  \"%s\";\n", func->signature->name);
        }
        func = func->next;
    }
    fprintf(f, "\n");

    /* Рёбра: caller -> callee */
    func = functions->functions;
    while (func) {
        if (!func->signature || !func->signature->name) {
            func = func->next;
            continue;
        }
        const char* caller = func->signature->name;

        char* calls[256];
        int   call_count = 0;
        collectCallsFromCFG(func->cfg, calls, &call_count, 256);

        for (int i = 0; i < call_count; i++) {
            fprintf(f, "  \"%s\" -> \"%s\";\n", caller, calls[i]);
        }

        func = func->next;
    }

    fprintf(f, "}\n");
    fclose(f);
}
