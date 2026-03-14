.PHONY: run clean

CC=gcc
CFLAGS=-c
SOURCES=main.c error.c node.c parser.tab.c lex.yy.c export_ast.c export_cfg.c cfg.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=printer

# Targets

run: $(EXECUTABLE)
	chmod +x $(EXECUTABLE)
	./$(EXECUTABLE) input.txt > diagram.md

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE)