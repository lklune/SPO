/*
 * main.c
 * Тестовая программа: разбор входных файлов, построение CFG,
 * экспорт в DOT-файлы.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"
#include "error.h"
#include "export_ast.h"
#include "cfg.h"
#include "export_cfg.h"
#include "codegen.h"
#include "export_code.h"

extern FILE* yyin;
extern int   yyparse(void);

extern Node** allNodes;
extern size_t allNodesCount;

static void basename_noext(const char* path, char* out, int out_size) {
    const char* p = path;
    const char* last_sep = NULL;
    for (const char* q = path; *q; q++)
        if (*q == '/' || *q == '\\') last_sep = q;
    if (last_sep) p = last_sep + 1;
    strncpy(out, p, out_size - 1);
    out[out_size - 1] = '\0';
    char* dot = strrchr(out, '.');
    if (dot) *dot = '\0';
}

static void dirname_of(const char* path, char* out, int out_size) {
    strncpy(out, path, out_size - 1);
    out[out_size - 1] = '\0';
    char* last_sep = NULL;
    for (char* q = out; *q; q++)
        if (*q == '/' || *q == '\\') last_sep = q;
    if (last_sep) *last_sep = '\0';
    else          strncpy(out, ".", out_size);
}

static Node* parseFile(const char* filename) {
    size_t before = allNodesCount;
    FILE* input_file = fopen(filename, "r");
    if (!input_file) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return NULL;
    }
    yyin = input_file;
    int parse_result = yyparse();
    fclose(input_file);
    if (parse_result != 0) {
        fprintf(stderr, "Parse error in file: %s\n", filename);
        return NULL;
    }
    if (allNodesCount <= before) {
        fprintf(stderr, "No AST nodes produced for file: %s\n", filename);
        return NULL;
    }
    return allNodes[allNodesCount - 1];
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [-o <outdir>] <file1> [file2 ...]\n", argv[0]);
        return 1;
    }

    const char* outdir = NULL;
    int         first_file = 1;

    if (argc >= 3 && strcmp(argv[1], "-o") == 0) {
        outdir = argv[2];
        first_file = 3;
    }

    if (first_file >= argc) {
        fprintf(stderr, "No input files specified.\n");
        return 1;
    }

    // start
    FileCollection* file_col = createFileCollection();

    for (int i = first_file; i < argc; i++) {
        const char* fname = argv[i];
        printf("Parsing: %s\n", fname);

        Node* root = parseFile(fname);
        if (!root) {
            fprintf(stderr, "Skipping file to parse error: %s\n", fname);
            continue;
        }

        /* Экспорт AST в JSON */
        char ast_path[1024];
        char bname[256], dname[256];
        basename_noext(fname, bname, sizeof(bname));
        dirname_of(fname, dname, sizeof(dname));
        snprintf(ast_path, sizeof(ast_path), "%s/%s.ast.json", dname, bname);
        exportAstToJson(root, ast_path);
        printf("  AST saved: %s\n", ast_path);

        addFileToCollection(file_col, fname, root);
    }

    /* Построение CFG */
    printf("\nBuilding CFGs...\n");
    AnalysisResult* result = buildCFGFromAST(file_col);
    CompiledFunctionCollection* compiled = generateCodeFromAST(result->functions);
    exportAllCompiledFunctions(compiled, "");

    /* Ошибки → stderr */
    if (result->errors && result->errors->error_count > 0) {
        Error* e = result->errors->errors;
        while (e) {
            fprintf(stderr, "CFG Error [%s:%d]: %s\n",
                e->filename ? e->filename : "?",
                e->line_number,
                e->message ? e->message : "unknown");
            e = e->next;
        }
    }

    /* Экспорт CFG каждой функции */
    const char* main_func_file = NULL;

    Function* func = result->functions->functions;
    while (func) {
        if (!func->signature || !func->signature->name) { func = func->next; continue; }

        const char* src = func->source_file ? func->source_file : ".";
        const char* fname = func->signature->name;

        char dir[512];
        if (outdir) strncpy(dir, outdir, sizeof(dir) - 1);
        else        dirname_of(src, dir, sizeof(dir));
        dir[sizeof(dir) - 1] = '\0';

        char src_base[256];
        basename_noext(src, src_base, sizeof(src_base));

        char dot_path[1024];
        snprintf(dot_path, sizeof(dot_path), "%s/%s.%s.dot", dir, src_base, fname);

        exportCFGToDot(func, dot_path);
        printf("  CFG saved: %s\n", dot_path);

        if (strcmp(fname, "main") == 0 && !main_func_file)
            main_func_file = func->source_file;

        func = func->next;
    }

    /* Граф вызовов */
    {
        char cg_dir[512];
        if (outdir)           strncpy(cg_dir, outdir, sizeof(cg_dir) - 1);
        else if (main_func_file) dirname_of(main_func_file, cg_dir, sizeof(cg_dir));
        else                  strncpy(cg_dir, ".", sizeof(cg_dir));
        cg_dir[sizeof(cg_dir) - 1] = '\0';

        char cg_path[1024];
        snprintf(cg_path, sizeof(cg_path), "%s/call_graph.dot", cg_dir);
        exportCallGraphToDot(result->functions, cg_path);
        printf("  Call graph saved: %s\n", cg_path);
    }

    printf("\nDone. Functions found: %d, Errors: %d\n",
        result->functions->function_count,
        result->errors->error_count);

    freeAnalysisResult(result);
    return 0;
}