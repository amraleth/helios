#ifndef HELIOS_LEXER_H
#define HELIOS_LEXER_H

#include <stdlib.h>
#include <string.h>

typedef enum {
  TOK_INTEGER  = 0,
  TOK_OPERATOR = 1,
  TOK_EOF      = 2,
  TOK_INVALID  = 3,
  TOK_LPAREN   = 4,
  TOK_RPAREN   = 5
} t_token_kind;

typedef struct {
  t_token_kind  kind;
  char         *value;
  size_t        line;
  size_t        col;
} t_token;

typedef struct {
  char*  contents;
  char* filename;
  size_t len;
  size_t position;
  size_t line;
  size_t col;
} t_lexer;

t_lexer *l_create(char* filename, const char* contents);

void l_free(t_lexer *lexer);

t_token l_next_tok(t_lexer *lexer);

void t_print(t_lexer *lexer, t_token *token);

void l_print_error(t_lexer *lexer);

#endif // HELIOS_LEXER_H ends here
