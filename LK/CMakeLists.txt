cmake_minimum_required(VERSION 3.26)
project(SPO_LK C)

set(CMAKE_C_STANDARD 11)

add_executable(run_compiler
    main.c
    node.c
    error.c
    lex.yy.c
    parser.tab.c
    cfg.c
    codegen.c
    export_ast.c
    export_cfg.c
    export_code.c
    vm/vm.c
    vm/vm_integration.c
)

target_include_directories(run_compiler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
