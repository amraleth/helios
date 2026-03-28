#include "ast.h"
#include <stdio.h>

static void node_print_r(t_node *node, const char *prefix, int is_last) {
  if (!node) return;

  printf("%s%s", prefix, is_last ? "> --- " : "| ---");

  switch (node->kind) {
  case NODE_INT:
    printf("INT(%lld)\n", node->ival);
    break;
  case NODE_FLOAT:
    printf("FLOAT(%g)\n", node->fval);
    break;
  case NODE_UNOP:
    printf("UNOP(%s)\n", node->unop.op);
    char unop_prefix[256];
    snprintf(unop_prefix, sizeof(unop_prefix), "%s%s", prefix, is_last ? "    " : "|   ");
    node_print_r(node->unop.operand, unop_prefix, 1);
    break;
  case NODE_BINOP:
    printf("BINOP(%s)\n", node->binop.op);
    char binop_prefix[256];
    snprintf(binop_prefix, sizeof(binop_prefix), "%s%s", prefix, is_last ? "    " : "|   ");
    node_print_r(node->binop.left,  binop_prefix, 0); // not last
    node_print_r(node->binop.right, binop_prefix, 1); // last
    break;
  default:
    printf("UNKNOWN(%d)\n", node->kind);
    break;
  }
}

void node_print(t_node *node) {
  if (!node) { printf("<null>\n"); return; }
  // print root label manually since it has no parent prefix
  switch (node->kind) {
  case NODE_INT:   printf("INT(%lld)\n",  node->ival); break;
  case NODE_FLOAT: printf("FLOAT(%g)\n",  node->fval); break;
  case NODE_UNOP:
    printf("UNOP(%s)\n", node->unop.op);
    node_print_r(node->unop.operand, "", 1);
    break;
  case NODE_BINOP:
    printf("BINOP(%s)\n", node->binop.op);
    node_print_r(node->binop.left,  "", 0);
    node_print_r(node->binop.right, "", 1);
    break;
  default: printf("UNKNOWN(%d)\n", node->kind); break;
  }
}
