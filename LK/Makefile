# Makefile for lab 3

CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lm

VM_DIR = vm
VM_CFLAGS = $(CFLAGS) -I.

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

TEST_OBJS = \
	node.o \
	error.o \
	codegen.o \
	vm.o \
	test_vm.o

TARGETS = test_vm run_compiler

all: $(TARGETS)

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

vm_integration.o: $(VM_DIR)/vm_integration.c $(VM_DIR)/vm.h codegen.h export_code.h
	$(CC) $(VM_CFLAGS) -c $< -o $@

test_vm.o: $(VM_DIR)/test_vm.c $(VM_DIR)/vm.h codegen.h
	$(CC) $(VM_CFLAGS) -c $< -o $@

test_vm: $(TEST_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "Built: test_vm"

run_compiler: main.c $(COMPILER_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "Built: run_compiler"

clean:
	rm -f *.o $(TARGETS)
	@echo "Cleaned"

test: test_vm
	./test_vm

test-verbose: test_vm
	./test_vm -v

help:
	@echo "Usage: make [target]"
	@echo "Targets: all run_compiler test_vm test test-verbose clean help"

.PHONY: all clean test test-verbose help
