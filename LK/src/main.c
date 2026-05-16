#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define MKDIR(path) mkdir(path, 0777)
#endif

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

/* Получение имени файла без пути и расширения */
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

/* Получение директории из пути */
static void dirname_of(const char* path, char* out, int out_size) {
    strncpy(out, path, out_size - 1);
    out[out_size - 1] = '\0';
    char* last_sep = NULL;
    for (char* q = out; *q; q++)
        if (*q == '/' || *q == '\\') last_sep = q;
    if (last_sep) *last_sep = '\0';
    else          strncpy(out, ".", out_size);
}

static void ensure_directory_exists(const char* path) {
    char buffer[1024];
    size_t len;

    if (!path || !*path) {
        return;
    }

    len = strlen(path);
    if (len >= sizeof(buffer)) {
        return;
    }

    strcpy(buffer, path);

    for (size_t i = 1; i < len; i++) {
        if (buffer[i] == '/' || buffer[i] == '\\') {
            char saved = buffer[i];
            buffer[i] = '\0';
            if (buffer[0] != '\0') {
                if (MKDIR(buffer) != 0 && errno != EEXIST) {
                    buffer[i] = saved;
                    return;
                }
            }
            buffer[i] = saved;
        }
    }

    if (MKDIR(buffer) != 0 && errno != EEXIST) {
        return;
    }
}

/* Разбор одного входного файла
 * Если парсер не собрал AST, возвращается NULL
 */
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

/* Основной вход компилятора
 * Здесь читаются файлы, строится CFG, генерируется код и экспортируются результаты
 */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [-o <outdir>] <file1> [file2 ...]\n", argv[0]);
        return 1;
    }

    const char* outdir = NULL;
    int         first_file = 1;
    int         had_parse_errors = 0;
    int         parsed_files = 0;

    if (argc >= 3 && strcmp(argv[1], "-o") == 0) {
        outdir = argv[2];
        first_file = 3;
    }

    if (first_file >= argc) {
        fprintf(stderr, "No input files specified.\n");
        return 1;
    }

    if (outdir) {
        ensure_directory_exists(outdir);
    }

    FileCollection* file_col = createFileCollection();

    for (int i = first_file; i < argc; i++) {
        const char* fname = argv[i];
        printf("Parsing: %s\n", fname);

        Node* root = parseFile(fname);
        if (!root) {
            had_parse_errors = 1;
            fprintf(stderr, "Skipping file to parse error: %s\n", fname);
            continue;
        }

        parsed_files++;

        /* Экспорт AST в JSON */
        char ast_path[1024];
        char bname[256], dname[256];
        basename_noext(fname, bname, sizeof(bname));
        dirname_of(fname, dname, sizeof(dname));
        snprintf(ast_path, sizeof(ast_path), "%s/%s.ast.json",
            outdir ? outdir : dname, bname);
        exportAstToJson(root, ast_path);
        printf("  AST saved: %s\n", ast_path);

        addFileToCollection(file_col, fname, root);
    }

    if (parsed_files == 0) {
        fprintf(stderr, "No input files parsed successfully.\n");
        return 1;
    }

    /* Build CFG */
    printf("\nBuilding CFGs...\n");
    AnalysisResult* result = buildCFGFromAST(file_col);
    if (!result || !result->functions || !result->errors) {
        fprintf(stderr, "CFG build failed\n");
        return 1;
    }
    CompiledFunctionCollection* compiled = generateCodeFromAST(result->functions);
    if (!compiled) {
        fprintf(stderr, "Code generation failed\n");
        freeAnalysisResult(result);
        return 1;
    }

    const char* code_outdir = outdir ? outdir : ".";
    exportAllCompiledFunctions(compiled, code_outdir);

    {
        char program_path[1024];
        snprintf(program_path, sizeof(program_path), "%s/program.asm", code_outdir);
        exportProgramAsm(result, compiled, program_path);
        printf("Exporting combined code: %s\n", program_path);
    }

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

    int exit_code = (had_parse_errors || result->errors->error_count > 0) ? 1 : 0;

    freeCompiledFunctionCollection(compiled);
    freeAnalysisResult(result);
    return exit_code;
}
