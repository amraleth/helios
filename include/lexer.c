#include "lexer.h"
#include <ctype.h>
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

// supported float variants are 0. , 0.0 and .0
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

static int match_keyword(t_lexer *lexer, const char *keyword, t_token *token) {
  size_t len = strlen(keyword);
  if (lexer->position + len > lexer->len) {
    return 0;
  }

  if (strncmp(&lexer->contents[lexer->position], keyword, len) != 0) {
    return 0;
  }

  int next = l_peek(lexer, len);
  if (next != -1 && (next >= 'a' && next <= 'z' ||
                     next >= 'A' && next <= 'Z' ||
                     next >= '0' && next <= '9' ||
                     next == '_')) {
    return 0;
  }

  for (size_t i = 0; i < len; i++) {
    l_advance(lexer);
  }

  token->value = strdup(keyword);
  token->kind  = TOK_KEYWORD;
  return 1;
}

static char *l_read_alnum(t_lexer *lexer) {
  size_t start = lexer->position;

  while (isalnum((unsigned char)lexer->contents[lexer->position])) {
    l_advance(lexer);
  }

  size_t len = lexer->position - start;
  return strndup(&lexer->contents[start], len);
}

static t_data_type l_parse_dtype(const char *value) {
  if (value[0] == 'i') return (t_data_type) {TYPE_INT, .width = atoi(value + 1)};
  if (value[0] == 'u') return (t_data_type) {TYPE_UINT, .width = atoi(value + 1)};
  if (value[0] == 'f') return (t_data_type) {TYPE_FLOAT, .width = atoi(value + 1)};
  if (strcmp(value, "bool") == 0) return (t_data_type) {TYPE_BOOL};
  if (strcmp(value, "void") == 0) return (t_data_type) {TYPE_VOID};
  return (t_data_type) {TYPE_UNKNOWN};
}

t_token l_next_tok(t_lexer *lexer) {
  t_token token;

  skip_whitespace(lexer);

  int c = l_peek(lexer, 0);
  if (c == -1) {
    token.kind  = TOK_EOF;
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
        token.value = NULL;
        token.kind  = TOK_INVALID;
        return token;
      }
    }

    size_t len = lexer->position - start;
    if (dots) {
      token.value = norm_float(&lexer->contents[start], len);
      token.kind  = TOK_FLOAT;
      return token;
    }
    token.value = strndup(&lexer->contents[start], len);
    token.kind = TOK_INTEGER;
    return token;
  } else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '^') {
    if (c == '+' && l_peek(lexer, 1) == '+') {
      l_advance(lexer);
      l_advance(lexer);
      token.value = NULL;
      token.kind = TOK_PP;
      return token;
    }
    if (c == '^' && lexer->contents[lexer->position + 1] == '^') {
      l_advance(lexer);
      l_advance(lexer);
      token.value = strndup(&lexer->contents[lexer->position - 2], 2);
      token.kind  = TOK_OPERATOR;
      return token;
    } else if (c == '^') {
      l_advance(lexer);
      token.value = NULL;
      token.kind  = TOK_INVALID;
      return token;
    }
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
    token.kind  = TOK_OPERATOR;
    return token;
  } else if (c == '(') {
    l_advance(lexer);
    token.value = NULL;
    token.kind  = TOK_LPAREN;
    return token;
  } else if (c == ')') {
    l_advance(lexer);
    token.value = NULL;
    token.kind  = TOK_RPAREN;
    return token;
  } else if (c == ';') {
    l_advance(lexer);
    token.value = NULL;
    token.kind  = TOK_SEMICOLON;
    return token;
  } else if (c == '{') {
    l_advance(lexer);
    token.value = NULL;
    token.kind  = TOK_LBRACE;
    return token;
  } else if (c == '}') {
    l_advance(lexer);
    token.value = NULL;
    token.kind  = TOK_RBRACE;
    return token;
  } else if (c == '=') {
    if (l_peek(lexer, 1) == '=') {
      l_advance(lexer);
      l_advance(lexer);
      token.value = NULL;
      token.kind = TOK_EQ;
      return token;
    }
    l_advance(lexer);
    token.value = NULL;
    token.kind  = TOK_ASSIGN;
    return token;
  } else if (c == ':') {
    l_advance(lexer);
    token.value = NULL;
    token.kind = TOK_COLON;
    return token;
  } else if (c == '>') {
    token.value = NULL;
    if (l_peek(lexer, 1) == '=') {
      l_advance(lexer);
      l_advance(lexer);
      token.kind = TOK_GEQ;
      return token;
    }
    l_advance(lexer);
    token.kind = TOK_GREATER;
    return token;
  } else if (c == '<') {
    token.value = NULL;
    if (l_peek(lexer, 1) == '=') {
      l_advance(lexer);
      l_advance(lexer);
      token.kind = TOK_LEQ;
      return token;
    }
    l_advance(lexer);
    token.kind = TOK_LESS;
    return token;
  } else if (c == ',') {
    l_advance(lexer);
    token.kind = TOK_COMMA;
    token.value = NULL;
    return token;
  } else if (c == '?') {
    l_advance(lexer);
    token.kind = TOK_QUESTION;
    token.value = NULL;
    return token;
  } else if (c == '.') {
    l_advance(lexer);
    token.kind = TOK_DOT;
    token.value = NULL;
    return token;
  }
  else if (c == '"') {
    l_advance(lexer);
    size_t start = lexer->position;

    while (1) {
      int ch = l_peek(lexer, 0);
      if (ch == -1 || ch == '\0') {
        token.value = NULL;
        token.kind = TOK_INVALID;
        return token;
      }
      if (ch == '"') {
        break;
      }
      l_advance(lexer);
    }
    size_t len = lexer->position - start;
    token.value = strndup(&lexer->contents[start], len);
    token.kind = TOK_STRING;
    l_advance(lexer);
    return token;
  } else if (match_keyword(lexer, "fn", &token)) {
    return token;
  }  else if (match_keyword(lexer, "let", &token)) {
    return token;
  } else if (match_keyword(lexer, "mut", &token)) {
    return token;
  } else if (match_keyword(lexer, "if", &token)) {
    return token;
  } else if (match_keyword(lexer, "else", &token)) {
    return token;
  } else if (match_keyword(lexer, "struct", &token)) {
    return token;
  } else if (match_keyword(lexer, "for", &token)) {
    return token;
  } else if (match_keyword(lexer, "interface", &token)) {
    return token;
  } else if (match_keyword(lexer, "@inline", &token)) {
    return token;
  } else if (match_keyword(lexer, "@external", &token)) {
    return token;
  }
  else if ((c == 'i' || c == 'u' || c == 'f') && isdigit(lexer->contents[lexer->position + 1])) {
    token.value = l_read_alnum(lexer);
    token.kind = TOK_DATA_TYPE;
    token.dtype = l_parse_dtype(token.value);
    return token;
  } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
    size_t start = lexer->position;
    while (l_peek(lexer, 0) != -1) {
      int ch = l_peek(lexer, 0);
      if ((ch >= 'a' && ch <= 'z') ||
          (ch >= 'A' && ch <= 'Z') ||
          (ch >= '0' && ch <= '9') ||
          ch == '_') {
        l_advance(lexer);
      } else {
        break;
      }
    }
    size_t len = lexer->position - start;
    token.value = strndup(&lexer->contents[start], len);
    token.kind  = TOK_IDENT;
    return token;
  }
  l_advance(lexer);
  token.kind  = TOK_INVALID;
  token.value = NULL;
  return token;
}

int t_print(t_lexer *lexer, t_token *token) {
  const char *kind;
  switch (token->kind) {
  case TOK_OPERATOR: kind  = "Operator"; break;
  case TOK_LPAREN: kind    = "LParen"; break;
  case TOK_RPAREN: kind    = "RParen"; break;
  case TOK_EOF: kind       = "EOF"; break;
  case TOK_SEMICOLON: kind = "Semicolon"; break;
  case TOK_KEYWORD: kind   = "Keyword"; break;
  case TOK_IDENT: kind     = "Identifier"; break;
  case TOK_ASSIGN: kind    = "Assign"; break;
  case TOK_LBRACE: kind    = "Lbrace"; break;
  case TOK_RBRACE: kind    = "Rbrace"; break;
  case TOK_COLON: kind     = "Colon"; break;
  case TOK_INTEGER: kind   = "Integer"; break;
  case TOK_FLOAT: kind     = "Float"; break;
  case TOK_STRING: kind    = "String"; break;
  case TOK_EQ: kind        = "Equals"; break;
  case TOK_LEQ: kind       = "LessEquals"; break;
  case TOK_GEQ: kind       = "GreaterEquals"; break;
  case TOK_LESS: kind      = "Less"; break;
  case TOK_GREATER: kind   = "Greater"; break;
  case TOK_COMMA: kind     = "Comma"; break;
  case TOK_QUESTION: kind  = "Question"; break;
  case TOK_PP: kind = "PlusPlus"; break;
  case TOK_DOT: kind = "Dot"; break;
  case TOK_DATA_TYPE: {
    const char *type_kind;
    switch (token->dtype.kind) {
    case TYPE_INT: type_kind     = "int"; break;
    case TYPE_UINT: type_kind    = "uint"; break;
    case TYPE_FLOAT: type_kind   = "float"; break;
    case TYPE_BOOL: type_kind    = "bool"; break;
    case TYPE_VOID: type_kind    = "void"; break;
    case TYPE_STRING: type_kind  = "str"; break;
    default: type_kind = "unknown"; break;
    }
    if (token->dtype.width) {
      printf("DataType(%s, kind=%s, width=%d)\n", token->value, type_kind, token->dtype.width);
      return 0;
    } else {
      printf("DataType(%s, kind=%s)\n", token->value, type_kind);
      return 0;
    }
  }
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
  fprintf(stderr, "  %*s^\n", (int)(lexer->col - 1), "");
}
