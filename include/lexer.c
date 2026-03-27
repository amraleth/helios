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

static char *norm_float(const char *raw, size_t len) {
  if (raw[0] == '.') {
    char *out = malloc(len + 2);
    out[0] = '0';
    memcpy(out + 1, raw, len);
    out[len + 1] = '\0';
    return out;
  }

  if (raw[len - 1] == '.') {
    char *out = malloc(len + 2);
    memcpy(out, raw, len);
    out[len] = '0';
    out[len + 1] = '\0';
    return out;
  }
  return strndup(raw, len);
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

  if ((c >= '0' && c <= '9') || (c == '.' && l_peek(lexer, 1) >= '0' && l_peek(lexer, 1) <= '9')) {
    size_t start = lexer->position;
    int dots = 0;

    // TODO: support for hex integers
    // integer part
    while (l_peek(lexer, 0) >= '0' && l_peek(lexer, 0) <= '9') {
      l_advance(lexer);
    }

    // fractional part
    if (l_peek(lexer, 0) == '.') {
      dots++;
      l_advance(lexer);
      while (l_peek(lexer, 0) >= '0' && l_peek(lexer, 0) <= '9') {
        l_advance(lexer);
      }

      // TODO: could also be an identifier
      if (l_peek(lexer, 0) == '.') {
        l_print_error(lexer);
        token.value = NULL;
        token.kind = TOK_INVALID;
        return token;
      }
    }

    size_t len = lexer->position - start;
    if (dots) {
      token.value = norm_float(&lexer->contents[start], len);
      token.kind = TOK_FLOAT;
      return token;
    }
    token.value = strndup(&lexer->contents[start], len);
    token.kind = TOK_INTEGER;
    return token;
  } else if (c == '+' || c == '-' || c == '*' || c == '/') {
    if (c == '/' && lexer->contents[lexer->position + 1] == '/') {
      while (lexer->contents[lexer->position] != '\0' &&
             lexer->contents[lexer->position] != '\n') {
        l_advance(lexer);
      }
      return l_next_tok(lexer);
    }
    
    if (c == '/' && lexer->contents[lexer->position + 1] == '*') {
      l_advance(lexer);
      l_advance(lexer);

      while (lexer->contents[lexer->position] != '\0') {
        if (lexer->contents[lexer->position] == '*' &&
            lexer->contents[lexer->position + 1] == '/') {
          l_advance(lexer);
          l_advance(lexer);
          break;
        }
        l_advance(lexer);
      }
      return l_next_tok(lexer);
    }
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

int t_print(t_lexer *lexer, t_token *token) {
  const char *kind;
  switch (token->kind) {
  case TOK_INTEGER: kind  = "Integer"; break;
  case TOK_OPERATOR: kind = "Operator"; break;
  case TOK_LPAREN: kind   = "LParen"; break;
  case TOK_RPAREN: kind   = "RParen"; break;
  case TOK_EOF: kind      = "EOF"; break;
  case TOK_FLOAT: kind    = "Float"; break;
  default: {
    l_print_error(lexer);
    return -1;
  }
  }
  printf("%s(%s)\n", kind, token->value ? token->value : "null");
  return 0;
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
  fprintf(stderr, "  %*s^-----------\n", (int)(lexer->col - 1), "");
}
