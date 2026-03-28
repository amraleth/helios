CC	   = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
TARGET = helios
SRC	   = main.c include/lexer.c include/parser.c include/ast.c

.PHONY: clean run

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)
