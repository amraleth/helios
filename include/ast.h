#ifndef HELIOS_AST_H
#define HELIOS_AST_H

typedef enum {
  NODE_INT,
  NODE_FLOAT,
  NODE_UNOP,
  NODE_BINOP
} t_node_kind;

typedef struct t_node t_node;

struct t_node {
  t_node_kind  kind;
  union {
    long long  ival;            // NODE_INT
    double     fval;            // NODE_FLOAT
    struct {
      char     op[3];
      t_node  *operand;
    } unop;                     // NODE_UNOP
    struct {
      char     op[3];
      t_node  *left;
      t_node  *right;
    } binop;                    // NODE_BINOP
  };
};

void node_print(t_node *node);

#endif // HELIOS_AST_H ends here
