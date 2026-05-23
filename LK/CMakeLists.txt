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

set(SCHEDULER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/scheduler/include)

set(SCHEDULER_HEADERS
    scheduler/include/platform_runtime.h
    scheduler/include/scheduler_base.h
)

set(SCHEDULER_COMMON_SOURCES
    scheduler/src/common/scheduler_base.c
)

set(SCHEDULER_APP_SOURCES
    scheduler/src/app/main.c
)

set(SCHEDULER_LINUX_SOURCES
    scheduler/src/platform_linux/platform_runtime_linux.c
)

set(SCHEDULER_WINDOWS_SOURCES
    scheduler/src/platform_windows/platform_runtime_windows.c
)

set(SCHEDULER_ALL_SOURCES
    ${SCHEDULER_HEADERS}
    ${SCHEDULER_COMMON_SOURCES}
    ${SCHEDULER_APP_SOURCES}
    ${SCHEDULER_LINUX_SOURCES}
    ${SCHEDULER_WINDOWS_SOURCES}
)

add_executable(scheduler_demo
    ${SCHEDULER_ALL_SOURCES}
)

if(WIN32)
    set_source_files_properties(${SCHEDULER_LINUX_SOURCES} PROPERTIES HEADER_FILE_ONLY ON)
else()
    set_source_files_properties(${SCHEDULER_WINDOWS_SOURCES} PROPERTIES HEADER_FILE_ONLY ON)
endif()

target_include_directories(scheduler_demo PRIVATE
    ${SCHEDULER_INCLUDE_DIR}
)

if(MSVC)
    source_group("scheduler\\include" FILES ${SCHEDULER_HEADERS})
    source_group("scheduler\\src\\common" FILES ${SCHEDULER_COMMON_SOURCES})
    source_group("scheduler\\src\\app" FILES ${SCHEDULER_APP_SOURCES})
    source_group("scheduler\\src\\platform_linux" FILES ${SCHEDULER_LINUX_SOURCES})
    source_group("scheduler\\src\\platform_windows" FILES ${SCHEDULER_WINDOWS_SOURCES})
endif()
