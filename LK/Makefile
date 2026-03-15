# Makefile for lab 3
CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude -Iparser
LDFLAGS = -lm

BIN_DIR = bin
OBJ_DIR = obj
GEN_DIR = generated
SRC_DIR = src
INCLUDE_DIR = include
PARSER_DIR = parser

COMPILER_OBJS = \
	$(OBJ_DIR)/node.o \
	$(OBJ_DIR)/error.o \
	$(OBJ_DIR)/cfg.o \
	$(OBJ_DIR)/codegen.o \
	$(OBJ_DIR)/export_ast.o \
	$(OBJ_DIR)/export_cfg.o \
	$(OBJ_DIR)/export_code.o \
	$(OBJ_DIR)/lex.yy.o \
	$(OBJ_DIR)/parser.tab.o

TARGETS = $(BIN_DIR)/run_compiler

all: $(TARGETS)

$(BIN_DIR) $(OBJ_DIR) $(GEN_DIR):
	mkdir -p $@

$(OBJ_DIR)/node.o: $(SRC_DIR)/node.c $(INCLUDE_DIR)/node.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/error.o: $(SRC_DIR)/error.c $(INCLUDE_DIR)/error.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/cfg.o: $(SRC_DIR)/cfg.c $(INCLUDE_DIR)/cfg.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/codegen.o: $(SRC_DIR)/codegen.c $(INCLUDE_DIR)/codegen.h $(INCLUDE_DIR)/cfg.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/export_ast.o: $(SRC_DIR)/export_ast.c $(INCLUDE_DIR)/export_ast.h $(INCLUDE_DIR)/node.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/export_cfg.o: $(SRC_DIR)/export_cfg.c $(INCLUDE_DIR)/export_cfg.h $(INCLUDE_DIR)/cfg.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/export_code.o: $(SRC_DIR)/export_code.c $(INCLUDE_DIR)/export_code.h $(INCLUDE_DIR)/codegen.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/lex.yy.o: $(PARSER_DIR)/lex.yy.c $(INCLUDE_DIR)/error.h $(PARSER_DIR)/parser.tab.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/parser.tab.o: $(PARSER_DIR)/parser.tab.c $(PARSER_DIR)/parser.tab.h $(INCLUDE_DIR)/node.h $(INCLUDE_DIR)/error.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/run_compiler: $(SRC_DIR)/main.c $(COMPILER_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "Built: $(BIN_DIR)/run_compiler"

clean:
	rm -f $(OBJ_DIR)/*.o $(TARGETS)
	rm -rf $(GEN_DIR)

run: $(BIN_DIR)/run_compiler | $(GEN_DIR)
	./$(BIN_DIR)/run_compiler -o $(GEN_DIR) input.txt

.PHONY: all clean run
