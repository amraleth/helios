#include "parser.h"
#include "lexer.h"
#include <stdlib.h>
#include <string.h>

t_parser *p_create(t_lexer *lexer) {
  t_parser *parser = malloc(sizeof(t_parser));
  parser->lexer = lexer;
  parser->current = l_next_tok(lexer);
  return parser;
}

static inline t_token p_peek(t_parser *parser) {
  return parser->current;
}

static inline t_token p_consume(t_parser *parser) {
  t_token tok = parser->current;
  parser->current = l_next_tok(parser->lexer);
  return tok;
}

static inline t_token p_expect(t_parser *parser, t_token_kind kind) {
  t_token tok = parser->current;
  if (tok.kind != kind) {
    PARSE_ERROR(parser->lexer, tok, "expected token kind %d, got '%s'", kind, tok.value);
  }
  return p_consume(parser);
}

static inline t_token p_expect_value(t_parser *parser, t_token_kind kind, const char* value) {
  t_token tok = parser->current;
  if (tok.kind != kind || strcmp(tok.value, value) != 0) {
    PARSE_ERROR(parser->lexer, tok, "expected '%s', got '%s'", value, tok.value);
  }
  return p_consume(parser);
}

static inline int p_match(t_parser *p, t_token_kind kind) {
  if (p->current.kind == kind) {
    p_consume(p);
    return 1;
  }
  return 0;
}
