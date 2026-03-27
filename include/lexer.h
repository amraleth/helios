#ifndef HELIOS_LEXER_H
#define HELIOS_LEXER_H

#include <stdlib.h>
#include <string.h>

typedef enum {
  TOK_INTEGER   = 0,            // whole number
  TOK_OPERATOR  = 1,            // +, -, /, *, ^^
  TOK_EOF       = 2,            // end of file
  TOK_INVALID   = 3,            // invalid token
  TOK_LPAREN    = 4,            // (
  TOK_RPAREN    = 5,            // )
  TOK_FLOAT     = 6,            // floating point number
  TOK_SEMICOLON = 7,            // ;
  TOK_KEYWORD   = 8,            // fn, let, mut
  TOK_IDENT     = 9,            // identifier
  TOK_LBRACE    = 10,           // {
  TOK_RBRACE    = 11,           // }
  TOK_ASSIGN    = 12,           // =
} t_token_kind;

typedef struct {
t_token_kind  kind;
char         *value;
size_t        line;
size_t        col;
} t_token;

typedef struct {
  char*  contents;
  char*  filename;
  size_t len;
  size_t position;
  size_t line;
  size_t col;
} t_lexer;

t_lexer *l_create(char* filename, const char* contents);

void l_free(t_lexer *lexer);

t_token l_next_tok(t_lexer *lexer);

int t_print(t_lexer *lexer, t_token *token);

void l_print_error(t_lexer *lexer);

#endif // HELIOS_LEXER_H ends here
