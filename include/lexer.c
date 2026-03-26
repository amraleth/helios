#include "lexer.h"
#include <stdio.h>
#include <string.h>

t_lexer *l_create(char* filename, const char* contents) {
  if (!contents) {
    return NULL;
  }

  t_lexer *lexer = malloc(sizeof(t_lexer));
  if (!lexer) {
    return NULL;
  }

  lexer->contents = strdup(contents);
  if (!lexer->contents) {
    free(lexer);
    return NULL;
  }
  lexer->position = 0;
  lexer->len      = strlen(lexer->contents);
  lexer->col      = 1;
  lexer->line     = 1;
  lexer->filename = filename;

  return lexer;
}

void l_free(t_lexer *lexer) {
  free(lexer->contents);
  free(lexer->filename);
  free(lexer);
}

static int l_peek(t_lexer *lexer, size_t offset) {
  size_t pos = lexer->position + offset;
  if (pos >= lexer->len) {
    return -1;
  }
  return lexer->contents[pos];
}

static int l_expect(t_lexer *lexer, char exp) {
  return l_peek(lexer, 0) == exp;
}

static int l_advance(t_lexer *lexer) {
  if (lexer->position >= lexer->len) {
    return -1;
  }
  char c = lexer->contents[lexer->position++];
  if (c == '\n') {
    lexer->line++;
    lexer->col = 1;
  } else {
    lexer->col++;
  }
  return c;
}

static void skip_whitespace(t_lexer *lexer) {
  int c;
  while ((c = l_peek(lexer, 0)) != -1) {
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
      l_advance(lexer);
    } else {
      break;
    }
  }
}

t_token l_next_tok(t_lexer *lexer) {
  t_token token;

  skip_whitespace(lexer);

  int c = l_peek(lexer, 0);
  if (c == -1) {
    token.kind = TOK_EOF;
    token.value = NULL;
    return token;
  }

  if (c >= '0' && c <= '9') {
    size_t start = lexer->position;
    while (l_peek(lexer, 0) != -1 && ((l_peek(lexer, 0) >= '0' && l_peek(lexer, 0) <= '9') || l_peek(lexer, 0) == '.')) {
      l_advance(lexer);
    }
    size_t len = lexer->position - start;
    token.value = strndup(&lexer->contents[start], len);
    token.kind = TOK_INTEGER;
    return token;
  } else if (c == '+' || c == '-' || c == '*' || c == '/') {
    l_advance(lexer);
    token.value = strndup(&lexer->contents[lexer->position - 1], 1);
    token.kind = TOK_OPERATOR;
    return token;
  } else if (c == '(') {
    l_advance(lexer);
    token.value = NULL;
    token.kind = TOK_LPAREN;
    return token;
  } else if (c == ')') {
    l_advance(lexer);
    token.value = NULL;
    token.kind = TOK_RPAREN;
    return token;
  }

  l_advance(lexer);
  token.kind = TOK_INVALID;
  token.value = NULL;
  return token;
}

void t_print(t_lexer *lexer, t_token *token) {
  const char *kind;
  switch (token->kind) {
  case TOK_INTEGER: kind  = "Integer"; break;
  case TOK_OPERATOR: kind = "Operator"; break;
  case TOK_LPAREN: kind   = "LParen"; break;
  case TOK_RPAREN: kind   = "RParen"; break;
  case TOK_EOF: kind      = "EOF"; break;
  default: {
    l_print_error(lexer);
    return;
  }
  }
  printf("%s(%s)\n", kind, token->value ? token->value : "null");
}

void l_print_error(t_lexer *lexer) {
  size_t start = lexer->position;
  while (start > 0 && lexer->contents[start - 1] != '\n') {
    start--;
  }

  size_t end = lexer->position;
  while (end < lexer->len && lexer->contents[end] != '\n') {
    end++;
  }

  fprintf(stderr, "\n%s:%zu:%zu: error: unexpected character '%c'\n", lexer->filename, lexer->line, lexer->col, lexer->contents[lexer->position]);
  fprintf(stderr, "  %.*s\n", (int)(end-start), &lexer->contents[start]);
  fprintf(stderr, "  %*s^-----------\n", (int)(lexer->col - 2), "");
}
