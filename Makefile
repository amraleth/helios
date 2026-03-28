CC	   = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
TARGET = build/helios
SRC	   = main.c include/lexer.c include/parser.c include/ast.c

.PHONY: clean run

$(TARGET): $(SRC) | build
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

build:
	mkdir -p build

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)
