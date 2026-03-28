#include <stdio.h>
#include <stdlib.h>

#include "include/ast.h"
#include "include/lexer.h"
#include "include/parser.h"

char* FILENAME = "test.helios";

enum file_op_error {
  SUCCESS        = 0,
  FILE_NOT_FOUND = 1,
  ALLOC_FAILED   = 2,
  READ_FAILED    = 3
};

void print_file_error(int i) {
  switch (i) {
  case 1: printf("File not found"); break;
  case 2: printf("Allocation failed"); break;
  case 3: printf("File read failed"); break;
  default: printf("Unknown error"); break;
  }
}

int read_file(const char** out, const char* filename) {
  char *buffer;
  size_t len;
  FILE *f = fopen(filename, "r");

  if (!f) {
    return FILE_NOT_FOUND;
  }

  fseek(f, 0, SEEK_END);
  len = ftell(f);
  fseek(f, 0, SEEK_SET);
  buffer = malloc(len + 1);
  if (!buffer) {
    return ALLOC_FAILED;
  }

  if (fread(buffer, 1, len, f) != len) {
    return READ_FAILED;
  }
  fclose(f);

  buffer[len] = '\0';
  *out = buffer;

  return 0;
}

int main() {
  const char *file;
  int err = read_file(&file, FILENAME);
  if (err != 0) {
    print_file_error(err);
    return 1;
  }

  // TODO: read filename from args
  printf("Evaluating file %s:\n%s\n\n", FILENAME, file);

  t_lexer *lexer = l_create(FILENAME, file);
  if (!lexer) {
    printf("Failed to create lexer for \n '%s'", file);
    return 1;
  }

  t_parser *parser = p_create(lexer);
  if (!parser) {
    printf("Failed to create parser\n");
    l_free(lexer);
    return 1;
  }

  t_node *tree = p_parse_expression(parser);
  node_print(tree);

  free(parser);
  l_free(lexer);
  free((void *)file);
  return 0;
}
