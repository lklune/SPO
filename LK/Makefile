CC     = gcc
CFLAGS = -O0 -g
SRC_DIR = .
OUT_DIR = pictures

SRC = $(SRC_DIR)/main.c \
      $(SRC_DIR)/node.c \
      $(SRC_DIR)/error.c \
      $(SRC_DIR)/cfg.c \
      $(SRC_DIR)/export_ast.c \
      $(SRC_DIR)/export_cfg.c \
      $(SRC_DIR)/parser.tab.c \
      $(SRC_DIR)/lex.yy.c

OBJ     = $(SRC:.c=.o)
BIN = printer

build: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $(BIN) $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: build
#	./$(BIN) main.txt 
#	math.txt control.txt
	./$(BIN) input.txt

clean:
	rm -f $(BIN) $(SRC_DIR)/*.o \
	      $(SRC_DIR)/*.dot $(SRC_DIR)/*.json \
	      $(OUT_DIR)/*.png

cleanout:
	rm -f $(SRC_DIR)/*.o $(SRC_DIR)/*.dot $(SRC_DIR)/*.json

rebuild: clean build

.PHONY: build run clean rebuild cleanout png
