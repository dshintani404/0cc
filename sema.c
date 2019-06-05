#include "0cc.h"

void walk(Node* node) {
  switch (node->type) {
    case '=':
      walk(node->lhs);
      walk(node->rhs);
      break;
    case '+':
      if (node->lhs->type != ND_IDENT) break;
      if (node->lhs->dtype->type == PTR) {
        if (node->lhs->dtype->pointer_of->type == PTR) node->rhs->value *= 8;
        if (node->lhs->dtype->pointer_of->type == INT) node->rhs->value *= 4;
      }
      break;
    case '-':
      if (node->lhs->type != ND_IDENT) break;
      if (node->lhs->dtype->type == PTR) {
        if (node->lhs->dtype->pointer_of->type == PTR) node->rhs->value *= 8;
        if (node->lhs->dtype->pointer_of->type == INT) node->rhs->value *= 4;
      }
      break;
  }
}

void sema() {
  for (int i=0; code[i][0]; i++){
    for (int j=1; code[i][j]; j++) {
      walk(code[i][j]);
    }
  }
}

