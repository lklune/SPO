# Makefile для задания 3
# Виртуальная машина с динамической типизацией и двухадресным кодом
# Основные исходники в корне, файлы ВМ в подкаталоге vm/

CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99
LDFLAGS = -lm

# Подкаталог с файлами виртуальной машины
VM_DIR = vm
# Флаг -I. нужен файлам из vm/, чтобы они находили codegen.h и прочие заголовки корня
VM_CFLAGS = $(CFLAGS) -I.

# Объектные файлы основного компилятора
COMPILER_OBJS = \
	node.o \
	error.o \
	cfg.o \
	codegen.o \
	export_ast.o \
	export_cfg.o \
	export_code.o \
	vm.o \
	vm_integration.o \
	lex.yy.o \
	parser.tab.o

# Объектные файлы для тестирования ВМ
TEST_OBJS = \
	node.o \
	error.o \
	codegen.o \
	vm.o \
	test_vm.o

# Все целевые программы
TARGETS = test_vm run_compiler

# Правило по умолчанию
all: $(TARGETS)

# ============================================================================
# Компиляция объектных файлов
# ============================================================================

node.o: node.c node.h
	$(CC) $(CFLAGS) -c $< -o $@

error.o: error.c error.h
	$(CC) $(CFLAGS) -c $< -o $@

cfg.o: cfg.c cfg.h
	$(CC) $(CFLAGS) -c $< -o $@

codegen.o: codegen.c codegen.h cfg.h
	$(CC) $(CFLAGS) -c $< -o $@

export_ast.o: export_ast.c export_ast.h node.h
	$(CC) $(CFLAGS) -c $< -o $@

export_cfg.o: export_cfg.c export_cfg.h cfg.h
	$(CC) $(CFLAGS) -c $< -o $@

export_code.o: export_code.c export_code.h codegen.h
	$(CC) $(CFLAGS) -c $< -o $@

vm.o: $(VM_DIR)/vm.c $(VM_DIR)/vm.h codegen.h
	$(CC) $(VM_CFLAGS) -c $< -o $@

vm_integration.o: $(VM_DIR)/vm_integration.c $(VM_DIR)/vm.h codegen.h
	$(CC) $(VM_CFLAGS) -c $< -o $@

test_vm.o: $(VM_DIR)/test_vm.c $(VM_DIR)/vm.h codegen.h
	$(CC) $(VM_CFLAGS) -c $< -o $@

# ============================================================================
# Компиляция целевых программ
# ============================================================================

# Программа для тестирования ВМ
test_vm: $(TEST_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "✓ Built: test_vm"

# Основной компилятор
run_compiler: main.c $(COMPILER_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "✓ Built: run_compiler"

# ============================================================================
# Очистка
# ============================================================================

clean:
	rm -f *.o $(TARGETS)
	@echo "✓ Cleaned"

# ============================================================================
# Запуск тестов
# ============================================================================

test: test_vm
	@echo "Running VM tests..."
	./test_vm

test-verbose: test_vm
	@echo "Running VM tests (verbose)..."
	./test_vm -v

# ============================================================================
# Помощь
# ============================================================================

help:
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build all binaries (default)"
	@echo "  run_compiler - Build main compiler"
	@echo "  test_vm      - Build VM test binary"
	@echo "  test         - Run VM tests"
	@echo "  test-verbose - Run tests with verbose output"
	@echo "  clean        - Remove object files and binaries"
	@echo "  help         - Show this help"

# ============================================================================
# Фиктивные правила
# ============================================================================

.PHONY: all clean test test-verbose help