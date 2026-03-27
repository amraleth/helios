#ifndef HELIOS_LEXER_H
#define HELIOS_LEXER_H

#include <stdlib.h>
#include <string.h>

typedef enum {
  TOK_OPERATOR,                 // +, -, /, *, ^^
  TOK_EOF,                      // end of file
  TOK_INVALID,                  // invalid token
  TOK_LPAREN,                   // (
  TOK_RPAREN,                   // )
  TOK_SEMICOLON,                // ;
  TOK_KEYWORD,                  // fn, let, mut
  TOK_IDENT,                    // identifier
  TOK_LBRACE,                   // {
  TOK_RBRACE,                   // }
  TOK_ASSIGN,                   // =
  TOK_AT,                       // @
  TOK_DATA_TYPE,                // void, str, i8, i16... u8, u16... bool
  TOK_COLON,                    // :
  TOK_INTEGER,
  TOK_FLOAT,
  TOK_STRING,
  TOK_EQ,
  TOK_LESS,
  TOK_LEQ,
  TOK_GREATER,
  TOK_GEQ,
  TOK_COMMA,
  TOK_QUESTION,
  TOK_PP,
  TOK_DOT,
} t_token_kind;

typedef enum {
  TYPE_INT,
  TYPE_UINT,
  TYPE_FLOAT,
  TYPE_BOOL,
  TYPE_VOID,
  TYPE_STRING,
  TYPE_UNKNOWN,
} t_type_kind;

typedef struct {
  t_type_kind kind;
  union {
    int width;
  };
} t_data_type;

typedef struct {
  t_token_kind  kind;
  char         *value;
  t_data_type   dtype;
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
