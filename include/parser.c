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
    PARSE_ERROR(parser, tok, "expected token kind %d, got '%s'", kind, tok.value);
  }
  return p_consume(parser);
}

static inline t_token p_expect_value(t_parser *parser, t_token_kind kind, const char* value) {
  t_token tok = parser->current;
  if (tok.kind != kind || strcmp(tok.value, value) != 0) {
    PARSE_ERROR(parser, tok, "expected '%s', got '%s'", value, tok.value);
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

static int p_precedence(const char *op) {
  if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0) return 1;
  if (strcmp(op, "*") == 0 || strcmp(op, "/") == 0) return 2;
  if (strcmp(op, "^^") == 0) return 3;
  return -1; // not a binary operation
}

static int p_is_right_assoc(const char *op) {
  return strcmp(op, "^^") == 0;
}

static t_node *p_parse_primary(t_parser *parser);
static t_node *p_parse_expr(t_parser *parser, int min_prec);

static t_node *p_parse_primary(t_parser *parser) {
  t_token tok = p_peek(parser);

  if (tok.kind == TOK_EOF) {
    PARSE_ERROR(parser, tok, "unexpected end of file, expected a number or '('");
  }

  // grouped expression
  if (tok.kind == TOK_LPAREN) {
    p_consume(parser);
    t_node *inner = p_parse_expr(parser, 0);
    p_expect(parser, TOK_RPAREN);
    return inner;
  }

  // unary minus -expr
  if (tok.kind == TOK_OPERATOR && strcmp(tok.value, "-") == 0) {
    p_consume(parser);
    t_node *node = malloc(sizeof(t_node));
    node->kind       = NODE_UNOP;
    node->unop.op[0] = '-';
    node->unop.op[1] = '\0';
    node->unop.operand = p_parse_primary(parser);
    return node;
  }

  // integer literal
  if (tok.kind == TOK_INTEGER) {
    t_token tok = p_consume(parser);
    t_node *node = malloc(sizeof(t_node));
    node->kind = NODE_INT;
    node->ival = strtoll(tok.value, NULL, 10);
    free(tok.value);
    return node;
  }

  // float literal
  if (tok.kind == TOK_FLOAT) {
    t_token tok = p_consume(parser);
    t_node *node = malloc(sizeof(t_node));
    node->kind = NODE_FLOAT;
    node->fval = strtod(tok.value, NULL);
    free(tok.value);
    return node;
  }

  PARSE_ERROR(parser, tok, "expected a number or '(', got '%s'", tok.value);
}

static t_node *p_parse_expr(t_parser *parser, int min_prec) {
    t_node *left = p_parse_primary(parser);

    for (;;) {
        t_token tok = p_peek(parser);
        if (tok.kind != TOK_OPERATOR) break;
        int prec = p_precedence(tok.value);
        if (prec < 0 || prec < min_prec) break;

        t_token optok = p_consume(parser); // capture instead of discarding
        int next_prec = p_is_right_assoc(optok.value) ? prec : prec + 1;
        t_node *right = p_parse_expr(parser, next_prec);

        t_node *node = malloc(sizeof(t_node));
        node->kind = NODE_BINOP;
        strncpy(node->binop.op, optok.value, 2);
        node->binop.op[2] = '\0';
        free(optok.value); // done with it
        node->binop.left  = left;
        node->binop.right = right;
        left = node;
    }

    t_token next = p_peek(parser);
    if (next.kind == TOK_INTEGER ||
        next.kind == TOK_FLOAT ||
        next.kind == TOK_LPAREN ||
        next.kind == TOK_IDENT) {
      PARSE_ERROR(parser, next, "expected operator between expressions, got '%s'", next.value ? next.value : "unknown");
    } 

    return left;
}

t_node *p_parse_expression(t_parser *parser) {
  return p_parse_expr(parser, 0);
}

void p_print_error(t_parser *parser, t_token *tok, const char *fmt, ...) {
  t_lexer *lexer = parser->lexer;

  size_t start = 0;
  size_t line  = 1;
  while (start < lexer->len && line < tok->line) {
    if (lexer->contents[start] == '\n') {
      line++;
    }
    start++;
  }

  size_t end = start;
  while (end < lexer->len && lexer->contents[end] != '\n') {
    end++;
  }

    fprintf(stderr, "\n%s:%zu:%zu: parse error: ", lexer->filename, tok->line, tok->col);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");

    fprintf(stderr, "  %.*s\n", (int)(end - start), &lexer->contents[start]);

    fprintf(stderr, "  %*s^\n", (int)(tok->col - 1), "");
}
