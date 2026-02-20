.PHONY: run clean all

CC = gcc
CFLAGS = -c

SOURCES = main.c error.c node.c parser.tab.c lex.yy.c export_ast.c cfg.c export_cfg.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = printer

# Targets

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(EXECUTABLE)
	chmod +x $(EXECUTABLE)
	./$(EXECUTABLE) input.txt

clean:
	rm -rf *.o $(EXECUTABLE) *.dot *.json
