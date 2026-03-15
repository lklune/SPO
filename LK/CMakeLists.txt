cmake_minimum_required(VERSION 3.26)
project(SPO_LK C)

set(CMAKE_C_STANDARD 11)

add_executable(run_compiler
    src/main.c
    src/node.c
    src/error.c
    parser/lex.yy.c
    parser/parser.tab.c
    src/cfg.c
    src/codegen.c
    src/export_ast.c
    src/export_cfg.c
    src/export_code.c
)

target_include_directories(run_compiler PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/parser
    ${CMAKE_CURRENT_SOURCE_DIR}
)
