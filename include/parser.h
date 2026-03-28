#ifndef HELIOS_PARSER_H
#define HELIOS_PARSER_H

#include "lexer.h"
#include <stdio.h>
#include "ast.h"
#include <stdarg.h>

#define PARSE_ERROR(parser, tok, fmt, ...) \
    do { \
        p_print_error(parser, &(tok), fmt, ##__VA_ARGS__); \
        exit(1); \
    } while (0)

#define PARSE_WARN(lexer, tok, fmt, ...)                                \
  do {                                                                  \
    fprintf(stderr, "%s:%zu:%zu: warning: " fmt "\n",                   \
            (lexer)->filename, (tok).line, (tok).col, ##__VA_ARGS__);   \
  } while (0)

typedef struct {
  t_lexer *lexer;
  t_token  current;
} t_parser;

t_parser *p_create(t_lexer *lexer);

t_node *p_parse_expression(t_parser *parser);

void p_print_error(t_parser *parser, t_token *tok, const char* fmt, ...);

#endif // HELIOS_PARSER_H ends here
