# Makefile for lab 3
CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude -Iparser
LDFLAGS = -lm
BISON = bison
FLEX = flex
TT = /mnt/c/Users/december/Desktop/SPO/260321/SPO/LK/RemoteTasks/Portable.RemoteTasks.Manager.exe -ul 472626 -up a72ac3ca-c226-422f-bcbe-a2b70133fd16

BIN_DIR = bin
OBJ_DIR = obj
GEN_DIR = generated
SRC_DIR = src
INCLUDE_DIR = include
PARSER_DIR = parser
PARSER_SRC = $(PARSER_DIR)/parser.y
LEXER_SRC = $(PARSER_DIR)/lexems.l
PARSER_GEN_C = $(PARSER_DIR)/parser.tab.c
PARSER_GEN_H = $(PARSER_DIR)/parser.tab.h
LEXER_GEN_C = $(PARSER_DIR)/lex.yy.c

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

$(PARSER_GEN_C) $(PARSER_GEN_H): $(PARSER_SRC)
	$(BISON) -d $< -o $(PARSER_GEN_C)

$(LEXER_GEN_C): $(LEXER_SRC) $(PARSER_GEN_H)
	$(FLEX) -o $@ $<

$(OBJ_DIR)/lex.yy.o: $(LEXER_GEN_C) $(INCLUDE_DIR)/error.h $(PARSER_GEN_H) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/parser.tab.o: $(PARSER_GEN_C) $(PARSER_GEN_H) $(INCLUDE_DIR)/node.h $(INCLUDE_DIR)/error.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/run_compiler: $(SRC_DIR)/main.c $(COMPILER_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "Built: $(BIN_DIR)/run_compiler"

clean:
	rm -f $(OBJ_DIR)/*.o $(TARGETS)
	rm -rf $(GEN_DIR)
	rm -f $(PARSER_GEN_C) $(PARSER_GEN_H) $(LEXER_GEN_C)

run: $(BIN_DIR)/run_compiler | $(GEN_DIR)
	./$(BIN_DIR)/run_compiler -o $(GEN_DIR) input.txt
	./$(BIN_DIR)/run_compiler -o $(GEN_DIR) calc.src

vm-run: $(BIN_DIR)/run_compiler | $(GEN_DIR)
	./$(BIN_DIR)/run_compiler -o $(GEN_DIR) calc.src
	id="$$( $(TT) -ws -w -id -s Assemble asmListing $(GEN_DIR)/program.asm definitionFile vm.target.pdsl archName vm | head -n 1 )"; \
	$(TT) -ws -g "$$id" -r stdout.txt; \
	$(TT) -ws -g "$$id" -r out.ptptb -o out.ptptb; \
	$(TT) -ws -il -s ExecuteBinaryWithInteractiveInput definitionFile vm.target.pdsl stdinRegStName sin stdoutRegStName sout archName vm binaryFileToRun out.ptptb ipRegStorageName ipst finishMnemonicName END codeRamBankName code_ram

.PHONY: all clean run vm-run
